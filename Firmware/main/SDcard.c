/*=============================================================================
 * Author: Burgos Fabian,Rubiolo Bruno
 * Date: 04/03/2023 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 * Descripcion: Libreria para leer memoria SD
 *===========================================================================*/

/*===================================================[ Inclusiones ]===============================================*/
#include "../include/SDcard.h"

/*===================================================[Prototipos de funciones]===============================================*/

/*===================================================[Variables]===============================================*/
static const char *TAGGG = "SDcard";
bool montado=false;                     //Variable local para indicar si esta montado el sniff
DIR* dir;                               //Direccion del archivo abierto
struct dirent* entry;                   //Valor de entrada en archivo abierto
bool archivo_abirto=false;              //Variable para indicar si el archivo esta abierto
uint8_t numero_list=0;                  //Variable para indicar en la posicion de la lista en la que se esta (sirve para moverse en el explorador de archivos que contiene la SD)

/*===================================================[Implementaciones]===============================================*/

/*========================================================================
Funcion: s_example_write_file
Descripcion: Para escribir archivo en sd
Parametro de entrada:  const char *path: direccion o ruta donde se dese escribir
                       char *data:       dato a escribir
Retorna:  ESP_OK   si se pudo escribir
          ESP_FAIL si no se pudo escribir
========================================================================*/
static esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAGGG, "Opening file %s", path);
    FILE *f = fopen(path, "w");                                 //Abro archivo en modo escritura
    if (f == NULL) {                                            //Si no se pudo abrir como escritura
            ESP_LOGE(TAGGG, "Failed to open file for writing");
            return ESP_FAIL;                                    //Retorno falso
    }
    fprintf(f, data);                                           //Imprimo el dato en el archivo
    fclose(f);                                                  //Cierro archivo
    ESP_LOGI(TAGGG, "File written");

    return ESP_OK;                                              //Retorno OK
}

/*========================================================================
Funcion: s_example_write_file
Descripcion: Para leer archivo en sd
Parametro de entrada:  const char *path: direccion o ruta donde se dese escribir
Retorna:  ESP_OK   si se pudo escribir
          ESP_FAIL si no se pudo escribir
========================================================================*/
static esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAGGG, "Reading file %s", path);
    FILE *f = fopen(path, "r");                              //Abre archivo como lectura
    if (f == NULL) {                                         //Si no se pudo abrir como lectura
        ESP_LOGE(TAGGG, "Failed to open file for reading");
        return ESP_FAIL;                                     //Retorno falso
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];                        //Creo char que va a contener la linea
    fgets(line, sizeof(line), f);                            //Obtengo linea
    fclose(f);                                               //Cierro archivo

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAGGG, "Read from file: '%s'", line);

    return ESP_OK;
}

/*========================================================================
Funcion: SD_abrir_archivo
Descripcion: Abre archivo de la sd si esta montado
Sin parametro de entrada
No retorna nada
========================================================================*/
void SD_abrir_archivo(void){
    if(montado==true){              //Si esta montado
        dir=opendir("/sdcard");     //Abrir directorio raíz    
        archivo_abirto=true;        //Indico que esta abierto el archivo
        return true;                //Retorno verdadero
    }
    return false;                   //Retarno falso
}

/*========================================================================
Funcion: SD_cerrar_archivo
Descripcion: Cierra el archivo de la sd si esta abierto
Sin parametro de entrada
Retorna: true si se pudo cerrar
         false si no se pudo cerrar
========================================================================*/
bool SD_cerrar_archivo(void){
    if(montado==true){                  //Si esta montado
        if(archivo_abirto==true){       //Si esta abierto
            // Cerrar directorio
            closedir(dir);              //Cierra archivo
            archivo_abirto=false;       //Indica que esta cerrado
            return true;                
        }
        return false;
    }
    return false;
}

/*========================================================================
Funcion: SD_contar_lineas_archivo
Descripcion: Cuenta las lineas de un archivo determinado
Parmatreo de entrada: char *nombre: Nombre del archivo al cual se le desean contar las lineas
Retorna: el numero de lineas contadas
========================================================================*/
int16_t SD_contar_lineas_archivo(char *nombre){
    char result[100];                                   //Variable para almacenar la ruta del archivo
    sprintf(result, "%s/%s", MOUNT_POINT, nombre);      //Une la ruta predeterminada con el nombre del archivo a contar lineas.
    FILE* f = fopen(result, "r");                       //Abre el archivo de la ruta anterior
    if (f == NULL) {                                    //Si no existe
        printf("Error opening file\n");
        return 0;                                       //Retorna cero lineas
    }
    char buffer[1024];                                  //Variable que va a contener el texto de la linea
    int16_t line_number = 1;                            //Variable para contar
    while (fgets(buffer, sizeof(buffer), f)) {          //Recorro cada linea del archivo
        //printf("Line %d: %s", line_number, buffer);
        line_number++;                                  //Incremento el contador
    }
    fclose(f);                                          //Cierro archivo
    return (line_number-1);                             //Retorno el numero de lineas contadas
}   

