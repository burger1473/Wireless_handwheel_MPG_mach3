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
void uart_init(void){
    // Configurar la UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);

}

uint8_t uart_ReadChar(void){
    uint8_t data[1024];
    memset(data, 0, sizeof(data));
    int len = uart_read_bytes(UART_NUM_0, data, sizeof(data), 20 / portTICK_RATE_MS);
    if (len > 0) {
        return data[0];
    }
    return ' ';
}