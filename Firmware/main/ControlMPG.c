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
#include "../include/Uart.h"


/*===================================================[Prototipos de funciones]===============================================*/
static void ControlMPG(void *pvParameters);
static char tag3[] = "Control_debug";

/*===================================================[Definiciones]===============================================*/


/*===================================================[Variables]===============================================*/
TaskHandle_t TaskHandle_1 = NULL;
char buffer[54];                   //Sirve para enviar datos por TCP/IP

/*===================================================[Implementaciones]===============================================*/

void Set_cero(void){
    while (true){
        LCDclr();
        LCDGotoXY(0, 0);
        LCD_print("Set Y to cero");
        LCDGotoXY(0, 1);
        LCD_print("...");
        LCDGotoXY(0, 3);
        LCD_print("Pres x para salir");
        uint8_t caracter=uart_ReadChar();
        if(caracter=='x'){break;}           //Tecla para salir del menu
        if(caracter=='z'){break;}           //Entrada opto
        strcpy(buffer, "N01S0.0001X+0100************************************F");
        ServerTCP_sendData(buffer, 53);     //Aumento un paso en match3
        vTaskDelay(200/portTICK_PERIOD_MS);
    }

    while(true){
        LCDclr();
        LCDGotoXY(0, 0);
        LCD_print("Bajar el espesor?");
        LCDGotoXY(0, 2);
        LCD_print("S para si");
        LCDGotoXY(0, 3);
        LCD_print("x para no");
        uint8_t caracter=uart_ReadChar();
        if(caracter=='x'){break;}           //Tecla para salir del menu
        if(caracter=='s'){  
            strcpy(buffer, "N01S0.0001X+1000************************************F");
            ServerTCP_sendData(buffer, 53);     //Aumento un paso en match3
            break;                          //Tecla para salir del menu
        }           
        vTaskDelay(300/portTICK_PERIOD_MS);
    }

}


