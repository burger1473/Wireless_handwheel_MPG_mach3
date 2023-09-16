#ifndef ADC_H_
#define ADC_H_

/*==================[ Inclusiones ]============================================*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h> //numero ramdon
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <driver/adc.h>

/*==================[Prototipos de funciones]======================*/
void config_adc(uint8_t adc, uint8_t pin, adc_bits_width_t resoluncion, adc_atten_t atenuacion);
int Leer_adc(uint8_t adc, uint8_t pin, uint8_t promedio, adc_bits_width_t resoluncion);


#include "../main/ADC.c"
#endif