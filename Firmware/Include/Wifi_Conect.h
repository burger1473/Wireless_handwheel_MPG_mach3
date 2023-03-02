/*=============================================================================
 * Author: Burgos Fabian, Rubiolo Bruno
 * Date: 01/10/2021 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 *ACLARACION: Esta libreria tiene 3 modos de funcionamiento:
 *
 *            Modo Estacion: Se conecta al Wifi declarado en Wifi_Conect.h
 *            Modo AP:       Genera una red Wifi con el nombre y contraseña declarados en Wifi_Conect.h
 *                           Genera un Web server con el codigo HTML establecido en html.h
 *            Modo DUAL:     Obtiene de la memoria EEPROM los datos de la red a la cual decea conectarse en modo estacion
 *                           Si no logra conectarse luego de x intentos, resetea el esp y en el proximo inicio genera un AP
 *                           Este AP genera un Web Server en 192.168.4.1 con el codigo HTML de html_conectar_wifi.h
 *                           Este codigo HTML es una web donde se puede ingresar la red y contraseña a la cual se quiere conectar en mdoo estacion
 *                           Cuando el usuario establece la red en la web generada, el esp32 almacena estos valores en la EEPROM y resetea el esp
 *                           Luego del reseteo, el micro comienza como Estacion e intenta conectarse a los datos antes ingresados en la EEPROM.
 *
 *            En este trabajo se utiliza este ultimo modo.
 *
 *IMPORTANTE: para evitar sobrecargar la memoria del micro, se utilizo ifdefined para que el comilador solo comile lo que necesite para el modo seleccionado.
 *===========================================================================*/
#ifndef Wifi_Conect_H_
#define Wifi_Conect_H_

/*==================[ Inclusiones ]============================================*/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <esp_http_server.h>
#include "lwip/err.h"
#include "lwip/sys.h"

#include "../include/EEPROM.h"



//=========================== Definiciones ================================
//Descomentar el modo que desea seleccionar
//#define MODE_ESTACION
//#define MODE_AP
#define MODE_DUAL

// El event group permite múltiples bits para cada evento, pero solo nos interesan dos:
#define WIFI_CONNECTED_BIT BIT0 //está conectado al AP con una IP
#define WIFI_FAIL_BIT      BIT1 //no pudo conectarse después de la cantidad máxima de reintentos

#if  MODE_ESTACION                         //Si esta en modo Estacion
    #define WIFI_SSID      "CONECTAR1354" //SSID para conectarse en modo estacion
    #define WIFI_PASS      "309997609"    //PASS para conectarse en modo estacion
    #define MAXIMUM_RETRY  2              //Cantidad de intentos de conexion
#endif

#if  defined (MODE_AP) || defined (MODE_DUAL)            // Si esta en modo AP o dual
    #define EXAMPLE_ESP_WIFI_SSID_AP      "Wifi-Config"  // Genera una red con este nombre
    #define EXAMPLE_ESP_WIFI_PASS_AP      "123456789"    // Genera una red con esta contraseña
    #define EXAMPLE_ESP_WIFI_CHANNEL   1                 // Canal
    #define EXAMPLE_MAX_STA_CONN       1                 // Numero maximo de conexiones al AP
#endif

#ifdef MODE_DUAL                                 //Si esta en modo DUAL
    #define MAXIMUM_RETRY  2                    //Cantidad de intentos de conexion en modo estacion
    static RTC_NOINIT_ATTR int error_estacion; //Esta variable es un tipo de variable que no se borra el dato al reiniciar el micro
    #include "../include/html_conectar_wifi.h" //Incluyo la variable que contiene el HTML de la pagina de configuracion de wifi
#endif

#ifdef MODE_AP                      //Si esta en modo AP
   #include "../include/html.h"     //Incluyo la variable que contiene el HTML de la pagina para modo AP
#endif

/*==================[Prototipos de funciones]======================*/
void iniciarWifi(void);

//=========================== Variables ================================
#ifdef MODE_DUAL
     uint8_t WIFI_SSID[32] = { 0 };
     uint8_t WIFI_PASS[64] = { 0 };
#endif

#include "../main/Wifi_Conect.c"
#endif