/*========================================================================
Funcion: SD_obtener_linea
Descripcion: Obtiene el contenido de una linea determinada en un archivo especifico
Parmatreo de entrada: char *texto: puntero al buffer donde se desea almacenar el contenido de la linea
                      char *nombre_archivo: puntero al buffer que contiene el nombre del archivo donde se desea buscar el contenido de la x linea.
                      int16_t linea:    numero de linea que se desea obtener
Retorna: true:  si se pudo obterne la linea
         false: si no se pudo obtener la linea
========================================================================*/
bool SD_obtener_linea(char *texto, char *nombre_archivo, int16_t linea){
    char result[100];                                       //Variable para almacenar la ruta del archivo
    sprintf(result, "%s/%s", MOUNT_POINT, nombre_archivo);  //Une la ruta predeterminada con el nombre del archivo a contar lineas.
    FILE* f = fopen(result, "r");                           //Abre el archivo de la ruta anterior
    if (f == NULL) {                                        //Si no existe
        printf("Error opening file\n");
        return false;                                       //Retorna false
    }
    char buffer[1024];                                      //Variable que va a contener el texto de la linea
    memset(buffer, 0, sizeof(buffer));                      //Vacia el buffer
    int16_t line_number = 1;                                //Contador para saber la linea actual
    while (fgets(buffer, sizeof(buffer), f)) {              //Recorro cada linea del archivo
        //printf("Line %d: %s", line_number, buffer);
        if(line_number==linea){                             //Si el numero de linea actual es igual al numero de linea que se quiere
            break;                                          //Sale del while
        }else{                                              //Sino incrementa el numero
            line_number++;
        }
    }
    strcpy(texto, buffer);                                  //Copia el contendio de buffer en el puntero
    fclose(f);                                              //Cierr archivo
    return true;                                            //Retorna verdadero
}

/*========================================================================
Funcion: SD_buscar_enlist
Descripcion: Busca de a uno, los archivos que existen en la sd y retorna su nombre
Parmatreo de entrada: char *nombre: puntero al buffer donde se desea guardar el nombre del archivo encontrado.
                      bool siguiente:   true si se desea buscar el siguiente nombre o false si se desea retornar al anterior.
Retorna: true:  si se pudo
         false: si no se pudo
========================================================================*/
bool SD_buscar_enlist(char *nombre, bool siguiente){
    if(montado==true){                                              //Si esta montado
        if(siguiente==true){numero_list++;}else{numero_list--;}     //Si es siguiente, se incrementa el numero de lista, sino se decrementa
        if(numero_list<1){numero_list=1;}                           
        //Primero obtengo la cantidad de archivos que posee la sd
        uint8_t numeros_existentes=0;
        DIR* dir2=opendir("/sdcard");                               //Abro directorio
        struct dirent* entry2;
        while((entry2=readdir(dir2))!=NULL){                        //Recorro todos los archivos e incremento variable
            numeros_existentes++;
        }
        if(numero_list>numeros_existentes){numero_list=numeros_existentes;} //Si se pide un archivo en una posicion mayor a la que existe, se remplaza por la posicion del ultimo archivo
        closedir(dir2);                                             //Cierro directorio

        //Busco el archivo siguiente o anterior segun el valor de la variable previamente modificada
        DIR* dir=opendir("/sdcard");
        struct dirent* entry;
        bool retrono_int=false;
        for(uint8_t i=0; i<numero_list; i++){                       //Recorro los archivos existente hasta el numero indicado
            if((entry=readdir(dir))!=NULL){                         // Leo nombres de archivos
                if(entry->d_type==DT_REG){                          //Si es tipo de archivo
                    //printf("Nombre de archivo: %s\n", entry->d_name);
                    //nombre=entry->d_name;
                    strcpy(nombre, entry->d_name);                  //Paso el nombre al puntero
                    retrono_int=true;                               //Retorno true
                    //ESP_LOGI(TAGGG, "Nombre de archivo: %s", entry->d_name);
                }
            }else{
                retrono_int=false;
            }
        }
        closedir(dir);
        if(retrono_int==false){strcpy(nombre, "NoFile");}           //Si no hay archivo, indico como NoFile
        return retrono_int;  
    } 
    return false;  
}

/*========================================================================
Funcion: Sd_unmout_list
Descripcion: Desmonta la SF
Sin parametro de entrada
Retorna: true:  si se pudo
         false: si no se pudo 
========================================================================*/
bool Sd_unmout_list(void){
    if(montado==true){                  //si esta montado
        esp_vfs_fat_sdmmc_unmount();    //desmonta
        montado=false;                  //Indico que esta desmontado
        return true;
    }
    return false;
}

/*========================================================================
Funcion: SD_init
Descripcion: Inicializo y configuro SD
Sin parametro de entrada
Retorna 0 si se pudo hacer
        1 Failed to initialize bus 
        2 Failed to mount filesystem
        3 Failed to initialize the card
========================================================================*/
uint8_t SD_init(void){
    ESP_LOGI(TAGGG, "Init SD");
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
    #else
        .format_if_mount_failed = false,
    #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAGGG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ESP_LOGI(TAGGG, "Using SPI peripheral");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = 10000;
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAGGG, "Failed to initialize bus.");
        return 1;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAGGG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAGGG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
             return 2;
        } else {
            ESP_LOGE(TAGGG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
            return 3;
        }
    }
    
    ESP_LOGI(TAGGG, "Filesystem mounted");
    montado=true;
    return 0;
    // Card has been initialized, print its properties
    //sdmmc_card_print_info(stdout, card);
    
}