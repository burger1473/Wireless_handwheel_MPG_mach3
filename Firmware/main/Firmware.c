/*=============================================================================
 * Author: Burgos Fabian, Casas Alejo, Rubiolo Bruno
 * Date: 01/10/2021 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Codigo para controlador inalambrico para mach3
 *
 *NOTA IMPORTANTE: Configurar esquema de particion de la memoria para no tener conflicto frente a codigo extenso
 *      Menuconfig -> Partition Table -> Partition table (.....) -> Seleccionar "Custom partition table cvs"
 *      Menuconfig -> Serial flasher config -> Flash size -> 4 MB
 *      Colocar archivo "partitions.cvs" con el siguiente contenido en la carpeta principal del proyecto.
 *               # Name,   Type, SubType, Offset,  Size, Flags
 *               # Note: if you change the phy_init or app partition offset, make sure to change the offset in Kconfig.projbuild
 *               nvs,      data, nvs,     0x9000,  0x6000,
 *               phy_init, data, phy,     0xf000,  0x1000,
 *               factory,  app,  factory, 0x10000, 2M,
 *      La primera vez que se compile, dara error ya que debe generar la imagen de la particion
 *      Luego la compilacion no generara errores por parte de la particion.
 *      Esto sirve para establecer las direcciones de memoria donde comienza y termina cada stack de memoria
 *      De esta forma en ningun momento se accede a una direccion de memoria prohibida o no existente
 *                            https://www.jianshu.com/p/a09eaf1a37ea
 *      Para borrar los datos almacenados en la eeprom, es necesario borrar los datos en la memoria no volatil (nvs)
 *      Por lo tanto, ejecutar el comando "esptool.py --port COM3 erase_region 0x9000 0x6000" en la consola
 *
 *Configuracion FreeRtos:
 *      CONFIG_FREERTOS_HZ = 1000  para generar un Tick cada 0.001 s
 *===========================================================================*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "soc/soc.h" //disable brownout detector
#include "soc/rtc_cntl_reg.h" //disable brownout detector (deteccion de apagon)
#include "soc/rtc_wdt.h" //disable brownout detector
#include <esp32/rom/ets_sys.h>    //Para retardo bloqueando todo el planificador
#include "../include/Wifi_Conect.h" //Libreria para generar un AP para configurar SSID y Password del WIFI al conectarse
                                    //Luego de detecar el wifi y poder conectarse, funciona en wifi estacion
#include "../include/LCD.h"
#include "../include/ControlMPG.h"
//#include "../include/SDcard.h"


void app_main() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    rtc_wdt_protect_off();
    rtc_wdt_disable();
    //SD_init();

    LCD_init();
    LCDGotoXY(5, 1);
    LCD_print("ControlMPG");
    LCDGotoXY(5, 2);
    LCD_print("Bienvenido");

    ets_delay_us(5000000);
    iniciarWifi();        //Inicio WIFI en modo dual
    ControlMPG_init();
    /*
    LCD_init();
    LCDGotoXY(0, 0);
    LCD_print("--- ---");
    LCDGotoXY(10, 0);
    LCD_print("ControlMPG");
    LCDGotoXY(0, 1);
    LCD_print("X:     +2.6162");
    LCDGotoXY(0, 2);
    LCD_print("Y:     +1.3808");
    LCDGotoXY(0, 3);
    LCD_print("Z:     -1.5034");
    */

    ESP_LOGI(TAG, "Finaliza app_main\n");

}