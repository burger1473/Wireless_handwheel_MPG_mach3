/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para periferico uart
 *===========================================================================*/

//=========================== Definiciones ================================

/*===================================================[ Inclusiones ]===============================================*/
#include "../include/Uart.h"

/*===================================================[ Imlementacion ]===============================================*/

/*========================================================================
Funcion: uart_init
Descripcion: Inicializa el periferico uart
Sin parametro de entrada
No retorna nada
========================================================================*/
void uart_init(void){
    // Configurar la UART
    uart_config_t uart_config = {                   //Genera variable de configuracion
        .baud_rate = 115200,                        //Configura baud en 115200
        .data_bits = UART_DATA_8_BITS,              //8 bit de dato
        .parity = UART_PARITY_DISABLE,              //Sin bit de paridad
        .stop_bits = UART_STOP_BITS_1,              //1 bit de stop
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);    //Configura el periferico uart con la configuracion anterior
    //Configura pines y driver:
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
}

/*========================================================================
Funcion: uart_ReadChar
Descripcion: Lee un caracter por uart
Sin parametro de entrada
Retorna el caracter leido o ' '  si no se pudo leer
========================================================================*/
uint8_t uart_ReadChar(void){
    uint8_t data[1024];
    memset(data, 0, sizeof(data));
    int len = uart_read_bytes(UART_NUM_0, data, sizeof(data), 20 / portTICK_RATE_MS);
    if (len > 0) {
        return data[0];
    }
    return ' ';
}