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
#define PORT_NUMBER 8001

//=========================== Variables ================================
const static char *tag = "socketServer";
struct sockaddr_in clientAddress;
struct sockaddr_in serverAddress;
int sock;
int clientSock;
uint8_t hay_cliente=0;
TaskHandle_t TaskHandle_2 = NULL;
int len_ext;
char rx_buffer_ext[128];
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; //Inicializa el spinlock desbloqueado
bool hay_mensaje_nuevo=false;
//SemaphoreHandle_t semaforo_socket = NULL; //Puntero al semaforo

//=========================== Prototipos ================================
esp_err_t wifi_event_handler2(void *ctx, system_event_t *event);
static void Socket_task(void *pvParameters);

//=========================== Implementacion ================================

void ServerTCP_configwifi(void){
	tcpip_adapter_init();
    esp_event_loop_init(wifi_event_handler2, NULL);
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init_config);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
}

void ServerTCP_configmDNS(void){
	//initialize mDNS
	ESP_ERROR_CHECK( mdns_init() );
	//set mDNS hostname (required if you want to advertise services)
	ESP_ERROR_CHECK( mdns_hostname_set(mDNShostName) );
    ESP_ERROR_CHECK( mdns_instance_name_set(mDNShostName) );
	ESP_LOGI(tag, "mdns hostname set to: [%s]", mDNShostName);
    ESP_LOGI(tag, "Se finalizó la inicialización del WiFi.");
}

bool ServerTCP_readData(void) {
    //xSemaphoreTake(semaforo_socket, portMAX_DELAY); //Tomo semaforo para evitar que se intente enviar y recibir mensajes tcp/ip al mismo tiempo
	if(hay_cliente==1){
		int len;
		char rx_bufferr[128];
		do {
			len = recv(clientSock, rx_bufferr, sizeof(rx_bufferr) - 1, 0); // Bloquea la tarea hasta recibir dato
			if (len < 0) {
				ESP_LOGE(tag, "Error occurred during receiving: errno %d", errno);
				hay_cliente=0;
				return false;
			} else if (len == 0) {
				ESP_LOGW(tag, "Connection closed");
				hay_cliente=0;
				return false;
			} else {
				rx_bufferr[len] = 0; // Null-terminate whatever is received and treat it like a string
				ESP_LOGI(tag, "Received %d bytes: %s", len, rx_bufferr);
				portENTER_CRITICAL(&mux);                               //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
				hay_mensaje_nuevo=true;
				len_ext=len;
				for(uint8_t i=0; i<128; i++){
					rx_buffer_ext[i]=rx_bufferr[i];
				}
				portEXIT_CRITICAL(&mux);                                //Salgo seccion critica
				return true;
			}
		} while (len > 0);
	}else{
		return false;
	}
	
	//xSemaphoreGive(semaforo_socket);			//Libero semaforo
}

bool ServerTCP_sendData(char *buffer, int len) {
	//xSemaphoreTake(semaforo_socket, portMAX_DELAY); //Tomo semaforo para evitar que se intente enviar y recibir mensajes tcp/ip al mismo tiempo
	bool retorno = true;
	if(hay_cliente==1){
		int to_write = len;
		ESP_LOGI(tag, "Send %d bytes: %s", len, buffer);
		while (to_write > 0) {
			int written = send(clientSock, buffer + (len - to_write), to_write, 0);
			if (written < 0) {
				ESP_LOGE(tag, "Error occurred during sending: errno %d", errno);
				// Failed to retransmit, giving up
				hay_cliente=0;
				retorno=false;
				break;
			}
			to_write -= written;
		}
	}else{
		retorno=false;
	}
    //xSemaphoreGive(semaforo_socket);			//Libero semaforo
	return retorno;
}

esp_err_t wifi_event_handler2(void *ctx, system_event_t *event) {
    return ESP_OK;
}


bool ServerTCP_socket_init(uint8_t prioridad){
	// Create a socket that we will listen upon.
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		ESP_LOGE(tag, "socket: %d %s", sock, strerror(errno));
		return false;
	}

	// Bind our server socket to a port.
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT_NUMBER);
	int rc  = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc < 0) {
		ESP_LOGE(tag, "bind: %d %s", rc, strerror(errno));
		return false;
	}

	// Flag the socket as listening for new connections.
	rc = listen(sock, 5);
	if (rc < 0) {
		ESP_LOGE(tag, "listen: %d %s", rc, strerror(errno));
		return false;
	}

	esp_err_t ret = xTaskCreate(&Socket_task, "Socket_task", 3072, NULL, prioridad, &TaskHandle_2); //Creo tarea
    if (ret != pdPASS)  {
        return false;
    }

	return true;
}

bool ServerTCP_buscarcliente(){
		// Listen for a new client connection.
		ESP_LOGD(tag, "Waiting for new client connection");
		socklen_t clientAddressLength = sizeof(clientAddress);
		clientSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (clientSock < 0) {
			ESP_LOGE(tag, "accept: %d %s", clientSock, strerror(errno));
			hay_cliente=0;
			return false;
		}
		hay_cliente=1;
		return true;
} 

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
	while(true){
		if(hay_cliente==0){
			if(ServerTCP_buscarcliente()==true){ 
			}
		}else{
			if(ServerTCP_readData() == true){  //Si se pudo leer el mensaje. Bloquea la tarea hasta recibir dato
				
			}
		}
		vTaskDelay(5/portTICK_PERIOD_MS) ;      // Delay para retardo del contador
	}
}

uint8_t ServerTCP_leermensaje(char *buffer){
	if (hay_mensaje_nuevo==true){
		//ESP_LOGE(tag, "LL: %d", len_ext);
		int variable_local=0;
		portENTER_CRITICAL(&mux);                               //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
		variable_local=len_ext;
		for(uint8_t i=0; i<128; i++){
			buffer[i]=rx_buffer_ext[i];
		}
		hay_mensaje_nuevo=false;
		portEXIT_CRITICAL(&mux);                                //Salgo seccion critica
		
		return variable_local;
	}else{
		return 0;
	}
	
}