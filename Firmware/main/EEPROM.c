/*=============================================================================
 * Author: Burgos Fabian, Casas Alejo, Rubiolo Bruno
 * Date: 01/10/2021 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
*===========================================================================*/

/*==================[ Inclusiones ]============================================*/
#include "../include/EEPROM.h"

/*==================[Prototipos de funciones]======================*/
void begin_eeprom(void);
int8_t read_eeprom(char key_dir[]);
char * read_array_eeprom(char key_dir[]);
void write_eeprom(char key_dir[], int8_t val);
void write_array_eeprom(char key_dir[], char value[]) ;

//=========================== Variables ================================
nvs_handle my_handle;   //Variable para apuntador
esp_err_t err;          //Variable del tipo error

/*==================[Implementaciones]=================================*/

/*========================================================================
Funcion: begin_eeprom
Descripcion: Inicia la memoria no volatil
Parametro de entrada: NULO
No retorna nada
========================================================================*/
void begin_eeprom(void) {
    err = nvs_flash_init();                 //Inicia memoria no volatil
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) { //Si falla
        ESP_ERROR_CHECK(nvs_flash_erase()); //La partición NVS se truncó y debe borrarse
        err = nvs_flash_init();             //Vuelva a intentar nvs_flash_init
    }
    ESP_ERROR_CHECK( err );                 //Verifica error
}



/*========================================================================
Funcion: read_eeprom
Descripcion: Lee un byte del tipo int8
Parametro de entrada:
                    char key_dir[]= llave o referencia (posicion) donde se quiere leer el dato almacenado en eeprom de tipo int8
Retorna int8_t value retorno el dato leio en la eeprom o caso contrario retorna 0
========================================================================*/
int8_t read_eeprom(char key_dir[]){
    int8_t value = 0; // El valor predeterminado será 0, si aún no se ha configurado en NVS
    err = nvs_open("Eeprom", NVS_READWRITE, &my_handle); //Abro la memoria volatil como etiqueta "Eeprom" y lo referencio en la variable my_handle
    if (err != ESP_OK) {                                 //Si ocurrio error
        printf("Error (%d) opening NVS!\n", err);   
    } else {                                            //Si esta todo bien

        esp_err_t err =  nvs_get_i8(my_handle, key_dir, &value); //obtengo en valor refernciado con key_dir y lo almaceno en value
        switch (err) {                                  
            case ESP_OK:
                printf("Done\n");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%d) reading!\n", err);
        }
    }
  
  nvs_close(my_handle); //Cierro la memoria volatil abierta con la etiqueta "Eeprom" la cual esta referenciada en my_handle
  
  return value;
}



/*========================================================================
Funcion: read_array_eeprom
Descripcion: Lee un array de bytes del tipo int8
Parametro de entrada:
                    char key_dir[]= llave o referencia (posicion) donde se quiere leer el dato almacenado en eeprom de tipo int8
Retorna char * buf: retorno el dato leio en la eeprom o caso contrario retorna "ESP"
========================================================================*/
char * read_array_eeprom(char key_dir[]){
    char * buf;                     //Variable donde se va a almacenar el dato leido
    buf = (char *) malloc (64);     //Seteo una memoria reservada de 64
    strcpy (buf, "ESP");            //Seteo como valor predeterminado la cadena "ESP" por si falla la lectura o si no existe dato en esa posicion de memoria eeprom

    err = nvs_open("Eeprom", NVS_READWRITE, &my_handle); //Abro la memoria volatil como etiqueta "Eeprom" y lo referencio en la variable my_handle
    if (err != ESP_OK) {                                 //Si ocurrio error
        printf("Error (%d) opening NVS!\n", err);
    } else {                                             //Si esta todo bien

        size_t required_size;                                  //varibale para almacenar el tamaño del array
        nvs_get_str(my_handle, key_dir, NULL, &required_size); //Obtengo el tamaño del array que esta referenciado con la llave key_dir en la memoria EEPROM
        char* dato = malloc(required_size);                    //Creo variable dato con un tamaño igual al obtenido en el paso anterior para luego almacenar el array 
        esp_err_t err = nvs_get_str(my_handle, key_dir, dato, &required_size); //Almaceno en dato el array leido en la eeprom con la referencia key_dir
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                strcpy (buf, dato);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%d) reading!\n", err);
        }
        nvs_close(my_handle);   //Cierro la memoria volatil abierta con la etiqueta "Eeprom" la cual esta referenciada en my_handle
        
    }
  return buf;  //retorno el dato leio en la eeprom o caso contrario retorna "ESP"
}




/*========================================================================
Funcion: write_eeprom
Descripcion: Escribe un byte del tipo int8 en la referencia key_dir
Parametro de entrada:
                    char key_dir[]= llave o referencia (posicion) donde se quiere leer el dato almacenado en eeprom de tipo int8
                    int8_t value= valor del tipo int8 a almacenar en la memora no volatil con referencia key_dir
No retorna nada
========================================================================*/
void write_eeprom(char key_dir[], int8_t value) {
    err = nvs_open("Eeprom", NVS_READWRITE, &my_handle); //Abro la memoria volatil como etiqueta "Eeprom" y lo referencio en la variable my_handle
    if (err != ESP_OK) {                                 //Si ocurrio error
        printf("Error (%d) opening NVS!\n", err);   
    }else{                                               //Si todo esta bien
        err = nvs_set_i8(my_handle, key_dir, value);     //Almaceno variable value del tipo int8 con referencia key_dir
        printf((err != ESP_OK) ? "Failed!\n" : "");
        
        err =  nvs_commit(my_handle);                    //Guardo cambios
        printf((err != ESP_OK) ? "Failed!\n" : "");
        nvs_close(my_handle);                            //Cierro la memoria volatil abierta con la etiqueta "Eeprom" la cual esta referenciada en my_handle
    }
}




/*========================================================================
Funcion: write_array_eeprom
Descripcion: escribe un array de bytes del tipo char en la referencia key_dir
Parametro de entrada:
                    char key_dir[]= llave o referencia (posicion) donde se quiere leer el dato almacenado en eeprom de tipo int8
                    char value[]= valor del tipo char a almacenar en la memora no volatil con referencia key_dir
No retorna nada
========================================================================*/
void write_array_eeprom(char key_dir[], char value[]) {
    err = nvs_open("Eeprom", NVS_READWRITE, &my_handle); //Abro la memoria volatil como etiqueta "Eeprom" y lo referencio en la variable my_handle
    if (err != ESP_OK) {                                 //Si ocurrio error
        printf("Error (%d) opening NVS!\n", err);
    }else{                                               //Si todo esta bien
        err = nvs_set_str(my_handle, key_dir, value);    //Almaceno arreglo value del tipo char con referencia key_dir
        printf((err != ESP_OK) ? "Failed!\n" : "");
        
        err =  nvs_commit(my_handle);                    //Guardo cambios
        printf((err != ESP_OK) ? "Failed!\n" : "");
        nvs_close(my_handle);                            //Cierro la memoria volatil abierta con la etiqueta "Eeprom" la cual esta referenciada en my_handle
    }
}
