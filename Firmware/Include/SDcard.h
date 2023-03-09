/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para leer memoria SD
 *===========================================================================*/
//=========================== Definiciones ================================
#ifndef SDcard_H_
#define SDcard_H_

#define EXAMPLE_MAX_CHAR_SIZE    64
#define MOUNT_POINT "/sdcard"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.


/*===================================================[ Inclusiones ]===============================================*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include <esp_log.h>
#include <dirent.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <stdlib.h>
#include "Defines.h"


#ifndef SD_PIN_DEFINE
#define PIN_NUM_MISO  GPIO_NUM_19
#define PIN_NUM_MOSI  GPIO_NUM_23
#define PIN_NUM_CLK   GPIO_NUM_18
#define PIN_NUM_CS    GPIO_NUM_5
#endif

// Ruta de montaje del sistema de archivos
#define EXAMPLE_MAX_CHAR_SIZE    64
#define MOUNT_POINT "/sdcard"

/*==================[Prototipos de funciones]======================*/
bool Sd_unmout_list(void);
void SD_abrir_archivo(void);
bool SD_buscar_enlist(char *nombre, bool siguiente);
bool SD_cerrar_archivo(void);
void SD_init(void);
uint8_t SD_contar_lineas_archivo(char *nombre);
bool SD_obtener_linea(char *texto, char *nombre_archivo, uint8_t linea);

//=========================== Variables ================================

#include "../main/SDcard.c"
#endif