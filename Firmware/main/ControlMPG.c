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
#include "../include/Interrupcion_gpio.h"
#include "../include/ADC.h"


/*===================================================[Prototipos de funciones]===============================================*/
static void ControlMPG(void *pvParameters);
static char tag3[] = "Control_debug";

/*===================================================[Definiciones]===============================================*/


/*===================================================[Variables]===============================================*/
TaskHandle_t TaskHandle_1 = NULL;
char buffer[54];                   //Sirve para enviar datos por TCP/IP
portMUX_TYPE mux3 = portMUX_INITIALIZER_UNLOCKED; //Inicializa el spinlock desbloqueado
int16_t pasos_encoder=0; //Variable que contiene lo que se incremento el encoder
float pasos=0.0001;      //Para indicar x1 x0.1 x0.01 x0.001 x0.0001
char eje='X';            //Eje actual
const int timeThreshold = 8000;
long timeCounter = 0;

/*===================================================[Implementaciones]===============================================*/

/*========================================================================
Funcion: IRAM_ATTR gpio_isr_handlers
Descripcion: Funcion que se llama por la interrupcion DEL PIN A del encoder.
Sin parametro de entrada
No retorna nada
========================================================================*/
static void IRAM_ATTR gpio_isr_handlers(void* arg)
{
    if (esp_timer_get_time() > timeCounter + timeThreshold)
   {
      if (gpio_get_level(PIN_encoder_A) == gpio_get_level(PIN_encoder_B))
      {
        portENTER_CRITICAL(&mux3); //seccion critica para evitar que se ejecute cambio de contexto mientras se esta realizando la interrpcion
        pasos_encoder++;
        portEXIT_CRITICAL(&mux3);
      }
      else
      {
         portENTER_CRITICAL(&mux3); //seccion critica para evitar que se ejecute cambio de contexto mientras se esta realizando la interrpcion
         pasos_encoder--;
         portEXIT_CRITICAL(&mux3);
      }
      timeCounter = esp_timer_get_time();
   }
}

/*========================================================================
Funcion: IRAM_ATTR gpio_isr_handlers2
Descripcion: Funcion que se llama por la interrupcion DEL PIN AB del encoder.
Sin parametro de entrada
No retorna nada
========================================================================*/
static void IRAM_ATTR gpio_isr_handlers2(void* arg)
{
    if (esp_timer_get_time() > timeCounter + timeThreshold)
   {
      if (gpio_get_level(PIN_encoder_A) == gpio_get_level(PIN_encoder_B))
      {
        portENTER_CRITICAL(&mux3); //seccion critica para evitar que se ejecute cambio de contexto mientras se esta realizando la interrpcion
        pasos_encoder++;
        portEXIT_CRITICAL(&mux3);
      }
      else
      {
         portENTER_CRITICAL(&mux3); //seccion critica para evitar que se ejecute cambio de contexto mientras se esta realizando la interrpcion
         pasos_encoder--;
         portEXIT_CRITICAL(&mux3);
      }
      timeCounter = esp_timer_get_time();
   }
}

/*========================================================================
Funcion: Set_cero
Descripcion: Setea en cero el eje Y
Sin parametro de entrada
No retorna nada
========================================================================*/
void Set_cero(void){
    while (true){
        LCDclr();
        LCDGotoXY(0, 0);
        LCD_print("Set Y to cero");
        LCDGotoXY(0, 1);
        LCD_print("...");
        LCDGotoXY(0, 3);
        LCD_print("Pres x para salir");
        //uint8_t caracter=uart_ReadChar();
        uint8_t caracter='p';
        if(caracter=='x' || gpio_get_level(Tecla_stop)==0){break;}               //Tecla para salir del menu
        if(caracter=='z' || gpio_get_level(PIN_opto)==0){break;}                 //Si el opto detecta, salgo del while
        strcpy(buffer, "N01S0.0001X+0100************************************F"); //Mientras el while siga, envio al mach3 que baje el eje y
        ServerTCP_sendData(buffer, 53);     //Aumento un paso en match3
        vTaskDelay(200/portTICK_PERIOD_MS);
    }

    //Se baja el espesor de la chapa (si se desea )
    while(true){
        LCDclr();
        LCDGotoXY(0, 0);
        LCD_print("Bajar el espesor?");
        LCDGotoXY(0, 2);
        LCD_print("Pres ENTER para si");
        LCDGotoXY(0, 3);
        LCD_print("Pres STOP para no");
        //uint8_t caracter=uart_ReadChar();
        uint8_t caracter='p';
        if(caracter=='x' || gpio_get_level(Tecla_stop)==0){break;}           //Si presiona salir, sale del while
        if(caracter=='s' || gpio_get_level(Tecla_encendido)==0){             //Si se desea bajar
            strcpy(buffer, "N01S0.0001X+1000************************************F"); //Bajo el espesor por mach3
            ServerTCP_sendData(buffer, 53);     //Aumento un paso en match3
            break;                              //Salgo
        }           
        vTaskDelay(300/portTICK_PERIOD_MS);
    }

}


