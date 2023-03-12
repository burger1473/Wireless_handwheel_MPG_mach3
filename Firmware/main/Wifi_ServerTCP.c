/*=============================================================================
 * Author: Burgos Fabian, Rubiolo Bruno
 * Date: 01/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 *Descripcion: Libreria para crear server TCP y realizar comunicacion bidireccional
 *
 *IMPORTANTE: para evitar sobrecargar la memoria del micro, se utilizo ifdefined para que el comilador solo comile lo que necesite para el modo seleccionado.
 *===========================================================================*/
/*===================================================[ Inclusiones ]===============================================*/

#include "../include/Wifi_ServerTCP.h"
//#include "freertos/semphr.h" //librería para el uso de semaforos

//=========================== Definiciones ================================
#define PORT_NUMBER 8001	   				//Puerto server TCP/IP

//=========================== Variables ================================
const static char *tag = "socketServer";	//variable para indicar de donde vienen los mensajes DEBUG
struct sockaddr_in clientAddress;			//
struct sockaddr_in serverAddress;
int sock;									//Numero de socket
int clientSock;								//Numero del cliente
uint8_t hay_cliente=0;					    //Numero para indicar que hay cliente conectado
TaskHandle_t TaskHandle_2 = NULL;			//Puntero a la tarea de recepcion por tcpip
int len_ext;								//Variable para acceder externa ente al tamaño del mensaje recibido por TCP/IP
char rx_buffer_ext[128];					//Variable para acceder externamente al  mensaje recibido por TCP/IP
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; //Inicializa el spinlock desbloqueado (para seccion critica)
bool hay_mensaje_nuevo=false;				//Indica si hay un mensaje nuevo para leer
//SemaphoreHandle_t semaforo_socket = NULL; //Puntero al semaforo

//=========================== Prototipos ================================
esp_err_t wifi_event_handler2(void *ctx, system_event_t *event);
static void Socket_task(void *pvParameters);

//=========================== Implementacion ================================

/*========================================================================
Funcion: ServerTCP_configwifi
Descripcion: Configura el servidor TCP/IP y lo inicializa
Sin parametro de entrada
No retorna nada
========================================================================*/
void ServerTCP_configwifi(void){
	tcpip_adapter_init();												//Inicia el adapatador tcpip
    esp_event_loop_init(wifi_event_handler2, NULL);					    //Indica el callback al cual debe llamarse cuando ocurre algun evento en el periferico wifi
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();	//Toma configuraciones de wifi
    esp_wifi_init(&wifi_init_config);									//Inicia el wifi con las configuraciones anteriores
    esp_wifi_set_storage(WIFI_STORAGE_RAM);							    //Asigna ram al periferico wifi
}

/*========================================================================
Funcion: ServerTCP_mDNS_addService
Descripcion: Agrega un servicio al mDNS para indicar la actividad http en un puerto especifico
Parametro de entrada: uint8_t port: puerto donde se quiere agregar mDNS para http
No retorna nada
========================================================================*/
void ServerTCP_mDNS_addService(uint8_t port){
	mdns_service_add(NULL, "_http", "_tcp", port, NULL, 0);
}

/*========================================================================
Funcion: ServerTCP_configmDNS
Descripcion: Configura mDNS para poder acceder sin saber la IP.
Sin parametro de entrada
No retorna nada
========================================================================*/
void ServerTCP_configmDNS(void){
	
	ESP_ERROR_CHECK( mdns_init() );								//Inicializa mDNS
	ESP_ERROR_CHECK( mdns_hostname_set(mDNShostName) );			//Setea el nombre de mDNS (se puede cambiar en la definicion de Wifi_ServerTCP.h o en el Defines.h)
    ESP_ERROR_CHECK( mdns_instance_name_set(mDNShostName) );	//Setea el nombre de mDNS (se puede cambiar en la definicion de Wifi_ServerTCP.h o en el Defines.h)
	ESP_LOGI(tag, "mdns hostname set to: [%s]", mDNShostName);
    ESP_LOGI(tag, "Se finalizó la inicialización del WiFi.");
}

