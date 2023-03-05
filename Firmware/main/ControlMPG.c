/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para realizar las activaders del control MPG
 *
 *===========================================================================*/

/*===================================================[ Inclusiones ]===============================================*/
#include "../include/ControlMPG.h"
#include <esp32/rom/ets_sys.h>    //Para retardo bloqueando todo el planificador
#include "../include/LCD.h"

/*===================================================[Prototipos de funciones]===============================================*/
static void ControlMPG(void *pvParameters);

/*===================================================[Variables]===============================================*/
TaskHandle_t TaskHandle_1 = NULL;

/*===================================================[Implementaciones]===============================================*/


void ControlMPG_init(void){
    esp_err_t ret = xTaskCreate(&ControlMPG, "ControlMPG", 3072, NULL, 1, &TaskHandle_1); //Creo tarea
    if (ret != pdPASS)  {
        //ESP_LOGI(TAG, "Error al crear tarea");
    }
}

static void ControlMPG(void *pvParameters) {
    while (1) {

        char buffer[54];
        strcpy(buffer, "N0451*********************************************51F");
        sendData(buffer, 53);

        LCDclr();
        LCDGotoXY(0, 0);
        LCD_print("--- ---");
        LCDGotoXY(10, 0);
        LCD_print("ControlMPG");
        LCDGotoXY(0, 1);
        LCD_print("X:     +2.6162");
        LCDGotoXY(0, 2);
        LCD_print("Y:     +1.3808");
        LCDGotoXY(0, 3);
        LCD_print("Z:     -1.5034");
        vTaskDelay(100/portTICK_PERIOD_MS) ;      // Delay para retardo del contador 
    }
}