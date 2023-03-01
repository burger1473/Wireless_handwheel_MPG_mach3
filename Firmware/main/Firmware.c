/*=============================================================================
 * Author: Fabian Burgos
 * Date: 07/04/2021 
 * Board: ESP32-CAM
 *===========================================================================*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "soc/soc.h" //disable brownout detector
#include "soc/rtc_cntl_reg.h" //disable brownout detector (deteccion de apagon)
#include "soc/rtc_wdt.h"


#define LED1 4
#define Cantidad_Salidas		6
#define Cantidad_Entradas		2

int8_t Salida [Cantidad_Salidas] = {4, 2, 14, 15, 13, 12}; //especifico pines de salida
int8_t Entrada [Cantidad_Entradas] = {0, 16}; //especifico pines de entrada

void app_main() {
	//Configuración
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    rtc_wdt_protect_off();
    rtc_wdt_disable();
	
	//Configuro salidas y las pongo en estado bajo
    for(int8_t i = 0; i < Cantidad_Salidas; i++){
		gpio_pad_select_gpio(Salida[i]);
		gpio_set_direction(Salida[i], GPIO_MODE_OUTPUT);
        gpio_set_level(Salida[i], 0);
	}
	
	//Configuro entradas y las pongo pulldown
    for(int8_t j = 0; j < Cantidad_Entradas; j++){
		gpio_pad_select_gpio(Entrada[j]);
		gpio_set_direction(Entrada[j], GPIO_MODE_INPUT);
		gpio_set_pull_mode(Entrada[j], GPIO_PULLDOWN_ONLY);
	}
	
	//if (gpio_get_level(PULSADOR1) == 1){}
	
   //Bucle infinito 
    while(1) {
		printf("Escribo en serial\n");
		
        gpio_set_level(Salida[0], 1);//prendo solo led1
		
        vTaskDelay(1000 / portTICK_PERIOD_MS); //Espero 1000 milisegundo
		
        gpio_set_level(Salida[0], 0); //prendo solo led1
        vTaskDelay(1000 / portTICK_PERIOD_MS); //Espero 1000 milisegundo
		
    }

}