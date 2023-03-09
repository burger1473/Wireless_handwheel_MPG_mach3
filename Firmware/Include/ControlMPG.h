/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para realizar las activaders del control MPG
 *
 *===========================================================================*/
 
 //=========================== Definiciones ================================
 #ifndef ControlMPG_H_
 #define ControlMPG_H_

 /*===================================================[ Inclusiones ]===============================================*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include "Defines.h"

/*==================[Prototipos de funciones]======================*/
void ControlMPG_init(void);



#include "../main/ControlMPG.c"
#endif