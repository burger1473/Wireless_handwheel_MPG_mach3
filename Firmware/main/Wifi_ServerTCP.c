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
#define PORT                        8001
#define KEEPALIVE_IDLE              5
#define KEEPALIVE_INTERVAL          5
#define KEEPALIVE_COUNT             3

//=========================== Variables ================================
static char tag[] = "socket_server";

//=========================== Implementacion ================================
static void sendData(int socket) {
	char text[80];
	int i;
	for(i=1;i<=10;i++){
		sprintf(text, "Message %d\n", i);
		send(socket, text, strlen(text), 0);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
	close(socket);
}

/*
esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    return ESP_OK;
}
*/

void socket_server_task(void *ignore) {
	struct sockaddr_in clientAddress;
	struct sockaddr_in serverAddress;

	// Create a socket that we will listen upon.
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
		sendData(clientSock);
	}
	END:
	vTaskDelete(NULL);
}


void socket_server(void) {
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
  xTaskCreate(socket_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
}


