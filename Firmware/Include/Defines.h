/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Encabezado para el manejo de definiciones de los conexionados de los pines
 *
 *===========================================================================*/
//=========================== Definiciones ================================

#define Pin_apagado GPIO_NUM_1

#define LCD_PIN_DEFINE
#define LCD_pin_RS  GPIO_NUM_3
#define LCD_pin_E   GPIO_NUM_21
#define LCD_pin_D4  GPIO_NUM_17
#define LCD_pin_D5  GPIO_NUM_16
#define LCD_pin_D6  GPIO_NUM_4
#define LCD_pin_D7  GPIO_NUM_2

#define SD_PIN_DEFINE
#define PIN_NUM_MISO  GPIO_NUM_19
#define PIN_NUM_MOSI  GPIO_NUM_23
#define PIN_NUM_CLK   GPIO_NUM_18
#define PIN_NUM_CS    GPIO_NUM_5

#define mDNS_DEFINE
#define mDNShostName "MPG000001"

#define Tecla_encendido GPIO_NUM_36
#define Tecla_start GPIO_NUM_12
#define Tecla_stop GPIO_NUM_14
#define Tecla_reset GPIO_NUM_27
#define Tecla_cero GPIO_NUM_32
#define Tecla_eje GPIO_NUM_35
#define Tecla_sube GPIO_NUM_39
#define Tecla_baja GPIO_NUM_34
#define Tecla_pasos GPIO_NUM_33
#define Tecla_fn GPIO_NUM_13

#define PIN_opto  GPIO_NUM_22
#define PIN_encoder_A GPIO_NUM_25 //DT
#define PIN_encoder_B GPIO_NUM_26 //CLK
#define PIN_VBAT ADC2_CHANNEL_3  //pin gpio_15

/*
#define Pin_apagado GPIO_NUM_15

#define LCD_PIN_DEFINE
#define LCD_pin_RS  GPIO_NUM_14
#define LCD_pin_E   GPIO_NUM_27
#define LCD_pin_D4  GPIO_NUM_25
#define LCD_pin_D5  GPIO_NUM_33
#define LCD_pin_D6  GPIO_NUM_32
#define LCD_pin_D7  GPIO_NUM_26

#define SD_PIN_DEFINE
#define PIN_NUM_MISO  GPIO_NUM_19
#define PIN_NUM_MOSI  GPIO_NUM_23
#define PIN_NUM_CLK   GPIO_NUM_18
#define PIN_NUM_CS    GPIO_NUM_5

#define mDNS_DEFINE
#define mDNShostName "MPG000001"

#define Tecla_encendido GPIO_NUM_34
#define Tecla_start GPIO_NUM_2
#define Tecla_stop GPIO_NUM_4
#define Tecla_reset GPIO_NUM_34
#define Tecla_cero GPIO_NUM_2
#define Tecla_eje GPIO_NUM_34
#define Tecla_sube GPIO_NUM_34
#define Tecla_baja GPIO_NUM_34
#define Tecla_pasos GPIO_NUM_34
#define Tecla_fn GPIO_NUM_34

#define PIN_opto  GPIO_NUM_36
#define PIN_encoder_A GPIO_NUM_36
#define PIN_encoder_B GPIO_NUM_36
#define PIN_VBAT ADC1_CHANNEL_6  //pin gpio_34
*/