void Cargar_gcode(void){
    while(true){
        LCDclr();
        LCDGotoXY(0, 0);
        LCD_print("Seleccione archivo:");
        LCDGotoXY(19, 1);
        LCD_print(">");
        LCDGotoXY(0, 3);
        LCD_print("Pres x para salir  <");
        
        uint8_t caracter=uart_ReadChar();
        if(caracter=='x'){break;}           //Tecla para salir del menu

        char nombree[512];
        if(caracter=='<'){memset(nombree, 0, sizeof(nombree)); SD_buscar_enlist(nombree, false);} //Busca archivo anterior
        if(caracter=='>'){memset(nombree, 0, sizeof(nombree)); SD_buscar_enlist(nombree, true); } //Busca archivo siguiente
        LCDGotoXY(0, 2);
        LCD_print(nombree);

        if(caracter=='o'){
            while(true){
                LCDGotoXY(0, 0);
                LCD_print("   Cargando Gcode");
                LCDGotoXY(0, 3);
                LCD_print("Pres x para salir");
                uint8_t cant_lines=SD_contar_lineas_archivo(nombree);
                if(cant_lines == 0){
                    LCDGotoXY(0, 1);
                    LCD_print("No hay archivos");
                    vTaskDelay(2000/portTICK_PERIOD_MS);
                    goto end;
                }else{
                    //char result[32];
                    memset(buffer, '*', sizeof(buffer));
                    char contenido_linea[1024];
                    char num_final[11];
                    sprintf(num_final, "%04d", cant_lines);
                    buffer[48]=num_final[0];  buffer[49]=num_final[1];   buffer[50]=num_final[2]; buffer[51]=num_final[3];
                    buffer[0]='N';  buffer[1]='0';  buffer[2]='3'; buffer[7]='C'; buffer[46]='N'; buffer[47]='f'; buffer[52]='F';  
                        
                    for(uint8_t j=1; j<cant_lines+1; j++){
                        uint8_t caracter=uart_ReadChar();
                        if(caracter=='x'){goto end;}           //Tecla para salir del menu
                        LCDGotoXY(0, 2);
                        //sprintf(result, "Linea %d de %d", j, cant_lines);
                        //LCD_print(result);
                        memset(contenido_linea, 0, sizeof(contenido_linea));
                        SD_obtener_linea(contenido_linea, nombree, j);
                        
                        char num_actual[11];
                        sprintf(num_actual, "%04d", j);
                        buffer[3]=num_actual[0];  buffer[4]=num_actual[1];  buffer[5]=num_actual[2]; buffer[6]=num_actual[3];
                        
                        //Borro el caracter salto de linea
                        uint8_t q = 0;
                        while(contenido_linea[q] != '\0') {
                            if(contenido_linea[q] == '\n') {
                                contenido_linea[q] = '\0';
                                break;
                            }
                            q++;
                        }

                        uint8_t N_caracteres=strlen(contenido_linea);
                        
                                                
                        if(N_caracteres>38){               //Si es mayor, parto el mensaje en 2
                            buffer[46]='A';
                            for(uint8_t h=0; h<38; h++){
                                buffer[8+h]=contenido_linea[h];
                            }
                            ServerTCP_sendData(buffer, 53);
                            ESP_LOGW(tag3, "%s", buffer); 
                            vTaskDelay(15/portTICK_PERIOD_MS);
                            buffer[46]='F';
                            for(uint8_t p=0; p<38; p++){
                                if(p+38<N_caracteres){  buffer[8+p]=contenido_linea[p+38];}
                                if(p+38>=N_caracteres){ buffer[8+p]='*';}
                            }
                            ServerTCP_sendData(buffer, 53);
                            ESP_LOGW(tag3, "%s", buffer); 
                        }else{                  //Si esta dentro del protocolo, mando directo
                            buffer[46]='N';
                            for(uint8_t p=0; p<38; p++){
                                if(p<N_caracteres){ buffer[8+p]=contenido_linea[p];}
                                if(p>=N_caracteres){ buffer[8+p]='*';}
                            }
                            ServerTCP_sendData(buffer, 53);
                            ESP_LOGW(tag3, "%s", buffer); 
                        }   
                        vTaskDelay(15/portTICK_PERIOD_MS);
                    }
                    printf("FIN");
                    LCDGotoXY(0, 2);
                    LCD_print("Verificando...");
                    uint8_t timeout=0;
                    while(true){
                        timeout++;
                        vTaskDelay(100/portTICK_PERIOD_MS);
                        if(timeout>4){
                            LCDGotoXY(0, 2);
                            LCD_print("Error al pasar gcode");
                            vTaskDelay(1000/portTICK_PERIOD_MS);
                            goto end;
                        }
                    }
                }
                
            }
            end:
            break; 
        }           //Tecla para salir del menu

        vTaskDelay(300/portTICK_PERIOD_MS);
    }
}

void Apagar(void){
    gpio_set_level(Pin_apagado, 0);
}

void menu(void){
    uint8_t i=1;
    while(true){
        uint8_t caracter=uart_ReadChar();
        if(caracter=='x'){break;}           //Tecla para salir del menu
        if(caracter=='<'){i--;}
        if(caracter=='>'){i++;}
        if(i<1){i=5;}  if(i>5){i=1;}
        LCDclr();
        LCDGotoXY(8, 0);
        LCD_print("Menu");
        LCDGotoXY(19, 1);
        LCD_print(">");
        LCDGotoXY(19, 3);
        LCD_print("<");
        LCDGotoXY(0, 2);
        if(i==1){LCD_print("Set cero");}
        if(i==2){LCD_print("Cargar gcode");}
        if(i==3){LCD_print("Name: "); LCD_print(mDNShostName);}
        if(i==4){LCD_print("Apagar dispositivo");}
        if(i==5){LCD_print("Volver");}
        
        if(caracter=='o'){
            if(i==1){Set_cero();}
            if(i==2){Cargar_gcode();}
            if(i==4){Apagar();}
            if(i==5){break;}
        }
        vTaskDelay(300/portTICK_PERIOD_MS) ;
    }
}

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

    uint8_t caracter=uart_ReadChar();

    if(caracter=='o'){
        menu();
    }

    uint8_t valor=0;
    if(gpio_get_level(MUX_pin_1)==1){valor=valor|0b001;} //DEC =1
    if(gpio_get_level(MUX_pin_2)==1){valor=valor|0b010;} //DEC =2
    if(gpio_get_level(MUX_pin_3)==1){valor=valor|0b100;} //DEC =4

    

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
    uart_init();
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