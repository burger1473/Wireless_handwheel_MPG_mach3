/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para comandar Display LCD 20x4 por medio de 4 bits
 *
 * IMPORTANTE: esta libreria usa delays bloqueantes de todo el planificador!!! y Ademas usa seccion critica
 *===========================================================================*/

/*===================================================[ Inclusiones ]===============================================*/
#include "../include/LCD.h"
#include <esp32/rom/ets_sys.h>    //Para retardo bloqueando todo el planificador

/*===================================================[Prototipos de funciones]===============================================*/
void LCD_init(void);
void LCDsendChar(uint8_t Dato);
void LCDsendCommand(uint8_t Dato);
void LCD_EnableChar(void);
void LCD_EnableCmd(void);
void LCDsend_nibble(uint8_t Dat);	//ENVIA EL COMANDO AL LCD
/*===================================================[Variables]===============================================*/
portMUX_TYPE mux2 = portMUX_INITIALIZER_UNLOCKED; //Inicializa el spinlock desbloqueado

/*===================================================[Implementaciones]===============================================*/

/*========================================================================
Funcion: LCD_init
Descripcion: Configura los pines e inicializa el lcd
Parametro de entrada: NULO
No retorna nada
========================================================================*/
void LCD_init(void){

    //Configuro los pines del LCD como salida
    gpio_pad_select_gpio(LCD_pin_RS);
    gpio_set_direction(LCD_pin_RS, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LCD_pin_E);
    gpio_set_direction(LCD_pin_E, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LCD_pin_D4);
    gpio_set_direction(LCD_pin_D4, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LCD_pin_D5);
    gpio_set_direction(LCD_pin_D5, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LCD_pin_D6);
    gpio_set_direction(LCD_pin_D6, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(LCD_pin_D7);
    gpio_set_direction(LCD_pin_D7, GPIO_MODE_OUTPUT);

    //Establesco todos los pines en bajo o cero
    gpio_set_level(LCD_pin_RS, 0);
    gpio_set_level(LCD_pin_E, 1);
    gpio_set_level(LCD_pin_D4, 0);
    gpio_set_level(LCD_pin_D5, 0);
    gpio_set_level(LCD_pin_D6, 0);
    gpio_set_level(LCD_pin_D7, 0);

    ets_delay_us(20000);
    //vTaskDelay(20 / portTICK_PERIOD_MS);        //Espero 20 milisegundo

    gpio_set_level(LCD_pin_RS, 0);              //Pongo en cero para indicar que es mensaje de configuracion y no de escritura de mensaje
 
    //Inicailizacion diaplay
    for(char i=0; i<3; i++){ //envio 3 veces el 0b000011
        LCDsend_nibble(0b0011);
        LCD_EnableCmd();	
        ets_delay_us(5000);
     }

    //Inicailizacion en 4 bits segun hoja de dato https://www.farnell.com/datasheets/50586.pdf
    LCDsend_nibble(0b0010);
    LCD_EnableCmd();
    LCDsend_nibble(0b0010); //Segun pagina 46 de hoja de datos, se debe hacer 2 veces esto y luego establecer NF
    LCD_EnableCmd();

    //Configuro parametros del display

	//LCDsend_nibble(0b0010);LCD_EnableCmd();    //Comando de configuracion de parametro
	LCDsend_nibble(0b1000);LCD_EnableCmd();    //0bNFxx  donde N= numeros de lineas del display (0= 1 lindea  1=2 lineas)
                                               //              F=tipo de matriz de caracter (0= 5x8  1= 5x11)
    
    //Sets entire display (D) on/off, cursor on/off (C), and blinking of cursor position character (B).
    //Tabla 6 pagina 24 PDF HD4
    //Segun hoja de dato pagina 46 pdf hd4 se debe mandar 000000 y luego 001000 por que la hoja de dato busca apagar el display, pero nosotros lo queremos prendido por lo cual le cambiamos el valor a on
    //Entonces seguimos con la instruccion de prender el display
    LCDsend_nibble(0b0000);LCD_EnableCmd();

    //prendemos display
    //de los 4 bits de configuracion, el primer bit se pone en 1 
    //el segundo bit es el D=1 -> display on   D=0 -> display off
    // el tercer bit es el C=1 -> cursor on   c=0 -> cursor off
    //el cuarto bits es el B=1 -> blinking on   B=0 -> blinking off
    //esto se obtuvo de hoja 24 datasheet hd4
    LCDsend_nibble(0b1100);LCD_EnableCmd();

    //Envia Clear display
    //primero se envia 000000
    LCDsend_nibble(0b0000);LCD_EnableCmd();

    //limpio display, para esto se debe colocar 0000 0000 01 segun hoja de datos pagina 24
    LCDsend_nibble(0b0001);LCD_EnableCmd();

    //Envia entry mode set (I/D - S) (Sets cursor move direction specifies display shift These operations are performed during data write and read)
    //primero se envia 000000
    LCDsend_nibble(0b0000);LCD_EnableCmd();

    //segun pagina 46 pdf el primer bit de la configuracion se pone a cero, el segundo a 1 
    //el tercero es  I/D=1 -> INCREMENTE (DDRAM: Display data RAM)  I/D=0->DECREMENT (CGRAM: Character generator RAM)
    //el cuarto es  S=1 -> Accompanies display shift  S/C = 1 -> Display shift  S/C = 0 -> Cursor move
    LCDsend_nibble(0b0110);LCD_EnableCmd();
 
}

void LCD_print(char * str){
    portENTER_CRITICAL(&mux2);                               //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
    unsigned char i = 0;
	while (str[i] !=0)
	{
		LCDsendChar(str[i]);
		i++ ;
	}
    portEXIT_CRITICAL(&mux2);                                //Salgo seccion critica
}

void LCD_print_char(char str){
    portENTER_CRITICAL(&mux2);                               //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
    LCDsendChar(str);
    portEXIT_CRITICAL(&mux2);                                //Salgo seccion critica
}

void LCDGotoXY(uint8_t columna, uint8_t fila)	//POSICIONA EL CURSOR EN X y Y
{
    portENTER_CRITICAL(&mux2);                               //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
	register uint8_t DDRAMAddr;

	switch(fila)
	{
	case 0: DDRAMAddr = LCD_LINE0_DDRAMADDR+columna; break;
	case 1: DDRAMAddr = LCD_LINE1_DDRAMADDR+columna; break;
	case 2: DDRAMAddr = LCD_LINE2_DDRAMADDR+columna; break;
	case 3: DDRAMAddr = LCD_LINE3_DDRAMADDR+columna; break;
	default: DDRAMAddr = LCD_LINE0_DDRAMADDR+columna;
	}
	
	LCDsendCommand(1<<LCD_DDRAM | DDRAMAddr);
    portEXIT_CRITICAL(&mux2);                                //Salgo seccion critica
}


void LCDsend_nibble(uint8_t dat)	//ENVIA EL COMANDO AL LCD
{   
	//Transformo los 1 y 0 del nibble en salidas hacia el display
    if((dat&0x08)  == 0x08){ gpio_set_level(LCD_pin_D7, 1); }else{ gpio_set_level(LCD_pin_D7, 0);}
	if((dat&0x04)  == 0x04){ gpio_set_level(LCD_pin_D6, 1); }else{ gpio_set_level(LCD_pin_D6, 0);} 
    if((dat&0x02)  == 0x02){ gpio_set_level(LCD_pin_D5, 1); }else{ gpio_set_level(LCD_pin_D5, 0);}
    if((dat&0x01)  == 0x01){ gpio_set_level(LCD_pin_D4, 1); }else{ gpio_set_level(LCD_pin_D4, 0);}   	

}

void LCD_EnableCmd(void)
{
    //Doy pulso a enable para indicar que se mando el dato
    gpio_set_level(LCD_pin_E, 1);
    //vTaskDelay(1 / portTICK_PERIOD_MS); //Espero 1 milisegundo
    ets_delay_us(1000);
    gpio_set_level(LCD_pin_E, 0);
    ets_delay_us(1000);
    gpio_set_level(LCD_pin_E, 1);
    //vTaskDelay(1 / portTICK_PERIOD_MS); //Espero 1 milisegundo
}

void LCD_EnableChar(void)
{
    //Doy pulso a enable para indicar que se mando el dato
    gpio_set_level(LCD_pin_E, 1);
    ets_delay_us(100);
    gpio_set_level(LCD_pin_E, 0);
    ets_delay_us(100);
    gpio_set_level(LCD_pin_E, 1);
}

void LCDsendChar(uint8_t Dato)
{
    gpio_set_level(LCD_pin_RS, 1);

	uint8_t DatAux=Dato;
	Dato>>=4;
    LCDsend_nibble(Dato);
    LCD_EnableChar();
	LCDsend_nibble(DatAux);
    LCD_EnableChar();
}

void LCDsendCommand(uint8_t Dato)
{
	gpio_set_level(LCD_pin_RS, 0);

	uint8_t DatAux=Dato;
	Dato>>=4;
    LCDsend_nibble(Dato);
    LCD_EnableCmd();
	LCDsend_nibble(DatAux);
    LCD_EnableCmd();
}

void LCDclr(void)
{	
    portENTER_CRITICAL(&mux2);                               //Seccion critica ya que la mayoria de las variables se modifican en otras tareas o interrupciones
	LCDsendCommand(0b00000001);//Limpiar Display 
	LCDhome();
    portEXIT_CRITICAL(&mux2);                                //Salgo seccion critica
}

void LCDhome(void)
{	
	LCDsendCommand(0b00000010);//Envia el cursor a la posicion inicial	
}

void LCDcursorOn(void)
{	
	LCDsendCommand(0b00001110);//Cursor on
}

void LCDcursorOFF(void)
{	LCDsendCommand(0b00001100);//Cursor oFF
}

void LCDcursorOnBlink(void)
{	
	LCDsendCommand(0b00001111);// Mostrar Cursor + Blink 
}

void LCDblinkOn(void)
{ 	
	LCDsendCommand(0b00001101);//Activa parpadeo //  Blink On
}

void LCDblank(void)
{	
	LCDsendCommand(0b00001000); //APAGA DISPLAY
}

void LCDvisible(void)
{	
	LCDsendCommand(0b00001100);//ENCIENDE DISPLAY	
}

void LCDwriteRight(void)
{	
	LCDsendCommand(0b00000110);//ESCRIBE DE IZQUIERDA A DERECHA 
}
void LCDwriteLeft(void)
{	
	LCDsendCommand(0b00000100);//ESCRIBE DE DERECHA A IZQUIERDA
} 

void LCDcursorLeft(uint8_t n)	//MUEVE EL CURSOR "n" POSICIONES A LA IZQUIERDA
{	
	uint8_t i=0;
	
	for (i=0;i<n;i++)
	{
		LCDsendCommand(0b00010000);//MUEVE EL CURSOR A LA IZQUIERDA  
	}
}
void LCDcursorRight(uint8_t n)	//MUEVE EL CURSOR "n" POSICIONES A LA DERECHA
{
	uint8_t i=0;
	for ( i=0;i<n;i++)
	{
		LCDsendCommand(0b00010100);//MUEVE EL CURSOR A LA DERECHA
	}
}

void LCDshiftLeft(uint8_t n)
{
	uint8_t i=0;
	for (i=0;i<n;i++)
	{
		LCDsendCommand(0b00011000);//DESPLAZA EL TXT DEL LCD A LA IZQUIERDA
	}
}

void LCDshiftRight(uint8_t n)
{
	uint8_t i=0;
	for ( i=0;i<n;i++)
	{
		LCDsendCommand(0b00011100);//DESPLAZA EL TXT DEL LCD A LA DERECHA
	}
}