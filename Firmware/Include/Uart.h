/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para periferico uart
 *===========================================================================*/
//=========================== Definiciones ================================
#ifndef Uart_H_
#define Uart_H_

/*===================================================[ Inclusiones ]===============================================*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/uart.h"

/*==================[Prototipos de funciones]======================*/
void uart_init(void);
uint8_t uart_ReadChar(void);

//=========================== Variables ================================

#include "../main/Uart.c"
#endif