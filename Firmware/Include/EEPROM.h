/*=============================================================================
 * Author: Burgos Fabian, Rubiolo Bruno
 * Date: 01/10/2021 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para el manejo de la memoria EEPROM
*===========================================================================*/

/*==================[ Definiciones ]============================================*/
#ifndef EEPROM_h
#define EEPROM_h

/*==================[ Inclusiones ]============================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>


/*==================[Prototipos de funciones]======================*/
void begin_eeprom(void);
int8_t read_eeprom(char key_dir[]);
char * read_array_eeprom(char key_dir[]);
void write_eeprom(char key_dir[], int8_t val);
void write_array_eeprom(char key_dir[], char value[]) ;

#include "../main/EEPROM.c"
#endif