/*========================================================================
Funcion: ServerTCP_readData
Descripcion: Lee la data disponible proveniente del cliente en el server tcp/ip
Sin parametro de entrada
Retorna:  true:  si se puedo leer el mensaje
		  false: si no se pudo leer el mensaje
========================================================================*/
bool ServerTCP_readData(void) {
    //xSemaphoreTake(semaforo_socket, portMAX_DELAY); //Tomo semaforo para evitar que se intente enviar y recibir mensajes tcp/ip al mismo tiempo
	if(hay_cliente==1){			//Si hay cliente conectado
		int len;				//Variable para almacenar localmente el tamaño del mensaje recibido
		char rx_bufferr[128];   //Variable para almacenar localmente el mensaje recibido
		do {
			len = recv(clientSock, rx_bufferr, sizeof(rx_bufferr) - 1, 0); // Bloquea la tarea hasta recibir dato y almacena el mensaje y el tamaño en las variables pasadas
			if (len < 0) {		//Si el tamaño del mensaje es menor a cero, ocurrio un error
				ESP_LOGE(tag, "Error occurred during receiving: errno %d", errno);
				hay_cliente=0;	//Indico que se desconecto el cliente
				return false;	//Retorno false para indicar que no se pudo leer el mensaje
			} else if (len == 0) { //Si el mensaje es igual a cero, se cerro la conexion
				ESP_LOGW(tag, "Connection closed");
				hay_cliente=0;	//Indico que el cliente se deconecto
				return false;	//Retorno false para indicar que no se pudo leer el mensaje
			} else {			//Si el dato es mayor a cero
				rx_bufferr[len] = 0; // Termina nulo lo que se reciba y trátelo como una cadena
				ESP_LOGI(tag, "Received %d bytes: %s", len, rx_bufferr);
				portENTER_CRITICAL(&mux);            //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
				hay_mensaje_nuevo=true;				 //Incico que hay un nuevo mensaje	
				len_ext=len;						 //Paso tamaño de variable local a global
				for(uint8_t i=0; i<128; i++){
					rx_buffer_ext[i]=rx_bufferr[i]; //Paso mensaje de variable local a global
				}
				portEXIT_CRITICAL(&mux);            //Salgo seccion critica
				return true;						//Retorno que se pudo leer el mensaje
			}
		} while (len > 0);
	}else{		
		return false;							   //Retorno que no se pudo leer el mensaje
	}
	
	//xSemaphoreGive(semaforo_socket);			//Libero semaforo
}

/*========================================================================
Funcion: ServerTCP_readData
Descripcion: Envia mensaje al cliente en el server tcp/ip
Parametro de entrada: char *buffer: puntero al buffer que contiene el mensaje
					  int len:      indica el numero de caracteres a enviar
Retorna:  true:  si se puedo enviar el mensaje
		  false: si no se pudo enviar el mensaje
========================================================================*/
bool ServerTCP_sendData(char *buffer, int len) {
	//xSemaphoreTake(semaforo_socket, portMAX_DELAY); //Tomo semaforo para evitar que se intente enviar y recibir mensajes tcp/ip al mismo tiempo
	bool retorno = true;															 //Indico que se debe retornar verdadero
	if(hay_cliente==1){															     //Si hay cliente conectado
		int to_write = len;															 //Paso de variables
		ESP_LOGI(tag, "Send %d bytes: %s", len, buffer);
		while (to_write > 0) {
			int written = send(clientSock, buffer + (len - to_write), to_write, 0);  //Envio el mensaje caracter por caracter por tcp indicando el buffer y el tamaño
			if (written < 0) {														 //Si el resultado es menor a cero, ocurrio un error
				ESP_LOGE(tag, "Error occurred during sending: errno %d", errno);
				hay_cliente=0;													     //Indico que el cliente se decsconecto
				retorno=false;														 //Indico que se debe retornar falso
				break;																 //Salgo del while
			}
			to_write -= written;													 //Indico al while que ya se envio x caracteres
		}
	}else{
		retorno=false;																 //Indico que se debe retornar falso
	}
    //xSemaphoreGive(semaforo_socket);			//Libero semaforo
	return retorno;																	 //Retorno el valor que quedo.
}

/*========================================================================
Funcion: wifi_event_handler2
Descripcion: Envia mensaje al cliente en el server tcp/ip
Parametro de entrada: propios del callback, no los usa
Retorna:  ESP_OK
========================================================================*/
esp_err_t wifi_event_handler2(void *ctx, system_event_t *event) {
    return ESP_OK;
}

