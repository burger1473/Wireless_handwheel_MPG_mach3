/*=============================================================================
 * Author: Burgos Fabian, Rubiolo Bruno
 * Date:   01/10/2021 
 * Update: 07/06/2022
 * Board:  ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Librerira para manejo del periferico timer del esp32
 * Documentacion: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/timer.html
 *                https://esp32developer.com/programming-in-c-c/timing/hardware-timers
*===========================================================================*/


/*==================[ Inclusiones ]============================================*/
#include "../include/Timer_Hardware.h"

/*==================[Prototipos de funciones]======================*/
static void timer_tg0_isr(void* arg);
void timer_hardware_config (int group, int timer, int tiempo_us,  uint32_t funcion);

//=========================== Variables ================================
static intr_handle_t s_timer_handle[2][2];              //Mango de interrupción, utilizado para liberar el isr después de su uso. Alias ​​a un identificador int por ahora.
uint8_t estado_timer[2][2]={{0, 0},{0, 0}};             //Variable para evitar pausar el timer cuando ya esta pausado
/*==================[Implementaciones]=================================*/


/*========================================================================
Funcion: timer_hardware_config
Descripcion: Configuro e inicializo el timer
Parametro de entrada: 
                    group: se indica cual de los dos grupos de timer del esp se quiere usar
                            TIMER_GROUP_0
                            TIMER_GROUP_1
                    timer: se indica cual de los timer del grupo elegido se quiere usar
                            TIMER_0
                            TIMER_1 
                    tiempo_us: tiempo en us de cuando se quiere desbordar el contador luego de inicializarlo
                    funcion: direccion al puntero de la funcion que se quiere llamar cuando el timer desborde
No retorna nada
========================================================================*/

void timer_hardware_config (int group, int timer, int tiempo_us, uint32_t funcion)
{
    timer_config_t config = {
            .alarm_en = true,				//Habilito la alarma
            .counter_en = false,			//If the counter is enabled it will start incrementing / decrementing immediately after calling timer_init()
            .intr_type = TIMER_INTR_LEVEL,	        //Is interrupt is triggered on timer’s alarm (timer_intr_mode_t)
            .counter_dir = TIMER_COUNT_UP,	        //Does counter increment or decrement (timer_count_dir_t)
            .auto_reload = true,			//If counter should auto_reload a specific initial value on the timer’s alarm, or continue incrementing or decrementing.
            .divider = 80  			        //Divisor of the incoming 80 MHz (12.5nS) APB_CLK clock. E.g. 80 = 1uS per timer tick
    };

    timer_init(group, timer, &config);                  //Inicio el timer con las configuraciones anteriores.
    timer_set_counter_value(group, timer, 0);           //Reseteo el contador a cero
    timer_set_alarm_value(group, timer, tiempo_us);     //Establesco alarma al tiempo establecido en la funcion
    timer_enable_intr(group, timer);                    //Habilito el timer
    timer_isr_register(group, timer, funcion, NULL, 0, &s_timer_handle[group][timer]); //Indico la funcion a la cual llamar cuando desborde el timer

   esp_err_t err = timer_start(group, timer);
   if(err != ESP_OK){
        printf("error al comenzar timer\r\n");
   }else{
        estado_timer[group][timer]=1;                   //Indico que el timer no esta pausado
   }
   
}

/*========================================================================
Funcion: Pausar_timer
Descripcion: Pausa el timer espesificado
Parametro de entrada: 
                    group: se indica cual de los dos grupos de timer del esp se quiere usar
                            TIMER_GROUP_0
                            TIMER_GROUP_1
                    timer: se indica cual de los timer del grupo elegido se quiere usar
                            TIMER_0
                            TIMER_1 
No retorna nada
========================================================================*/
void Pausar_timer(int group, int timer){        
    if(estado_timer[group][timer]==1){                  //Variable para evitar pausar el timer cuando ya esta pausado
        esp_err_t err = timer_pause(group, timer);
        if(err != ESP_OK){
                printf("error al pausar timer\r\n");
        }
        estado_timer[group][timer]=0;                   //Indico que el timer esta pausado
    }
}


/*========================================================================
Funcion: Comenzar_timer
Descripcion: Comienza el timer espesificado
Parametro de entrada: 
                    group: se indica cual de los dos grupos de timer del esp se quiere usar
                            TIMER_GROUP_0
                            TIMER_GROUP_1
                    timer: se indica cual de los timer del grupo elegido se quiere usar
                            TIMER_0
                            TIMER_1 
No retorna nada
========================================================================*/
void Comenzar_timer(int group, int timer){
    timer_start(group, timer);
    estado_timer[group][timer]=1;                       //Indico que el timer no esta pausado
}