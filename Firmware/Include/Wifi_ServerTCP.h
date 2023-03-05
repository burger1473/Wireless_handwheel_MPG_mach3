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

#ifndef Wifi_ServerTCP_H_
#define Wifi_ServerTCP_H_

/*==================[ Inclusiones ]============================================*/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <lwip/sockets.h>
#include <esp_log.h>
#include <string.h>
#include <errno.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <string.h>
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "mdns.h"

//=========================== Definiciones ================================


/*==================[Prototipos de funciones]======================*/
void socket_server(void);
esp_err_t wifi_event_handler2(void *ctx, system_event_t *event);
void sendData(char *buffer, int caracteres);

//=========================== Variables ================================

#include "../main/Wifi_ServerTCP.c"
#endif