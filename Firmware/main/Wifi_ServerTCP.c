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
static char tag[] = "socket_server";
int sock ;
int cliente;
int haycliente=0;
//=========================== Implementacion ================================

static void readData(int sock) {
	int len;
    char rx_bufferr[128];

    do {
        len = recv(sock, rx_bufferr, sizeof(rx_bufferr) - 1, 0);
        if (len < 0) {
            ESP_LOGE(tag, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(tag, "Connection closed");
        } else {
            rx_bufferr[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(tag, "Received %d bytes: %s", len, rx_bufferr);
           
        }
    } while (len > 0);
}

void sendData(char *buffer, int caracteres) {
	if(haycliente>0){
		int sock=cliente;
		int len=caracteres;

		int to_write = len;
		while (to_write > 0) {
			int written = send(sock, buffer + (len - to_write), to_write, 0);
			if (written < 0) {
				ESP_LOGE(tag, "Error occurred during sending: errno %d", errno);
				// Failed to retransmit, giving up
				return;
			}
			to_write -= written;
		}
	}
}

esp_err_t wifi_event_handler2(void *ctx, system_event_t *event) {
    return ESP_OK;
}

/**

Creates a new Wifi-AP on ESP32

*/
void Send_TCPmsg(int len, char rx_bufferr) {
  // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
	int to_write = len;
	while (to_write > 0) {
		int written = send(sock, rx_bufferr + (len - to_write), to_write, 0);
		if (written < 0) {
			ESP_LOGE(tag, "Error occurred during sending: errno %d", errno);
			// Failed to retransmit, giving up
			return;
		}
		to_write -= written;
	}
}


/**
 * Create a listening socket.  We then wait for a client to connect.
 * Once a client has connected, we then read until there is no more data
 * and log the data read.  We then close the client socket and start
 * waiting for a new connection.
 */
void socket_server_task(void *ignore) {
	struct sockaddr_in clientAddress;
	struct sockaddr_in serverAddress;

	// Create a socket that we will listen upon.
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		ESP_LOGE(tag, "socket: %d %s", sock, strerror(errno));
		goto END;
	}

	// Bind our server socket to a port.
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT_NUMBER);
	int rc  = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc < 0) {
		ESP_LOGE(tag, "bind: %d %s", rc, strerror(errno));
		goto END;
	}

	// Flag the socket as listening for new connections.
	rc = listen(sock, 5);
	if (rc < 0) {
		ESP_LOGE(tag, "listen: %d %s", rc, strerror(errno));
		goto END;
	}

	while (1) {
		// Listen for a new client connection.
		ESP_LOGD(tag, "Waiting for new client connection");
		socklen_t clientAddressLength = sizeof(clientAddress);
		int clientSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (clientSock < 0) {
			ESP_LOGE(tag, "accept: %d %s", clientSock, strerror(errno));
			goto END;
		}
		cliente =clientSock;
		haycliente++;
		readData(clientSock);
		vTaskDelay(5/portTICK_PERIOD_MS) ;      // Delay para retardo del contador 
	}
	END:
	vTaskDelete(NULL);
}



void socket_server(void) {
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
  xTaskCreate(socket_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
}