/*========================================================================
Funcion: ServerTCP_socket_init
Descripcion: Inicia socket en server tcp/ip y crea una tarea especifica para recibir mensajes por socket.
Parametro de entrada: uint8_t prioridad: prioridad de la tera a asignar
Retorna:  true:  si se puedo iniciar socker
		  false: si no se pudo iniciar socker
========================================================================*/
bool ServerTCP_socket_init(uint8_t prioridad){
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);			//Crea socket
	if (sock < 0) {												//Si no se pudo crer
		ESP_LOGE(tag, "socket: %d %s", sock, strerror(errno));
		return false;											//Retorna falso
	}		

	// Vincula nuestro socket de servidor a un puerto.
	serverAddress.sin_family = AF_INET;						
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT_NUMBER);				//Vincula el socket al puerto en la definicion (se puede modificar en Wifi_ServerTCP.h)
	int rc  = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); 
	if (rc < 0) {
		ESP_LOGE(tag, "bind: %d %s", rc, strerror(errno));
		return false;
	}

	// Marcar el socker para escuchar nuevas conexiones.
	rc = listen(sock, 5);
	if (rc < 0) {
		ESP_LOGE(tag, "listen: %d %s", rc, strerror(errno));
		return false;
	}

	//Crea tarea para recibir mensajes por TCP/IP
	esp_err_t ret = xTaskCreate(&Socket_task, "Socket_task", 3072, NULL, prioridad, &TaskHandle_2); //Creo tarea
    if (ret != pdPASS)  {
        return false;	//Si no se pudo creer, retorna falso
    }

	return true;		//Si no se retorno antes, entonces funciono todo y retorna true
}

/*========================================================================
Funcion: ServerTCP_buscarcliente
Descripcion: Funcion para buscar clientes que se quieren conectar y aceptarlos
Sin parametro de entrada
Retorna:  true:  si se puedo
		  false: si no se pudo
========================================================================*/
bool ServerTCP_buscarcliente(){
		//Escucha a un nuevo cliente
		ESP_LOGD(tag, "Waiting for new client connection");
		socklen_t clientAddressLength = sizeof(clientAddress);
		clientSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (clientSock < 0) {
			ESP_LOGE(tag, "accept: %d %s", clientSock, strerror(errno));
			hay_cliente=0;		//Indico que no hay cliente
			return false;
		}
		hay_cliente=1;			//Indico que hay cliente
		return true;			//Retorno verdadero
} 

/*========================================================================
Funcion: Socket_task
Descripcion: Tarea encargada de buscar nuevos clientes y escuchar sus mensajes periodicamente (debe tener mayor prioridad que las otras tareas)
Sin parametro de entrada
No retorna nada
========================================================================*/
static void Socket_task(void *pvParameters) {
    //semaforo_socket = xSemaphoreCreateBinary(); //Crear el semáforo (arranca “tomado”)
	//xSemaphoreGive(semaforo_socket);			//Libero semaforo
    /*
	if(semaforo_socket == NULL)
    {
		ESP_LOGE(tag, "No se pudo crear el semaforo");
        //while(true);
    }
	*/
	while(true){									//Bucle infinito
		if(hay_cliente==0){							//Si no hay cliente
			if(ServerTCP_buscarcliente()==true){    //Busca clientes nuevos
			}
		}else{										//Si hay clientes
			if(ServerTCP_readData() == true){       //Si se pudo leer el mensaje. Bloquea la tarea hasta recibir dato
				
			}
		}
		vTaskDelay(5/portTICK_PERIOD_MS) ;          //Delay para generar cambio de contexto y dar lugar a otras tareas
	}
}

/*========================================================================
Funcion: ServerTCP_leermensaje
Descripcion: funcion dedicada al intercambio del mensaje recibido por tcp entre la tarea local y otras tareas
Parametro de entrada: char *buffer: puntero al buffer donde se decea enviar la informacion que llego desde el cliente por medio de TCP/IP
Retorna: uint8_t: tamaño del mensaje recibido (tamaño del buffer)
========================================================================*/
uint8_t ServerTCP_leermensaje(char *buffer){
	if (hay_mensaje_nuevo==true){						//Si hay nuevo mensaje
		int variable_local=0;							//Variable local para retornar una variable que no accedan otras tareas y evitar problema en recursor compartidos
		portENTER_CRITICAL(&mux);                       //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
		variable_local=len_ext;							//Paso el tamaño del mensaje recibido desde la variable local a esta libreria a la variable que se va a retornar
		for(uint8_t i=0; i<128; i++){					//Recorro el buffer
			buffer[i]=rx_buffer_ext[i];					//Envio la informacion desde el buffer local a esta libreria al buffer indicado en el puntero
		}
		hay_mensaje_nuevo=false;						//Indico que ya no hay mensaje nuevo
		portEXIT_CRITICAL(&mux);                        //Salgo seccion critica
		
		return variable_local;							//Retorno el tamaño del buffer
	}else{												//Si no hay mensaje nuevo
	return 0;										    //Retorno tamaño 0
	}
	
}