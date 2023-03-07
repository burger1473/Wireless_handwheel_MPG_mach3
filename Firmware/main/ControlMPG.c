/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para realizar las activaders del control MPG
 *
 *===========================================================================*/

/*===================================================[ Inclusiones ]===============================================*/
#include "../include/ControlMPG.h"
#include <esp32/rom/ets_sys.h>    //Para retardo bloqueando todo el planificador
#include "../include/LCD.h"
#include "../include/Wifi_ServerTCP.h"
#include "../include/SDcard.h"

/*===================================================[Prototipos de funciones]===============================================*/
static void ControlMPG(void *pvParameters);
static char tag3[] = "Control_debug";

/*===================================================[Definiciones]===============================================*/
#define MUX_pin_1 GPIO_NUM_15
#define MUX_pin_2 GPIO_NUM_2
#define MUX_pin_3 GPIO_NUM_4

/*===================================================[Variables]===============================================*/
TaskHandle_t TaskHandle_1 = NULL;

/*===================================================[Implementaciones]===============================================*/

void Teclado_init(void){
    //Configuro entradas
    gpio_pad_select_gpio(MUX_pin_1);
	gpio_set_direction(MUX_pin_1, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(MUX_pin_2);
	gpio_set_direction(MUX_pin_2, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(MUX_pin_3);
	gpio_set_direction(MUX_pin_3, GPIO_MODE_INPUT);

	//gpio_set_pull_mode(Entrada[j], GPIO_PULLDOWN_ONLY);
}

void Obtener_teclado(void){
    uint8_t valor=0;
    if(gpio_get_level(MUX_pin_1)==1){valor=valor|0b001;} //DEC =1
    if(gpio_get_level(MUX_pin_2)==1){valor=valor|0b010;} //DEC =2
    if(gpio_get_level(MUX_pin_3)==1){valor=valor|0b100;} //DEC =4

    char buffer[54];

    switch (valor) {
        case 0:
            
            break;
        case 1:
            strcpy(buffer, "N0451*********************************************51F");
            ServerTCP_sendData(buffer, 53);
            break;
        case 2:
            strcpy(buffer, "N0451*********************************************51F");
            ServerTCP_sendData(buffer, 53);
            break;
        case 3:
            strcpy(buffer, "N0451*********************************************51F");
            ServerTCP_sendData(buffer, 53);
            break;
        case 4:
            strcpy(buffer, "N0451*********************************************51F");
            ServerTCP_sendData(buffer, 53);
            break;
        case 5:
            strcpy(buffer, "N0451*********************************************51F");
            ServerTCP_sendData(buffer, 53);
            break;
        case 6:
            strcpy(buffer, "N0451*********************************************51F");
            ServerTCP_sendData(buffer, 53);
            break;
        case 7:
            strcpy(buffer, "N0451*********************************************51F");
            ServerTCP_sendData(buffer, 53);
            break;
        default:
            break;
    }
}

void ControlMPG_init(void){

    esp_err_t ret = xTaskCreate(&ControlMPG, "ControlMPG", 5072, NULL, 2, &TaskHandle_1); //Creo tarea
    if (ret != pdPASS)  {
        ESP_LOGE(tag3, "Error al crear tarea ControlMPG. Reiniciando ESP"); 
        esp_restart();
    }
    Teclado_init();
    SD_init();
}

static void ControlMPG(void *pvParameters) {
    if(ServerTCP_socket_init(1)==false){ESP_LOGE(tag3, "Error al iniciar socket. Reiniciando ESP"); esp_restart();}
    
    while (1) {
        bool conectado=false;
        char buffer[54];
        strcpy(buffer, "N0451*********************************************51F");
        if(ServerTCP_sendData(buffer, 53)==true){    //Si se pudo enviar
            conectado=true;
            //LCDclr();
            LCDGotoXY(0, 0);
            LCD_print("C-- ---");
            LCDGotoXY(10, 0);
            LCD_print("ControlMPG");
            uint8_t len=ServerTCP_leermensaje(buffer);
            if(len>0){
            
               // ESP_LOGW(tag3, "Leng MPG %d bytes: %s", len, buffer);
                
                if(len==53){
                    //Si el dato es informacion de ejes, las muestro en el LCD
                    //N51E+00000.0000x+00000.0000y+00000.0000z+00000.0000aF
                    //ESP_LOGW(tag3, "SI 53");
                    //ESP_LOGW(tag3, "0: %c  1: %c 2: %c 3: %c 53: %c", buffer[0], buffer[1], buffer[2], buffer[3], buffer[53]);
                    if(buffer[0]=='N' && buffer[1]=='5' && buffer[2]=='1' && buffer[3]=='E' && buffer[52]=='F'){
                       // ESP_LOGW(tag3, "SI ACA2");
                        LCDGotoXY(0, 1);
                        LCD_print("X:");
                        LCDGotoXY(0, 2);
                        LCD_print("Y:");
                        LCDGotoXY(0, 3);
                        LCD_print("Z:");
                        
                        //borro los cero del numerador
                        for(uint8_t i=0; i<3; i++){
                            if(buffer[8+12*i]=='0' && buffer[7+12*i]=='0' && buffer[6+12*i]=='0' && buffer[5+12*i]=='0'){
                                buffer[8+12*i]=buffer[4+12*i];//Imprimo signo
                                buffer[7+12*i]=' ';
                                buffer[6+12*i]=' ';
                                buffer[5+12*i]=' ';
                                buffer[4+12*i]=' ';
                            }else{
                                if(buffer[7+12*i]=='0' && buffer[6+12*i]=='0' && buffer[5+12*i]=='0'){
                                    buffer[7+12*i]=buffer[4+12*i];//Imprimo signo
                                    buffer[6+12*i]=' ';
                                    buffer[5+12*i]=' ';
                                    buffer[4+12*i]=' ';
                                }else{
                                    if(buffer[6+12*i]=='0' && buffer[5+12*i]=='0'){
                                        buffer[6+12*i]=buffer[4+12*i];//Imprimo signo
                                        buffer[5+12*i]=' ';
                                        buffer[4+12*i]=' ';
                                    }else{
                                        if(buffer[5+12*i]=='0'){
                                            buffer[5+12*i]=buffer[4+12*i];//Imprimo signo
                                            buffer[4+12*i]=' ';
                                        }
                                    }
                                }
                            }
                        }

                        for(uint8_t i=0; i<11; i++){
                            LCDGotoXY(3+i, 1);
                            LCD_print_char(buffer[4+i]);
                            LCDGotoXY(3+i, 2);
                            LCD_print_char(buffer[16+i]);
                            LCDGotoXY(3+i, 3);
                            LCD_print_char(buffer[28+i]);
                        }
                        
                    }
                    //Si el dato es de informacion de proceso
                    //N02EL00:00ES00:00NL0000LA0000***********************F
                    if(buffer[0]=='N' && buffer[1]=='5' && buffer[2]=='2' && buffer[3]=='E' && buffer[53]=='F'){
                    
                    }
                }
            }
            
            LCDcursorOFF();
        }else{
            conectado=false;
        }
       
        if(conectado==false){
            LCDclr();
            LCDGotoXY(0, 0);
            LCD_print("NC- ---");
            LCDGotoXY(10, 0);
            LCD_print("ControlMPG");
            LCDGotoXY(0, 1);
            LCD_print("X:     +-.----");
            LCDGotoXY(0, 2);
            LCD_print("Y:     +-.----");
            LCDGotoXY(0, 3);
            LCD_print("Z:     +-.----");
            LCDcursorOFF();
        }
        
        Obtener_teclado();
        
        vTaskDelay(200/portTICK_PERIOD_MS) ;      // Delay para retardo del contador 
    }
}