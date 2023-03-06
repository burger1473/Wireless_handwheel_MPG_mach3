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

bool ServerTCP_readData() {
    int len;
	char rx_bufferr[128];
	do {
        len = recv(clientSock, rx_bufferr, sizeof(rx_bufferr) - 1, 0);
        if (len < 0) {
            ESP_LOGE(tag, "Error occurred during receiving: errno %d", errno);
			return false;
        } else if (len == 0) {
            ESP_LOGW(tag, "Connection closed");
			return false;
        } else {
            rx_bufferr[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(tag, "Received %d bytes: %s", len, rx_bufferr);
			//portENTER_CRITICAL(&mux);                               //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
			len_ext=len;
			for(uint8_t i=0; i<128; i++){
				rx_buffer_ext[i]=rx_buffer_ext[i];
			}
			//portEXIT_CRITICAL(&mux);                                //Salgo seccion critica
            hay_mensaje_nuevo=true;
			return true;
        }
    } while (len > 0);
}

bool ServerTCP_sendData(char *buffer, int len) {
	bool retorno = true;
	if(hay_cliente==1){
		int to_write = len;
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
	while(true){

		if(ServerTCP_buscarcliente()==true){ 
			
		}
		if(ServerTCP_readData() == true){  //Si se pudo leer el mensaje
			
		}
		vTaskDelay(5/portTICK_PERIOD_MS) ;      // Delay para retardo del contador
	}
}

bool ServerTCP_leermensaje(uint8_t *lenn, char *buffer){
	if (hay_mensaje_nuevo==true){
		//portENTER_CRITICAL(&mux);                               //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
		*lenn=len_ext;
		for(uint8_t i=0; i<128; i++){
			buffer[i]=rx_buffer_ext[i];
		}
		//portEXIT_CRITICAL(&mux);                                //Salgo seccion critica
		hay_mensaje_nuevo=false;
		return true;
	}else{
		return false;
	}
	
}