#ifndef Interrupcion_gpio_H_
#define Interrupcion_gpio_H_

/*==================[ Inclusiones ]============================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
/*==================[Prototipos de funciones]======================*/
void config_gpio_como_int(gpio_num_t GPIO_INPUT_IO,gpio_int_type_t MODO, gpio_pullup_t pull_up,  gpio_pulldown_t pull_down, uint32_t funcion);
void desactivar_int(gpio_num_t GPIO_INPUT_IO);
void activar_int(gpio_num_t GPIO_INPUT_IO, gpio_int_type_t MODO);


#include "../main/Interrupcion_gpio.c"
#endif