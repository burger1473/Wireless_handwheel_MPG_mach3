/*==================[ Inclusiones ]============================================*/
#include "../include/Interrupcion_gpio.h"

/*==================[Prototipos de funciones]======================*/
void config_gpio_como_int(gpio_num_t GPIO_INPUT_IO,gpio_int_type_t MODO, gpio_pullup_t pull_up,  gpio_pulldown_t pull_down, uint32_t funcion);
void desactivar_int(gpio_num_t GPIO_INPUT_IO);
void activar_int(gpio_num_t GPIO_INPUT_IO, gpio_int_type_t MODO);
/*=======================[Definiciones]================================*/
#define ESP_INTR_FLAG_DEFAULT 0



/*==================[Implementaciones]=================================*/

/*========================================================================
Funcion: desactivar_int
Descripcion: Desactiva la interupcion del pin determinado
Parametro de entrada: pin al cual se quiere desactivar la interrupcion
No retorna nada
========================================================================*/
void desactivar_int(gpio_num_t GPIO_INPUT_IO){
    gpio_set_intr_type(GPIO_INPUT_IO, GPIO_INTR_DISABLE);
}

/*========================================================================
Funcion: activar_int
Descripcion: Activa la interupcion del pin determinado
Parametro de entrada: pin al cual se quiere activar la interrupcion
No retorna nada
========================================================================*/
void activar_int(gpio_num_t GPIO_INPUT_IO, gpio_int_type_t MODO){
    gpio_set_intr_type(GPIO_INPUT_IO, MODO);
}


/*========================================================================
Funcion: config_gpio_como_int
Descripcion: Configura un pin determinado como interrupcion
Parametro de entrada: 
                    GPIO_INPUT_IO:  pin al cual se quiere asignar interrupcion
                    MODO: modo en el que se quiere llamar a la interrupcion:
                           GPIO_INTR_DISABLE    = Deshabilitar la interrupción GPIO
                           GPIO_INTR_POSEDGE    = flanco ascendente
                           GPIO_INTR_NEGEDGE    = flanco descendente
                           GPIO_INTR_ANYEDGE    = flanco ascendente y descendente
                           GPIO_INTR_LOW_LEVE   = disparador de nivel bajo de entrada
                           GPIO_INTR_HIGH_LEVEL = disparador de alto nivel de entrada
                    pull_up: establece si se activa la resistencia de pull up en el pin
                           GPIO_PULLUP_DISABLE
                           GPIO_PULLUP_ENABLE
                    pull_down: establece si se activa la resistencia de pull down en el pin  
                           GPIO_PULLDOWN_DISABLE
                           GPIO_PULLDOWN_ENABLE   
                    funcion: Direccion al puntero de la funcion a la cual se desea llamar cuando se dispare la interrupcion      
No retorna nada
========================================================================*/
void config_gpio_como_int(gpio_num_t GPIO_INPUT_IO, gpio_int_type_t MODO, gpio_pullup_t pull_up,  gpio_pulldown_t pull_down, uint32_t funcion)
{
    gpio_config_t io_conf;
    //Desabilita interrupcion
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //Establece el modo de interrupcion
    io_conf.intr_type = MODO;
    //Emascara el bit de la interrupcion para no alterar configuraciones previas de otros pines
    io_conf.pin_bit_mask = (1ULL<<GPIO_INPUT_IO);
    //Selecciona el pin como modo de entrada  
    io_conf.mode = GPIO_MODE_INPUT;
    //Habilita o no la resistencia pull up
    io_conf.pull_up_en = pull_up;
    //Habilita o no la resistencia pull down
    io_conf.pull_down_en = pull_down;

    esp_err_t ret = gpio_config(&io_conf);
    if(ret != ESP_OK) {
        printf("error, gpio_config failed\r\n");
    }
    //cambiar el tipo de intrusión gpio por un pin
    gpio_set_intr_type(GPIO_INPUT_IO, MODO);
 
 
    //inicia el servicio gpio isr
    ret = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    if(ret != ESP_OK) {
        printf("error, gpio_install_isr_service failed\r\n");
    }
    //Manipulador de disparo isr para pin gpio específico
    ret = gpio_isr_handler_add(GPIO_INPUT_IO, funcion, (void*) GPIO_INPUT_IO);
    if(ret != ESP_OK) {
        printf("error, gpio_isr_handler_add failed\r\n");
    }
 

}