/*========================================================================
Funcion: Cargar_gcode
Descripcion: Carga GCODE
Sin parametro de entrada
No retorna nada
========================================================================*/
void Cargar_gcode(void){
    while(true){
        LCDclr();
        LCDGotoXY(0, 0);
        LCD_print("Seleccione archivo:");
        LCDGotoXY(19, 1);
        LCD_print(">");
        LCDGotoXY(0, 3);
        LCD_print("STOP para salir    <");
        
        //uint8_t caracter=uart_ReadChar();
        uint8_t caracter='p';
        if(caracter=='x' || gpio_get_level(Tecla_stop)==0){break;}           //Aborto tarea y salgo

        char nombree[512];
        if(caracter=='<' || gpio_get_level(Tecla_baja)==0){memset(nombree, 0, sizeof(nombree)); SD_buscar_enlist(nombree, false);} //Busca archivo anterior
        if(caracter=='>' || gpio_get_level(Tecla_sube)==0){memset(nombree, 0, sizeof(nombree)); SD_buscar_enlist(nombree, true); } //Busca archivo siguiente
        LCDGotoXY(0, 2);
        LCD_print(nombree);
    
        if(caracter=='o' || gpio_get_level(Tecla_encendido)==0){            //Si estoy posicionado en el archivo que deseo y preciona enter
            while(true){
                LCDGotoXY(0, 0);
                LCD_print("   Cargando Gcode   ");
                LCDGotoXY(0, 3);
                LCD_print("Pres STOP para salir");
                uint16_t cant_lines=SD_contar_lineas_archivo(nombree);      //Obteiene el numero de lineas de archivo
                if(cant_lines == 0 || cant_lines>65534){                    //Si es 0 o el valor mayimo permitido
                    LCDGotoXY(0, 1);
                    LCD_print("Archivo nulo-grande ");                      //Indico error y salgo al menu
                    vTaskDelay(2000/portTICK_PERIOD_MS);
                    goto end;
                }else{                                                     //Si la cantidad de lines son correctas
                    char result[32];                                       //Para imprimir en display
                    char contenido_linea[1024];                            //Para alamcenar el contenido de cada linea
                    uint32_t N_caracteres_totales=0;                       //Para guardar el numero de caracteres totales del archivo
                    sprintf(buffer, "N03START***************************************%05dF", cant_lines);  //Indico al soft de windows que voy a enviar un gcode  con la cantidad de lineas que posee
                    ServerTCP_sendData(buffer, 53); 
                    //ESP_LOGW(tag3, "%s", buffer);
                    vTaskDelay(200/portTICK_PERIOD_MS);                    //Retardo
                    for(uint8_t j=1; j<cant_lines+1; j++){                 //Recorro cada linea del archivo
                        uint8_t caracter=uart_ReadChar();
                        if(caracter=='x' || gpio_get_level(Tecla_stop)==0){goto end;}           //Si se aborta el proceso, retorno al menu
                        LCDGotoXY(0, 2);
                        sprintf(result, "Linea %d de %d", j, cant_lines);  //Indico la linea que se esta enviando
                        LCD_print(result);
                        memset(contenido_linea, 0, sizeof(contenido_linea)); //seteo la variable en vacio
                        SD_obtener_linea(contenido_linea, nombree, j);     //Obtengo el contenido de la linea

                        //Borro el caracter salto de linea
                        uint8_t q = 0;
                        while(contenido_linea[q] != '\0') {     
                            if(contenido_linea[q] == '\n') {
                                contenido_linea[q] = '\0';
                                break;
                            }
                            q++;
                        }

                        N_caracteres_totales=N_caracteres_totales+strlen(contenido_linea);  //Voy sumando los caracteres totales del archivo
                        
                        ServerTCP_sendData(contenido_linea, strlen(contenido_linea));       //Envio el contenido de la linea
                        //ESP_LOGW(tag3, "%s", contenido_linea); 
                        vTaskDelay(50/portTICK_PERIOD_MS);                                  //Retardo para no saturar el ESP y el servidor tcp
                    }

                    vTaskDelay(200/portTICK_PERIOD_MS);                                     //Una vez que finalizo, espero un tiempo
                    sprintf(buffer, "N03FIN***********************************L%010dF", N_caracteres_totales);  //Envio fin de gcode con la cantidad de caracteres para comprobar que se envio correcto
                    ServerTCP_sendData(buffer, 53);
                    //ESP_LOGW(tag3, "%s", buffer);
                    //printf("FIN");
                    //Espero recepcion de mensaje por servidor indicando que se recibio correctamente el archivo gcode
                    LCDGotoXY(0, 2);
                    LCD_print("Verificando...      ");
                    uint8_t timeout=0;
                    while(true){
                        uint8_t len=ServerTCP_leermensaje(buffer);                          //Leo mensaje
                        if(len>0){                                                          //Si el tamañod el mesnaje es mayor a cero
                            if(len==53){                                                    //Si el mensaje contiene 53 caracteres
                                if(buffer[0]=='N' && buffer[1]=='5' && buffer[2]=='3' && buffer[3]=='C' && buffer[52]=='F'){  //Si el mensaje es de codigo g comprabado correctamente
                                    LCDGotoXY(0, 2);
                                    LCD_print("Cargado correctament");
                                    vTaskDelay(3000/portTICK_PERIOD_MS);
                                    goto end;                                               //Regreso al menu
                                }
                            }
                        }
                        timeout++;                                                          //Timeout de verificacion
                        vTaskDelay(300/portTICK_PERIOD_MS);
                        if(timeout>4){
                            LCDGotoXY(0, 2);
                            LCD_print("Error al pasar gcode");
                            vTaskDelay(3000/portTICK_PERIOD_MS);
                            goto end;                                                       //Regreso al menu
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

/*========================================================================
Funcion: Apagar
Descripcion: Apaga el dispositivo
Sin parametro de entrada
No retorna nada
========================================================================*/
void Apagar(void){
    LCDclr();
    LCDGotoXY(6, 1);
    LCD_print("Apagando");
    LCDGotoXY(5, 2);
    LCD_print("ControlMPG");
    vTaskDelay(3000/portTICK_PERIOD_MS) ;      // Delay para retardo del contador 
    gpio_set_level(Pin_apagado, 0);     //Apago enclavamiento de mosfet
}

/*========================================================================
Funcion: Menu
Descripcion: Funcion para moverse sobre el menu
Sin parametro de entrada
No retorna nada
========================================================================*/
void menu(void){                    
    uint8_t i=1;                                                        //Posicion del menu actual
    while(true){
        //uint8_t caracter=uart_ReadChar();
        uint8_t caracter='p';
        if(caracter=='x' || gpio_get_level(Tecla_stop)==0){break;}     //Tecla para salir del menu
        if(caracter=='<' || gpio_get_level(Tecla_baja)==0){i--;}       //Tecla para bajar en el menu
        if(caracter=='>' || gpio_get_level(Tecla_sube)==0){i++;}       //Tecla para subir en el menu
        if(i<1){i=5;}  if(i>5){i=1;}                                   //Limites Posicion
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
        
        if(caracter=='o' || gpio_get_level(Tecla_encendido)==0){            //Si se presiona enter, segun el valor del menu se va a la funcion especifica
            if(i==1){Set_cero();}
            if(i==2){Cargar_gcode();}
            if(i==4){Apagar();}
            if(i==5){break;}
        }
        vTaskDelay(100/portTICK_PERIOD_MS) ;
    }
}

/*========================================================================
Funcion: Teclado_init
Descripcion: Inicializa los pines del teclado como entradas
Sin parametro de entrada
No retorna nada
========================================================================*/
void Teclado_init(void){
    //Configuro entradas
    gpio_pad_select_gpio(Tecla_encendido);
	gpio_set_direction(Tecla_encendido, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_start);
	gpio_set_direction(Tecla_start, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_stop);
	gpio_set_direction(Tecla_stop, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_reset);
	gpio_set_direction(Tecla_reset, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_cero);
	gpio_set_direction(Tecla_cero, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_eje);
	gpio_set_direction(Tecla_eje, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_baja);
	gpio_set_direction(Tecla_baja, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_sube);
	gpio_set_direction(Tecla_sube, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_pasos);
	gpio_set_direction(Tecla_pasos, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(Tecla_fn);
	gpio_set_direction(Tecla_fn, GPIO_MODE_INPUT);
	//gpio_set_pull_mode(Entrada[j], GPIO_PULLDOWN_ONLY);
}

/*========================================================================
Funcion: Obtener_teclado
Descripcion: Obtiene las teclas presionadas y ejecuta la accion determianda
Sin parametro de entrada
No retorna nada
========================================================================*/
void Obtener_teclado(void){

    //uint8_t caracter=uart_ReadChar();
    uint8_t caracter='p';
    if(caracter=='o' || gpio_get_level(Tecla_encendido)==0){                        //Si se preciona enter, se abre el menu
        LCDclr();
        LCDGotoXY(6, 1);
        LCD_print("Cargando");
        LCDGotoXY(8, 2);
        LCD_print("Menu");
        vTaskDelay(1500/portTICK_PERIOD_MS) ;      // Delay para retardo del contador 
        LCDclr();
        menu();
        LCDclr();
        LCDGotoXY(6, 1);
        LCD_print("Saliendo");
        LCDGotoXY(8, 2);
        LCD_print("Menu");
        vTaskDelay(1500/portTICK_PERIOD_MS) ;      // Delay para retardo del contador 
        LCDclr();
    }

    //Envio comando a mach3 segun corresponda la tecla presionada

    if(gpio_get_level(Tecla_start)==0){
        if(gpio_get_level(Tecla_fn)==0){
            sprintf(buffer, "N02M1*********************************************M1F");
            ServerTCP_sendData(buffer, 53);
        }else{
            sprintf(buffer, "N02Start***************************************StartF");
            ServerTCP_sendData(buffer, 53);
        }
    }

    if(gpio_get_level(Tecla_stop)==0){
        if(gpio_get_level(Tecla_fn)==0){
            sprintf(buffer, "N02M2*********************************************M2F");
            ServerTCP_sendData(buffer, 53);
        }else{
            sprintf(buffer, "N02Stop*****************************************StopF");
            ServerTCP_sendData(buffer, 53);
        }
    }

    if(gpio_get_level(Tecla_reset)==0){
       if(gpio_get_level(Tecla_fn)==0){     //Tecla rewind
            sprintf(buffer, "N02Rewind*************************************RewindF");
            ServerTCP_sendData(buffer, 53);
       }else{
            sprintf(buffer, "N02Reset***************************************ResetF");
            ServerTCP_sendData(buffer, 53);
       }
    }

    if(gpio_get_level(Tecla_cero)==0){
       if(gpio_get_level(Tecla_fn)==0){
            sprintf(buffer, "N02HOME*****************************************HOMEF");
            ServerTCP_sendData(buffer, 53);
       }else{
        if(eje=='X'){
            sprintf(buffer, "N02ZeroX***************************************ZeroXF");
            ServerTCP_sendData(buffer, 53);
        }
        if(eje=='Y'){
            sprintf(buffer, "N02ZeroY***************************************ZeroYF");
            ServerTCP_sendData(buffer, 53);
        }
        if(eje=='Z'){
            sprintf(buffer, "N02ZeroZ***************************************ZeroZF");
            ServerTCP_sendData(buffer, 53);
        }
        if(eje=='A'){
            sprintf(buffer, "N02ZeroA***************************************ZeroAF");
            ServerTCP_sendData(buffer, 53);
        }
       }
    }

    if(gpio_get_level(Tecla_eje)==0){
       if(gpio_get_level(Tecla_fn)==0){
            sprintf(buffer, "N02M3*********************************************M3F");
            ServerTCP_sendData(buffer, 53);
       }else{               //Cambio el eje actual
        if(eje=='X'){
            eje='Y';
        }else{
            if(eje=='Y'){
                eje='Z';
            }else{
                if(eje=='Z'){
                    eje='A';
                }else{
                    if(eje=='A'){
                        eje='X';
                    }
                }
            }
        }
       }
    }

    if(gpio_get_level(Tecla_pasos)==0){
       if(gpio_get_level(Tecla_fn)==0){
            sprintf(buffer, "N02M4*********************************************M4F");
            ServerTCP_sendData(buffer, 53);
       }else{                                  //Cambio los pasos
            pasos=pasos*10;     
            if(pasos>=10){pasos=0.0001;}
       }
    }
    
}

/*========================================================================
Funcion: ControlMPG_init
Descripcion: Inicializa y crea la tarea encargada de procesar las acciones del control. Inicializa  teclado, sd, uart, encoder, adc.
Sin parametro de entrada
No retorna nada
========================================================================*/
void ControlMPG_init(void){
    esp_err_t ret = xTaskCreate(&ControlMPG, "ControlMPG", 7072, NULL, 2, &TaskHandle_1); //Creo tarea
    if (ret != pdPASS)  {
        ESP_LOGE(tag3, "Error al crear tarea ControlMPG. Reiniciando ESP"); 
        esp_restart();
    }

    Teclado_init();
    SD_init();
    //uart_init();
    gpio_pad_select_gpio(PIN_encoder_A);
    gpio_set_direction(PIN_encoder_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_encoder_A, GPIO_PULLUP_ONLY);
    config_gpio_como_int(PIN_encoder_A, GPIO_INTR_ANYEDGE, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE, &gpio_isr_handlers); //Activa interrupcion del pin de deteccion de cruce por cero. Funcion  gpio_isr_handlers
    gpio_pad_select_gpio(PIN_encoder_B);
    gpio_set_direction(PIN_encoder_B, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_encoder_B, GPIO_PULLUP_ONLY);
    config_gpio_como_int(PIN_encoder_B, GPIO_INTR_ANYEDGE, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE, &gpio_isr_handlers2); //Activa interrupcion del pin de deteccion de cruce por cero. Funcion  gpio_isr_handlers
    //Configuro los canales ADC para realizar mediciones
    config_adc(2, PIN_VBAT, ADC_WIDTH_12Bit, ADC_ATTEN_6db); //Canal 1 para leer tension bateria
}

/*========================================================================
Funcion: ControlMPG
Descripcion: Tarea del control
Sin parametro de entrada
No retorna nada
========================================================================*/
static void ControlMPG(void *pvParameters) {
    if(ServerTCP_socket_init(1)==false){ESP_LOGE(tag3, "Error al iniciar socket. Reiniciando ESP"); esp_restart();}             //Inicializa socket
    
    while (1) {
        //Lee voltaje bateria
        uint16_t lectura=Leer_adc(2, PIN_VBAT, 5, ADC_WIDTH_12Bit);
        double voltaje_local=((lectura*3.3)/4095.0)*11; //Obtengo el valor de tension en el adc

        //Envia valor del encoder al mach3
        uint8_t pasos_encoder_local=0;
        char signo_local='+'; 
        portENTER_CRITICAL(&mux3); //seccion critica para evitar que se ejecute cambio de contexto mientras se esta realizando la interrpcion
        if(pasos_encoder!=0){
            if(pasos_encoder<0){
                pasos_encoder_local=pasos_encoder*-1;
                signo_local='-';
            }else{
                pasos_encoder_local=pasos_encoder;
                signo_local='+';
            }
            pasos_encoder=0;
        }
        portEXIT_CRITICAL(&mux3);
        if(pasos_encoder_local>0){
            if (pasos_encoder_local>9999){pasos_encoder_local=9999;}
            sprintf(buffer, "N01S%06.4f%c%c%04d************************************F", pasos, eje, signo_local, pasos_encoder_local);
            //strcpy(buffer, "N01S0.0001X+1000************************************F");
            if(ServerTCP_sendData(buffer, 53)==true){ }
        }
        
                
        //Pide informacion de los EJE al mach3
        bool conectado=false;
        strcpy(buffer, "N0451*********************************************51F");
        if(ServerTCP_sendData(buffer, 53)==true){    //Si se pudo enviar
            conectado=true;
            //LCDclr();
            LCDGotoXY(0, 0);
            LCD_print("C-- ---");
            //LCDGotoXY(10, 0);
            //LCD_print("ControlMPG");
            uint8_t len=ServerTCP_leermensaje(buffer);
            //Muestro los valores de los ejes en el display
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
            //LCDGotoXY(10, 0);
            //LCD_print("ControlMPG");
            LCDGotoXY(0, 1);
            LCD_print("X:     +-.----");
            LCDGotoXY(0, 2);
            LCD_print("Y:     +-.----");
            LCDGotoXY(0, 3);
            LCD_print("Z:     +-.----");
            LCDcursorOFF();
        }
        
        //char texto[6];
        char *texto;
        texto = (char*) malloc(7*sizeof(char));    //Asigno memoria dinamica

        sprintf(buffer, "%c", eje);  
        LCDGotoXY(10, 0);
        LCD_print(buffer);

        portENTER_CRITICAL(&mux3); //seccion critica para evitar que se ejecute cambio de contexto mientras se esta realizando la interrpcion
        sprintf(buffer, "%.4f", pasos);
        portEXIT_CRITICAL(&mux3);
        LCDGotoXY(14, 0);
        LCD_print(buffer);
        free(texto);                                //Libero memoria dinamica

        Obtener_teclado();                        //Lee los pulsadores
        
        vTaskDelay(50/portTICK_PERIOD_MS) ;      // Delay para retardo del contador 
    }
}