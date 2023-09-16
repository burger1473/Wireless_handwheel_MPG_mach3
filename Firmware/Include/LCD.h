/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para comandar Display LCD 20x4 por medio de 4 bits
 *===========================================================================*/

 //=========================== Definiciones ================================
 #ifndef LCD_H_
 #define LCD_H_

#ifndef LCD_PIN_DEFINE
 #define LCD_pin_RS  GPIO_NUM_14
 #define LCD_pin_E   GPIO_NUM_27
 #define LCD_pin_D4  GPIO_NUM_25
 #define LCD_pin_D5  GPIO_NUM_33
 #define LCD_pin_D6  GPIO_NUM_32
 #define LCD_pin_D7  GPIO_NUM_26
#endif
 // cursor position to DDRAM mapping
 #define LCD_LINE0_DDRAMADDR		0x00
 #define LCD_LINE1_DDRAMADDR		0x40
 #define LCD_LINE2_DDRAMADDR		0x14
 #define LCD_LINE3_DDRAMADDR		0x54
 #define LCD_CGRAM           6	//DB6: set CG RAM address
 #define LCD_DDRAM           7	//DB7: set DD RAM address
 /*
 #define LCD_pin_RS  18
 #define LCD_pin_RW  17
 #define LCD_pin_E   16
 #define LCD_pin_D4  15
 #define LCD_pin_D5  14
 #define LCD_pin_D6  13
 #define LCD_pin_D7  12
 */

/*===================================================[ Inclusiones ]===============================================*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "Defines.h"

/*==================[Prototipos de funciones]======================*/
void LCD_init(void);
void LCDsendChar(uint8_t Dato);
void LCDGotoXY(uint8_t columna, uint8_t fila);   //Cursor to X Y position empieza de cero
void LCD_print(char * str);
void LCDclr(void);		            //Limpia Display y  Envia el cursor a la posicion inicial - Clears LCD  and send  Cursor home
void LCDhome(void);		            //Envia el cursor a la posicion inicial - LCD cursor home
void LCDcursorOn(void);
void LCDcursorOFF(void);
void LCDcursorOnBlink(void);
void LCDblinkOn(void);
void LCDblank(void);             //apaga display
void LCDvisible(void);  //prende display
void LCDwriteRight(void);
void LCDwriteLeft(void);
void LCDcursorLeft(uint8_t n);
void LCDcursorRight(uint8_t n);
void LCDshiftLeft(uint8_t n);
void LCDshiftRight(uint8_t n);
void LCD_print_char(char str);

//=========================== Variables ================================

#include "../main/LCD.c"
#endif