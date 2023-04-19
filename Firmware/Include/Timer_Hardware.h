#ifndef Timer_Hardware_H_
#define Timer_Hardware_H_

/*==================[ Inclusiones ]============================================*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "rom/gpio.h"
#include <stdlib.h>
#include <stddef.h>
#include "esp_intr_alloc.h"
#include "esp_attr.h"
#include "driver/timer.h"

/*==================[Prototipos de funciones]======================*/
static void timer_tg0_isr(void* arg);
void timer_hardware_config (int group, int timer, int tiempo_us, uint32_t funcion);


#include "../main/Timer_Hardware.c"
#endif