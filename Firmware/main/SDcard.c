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
bool montado=false;
DIR* dir;
struct dirent* entry;
bool archivo_abirto=false;
uint8_t numero_list=0;

/*===================================================[Implementaciones]===============================================*/

static esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAGGG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAGGG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAGGG, "File written");

    return ESP_OK;
}

static esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAGGG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAGGG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAGGG, "Read from file: '%s'", line);

    return ESP_OK;
}


void SD_abrir_archivo(void){
    if(montado==true){
        // Abrir directorio raíz
        dir=opendir("/sdcard");
        archivo_abirto=true;
        return true;
    }
    return false;
}

bool SD_cerrar_archivo(void){
    if(montado==true){
        if(archivo_abirto==true){
            // Cerrar directorio
            closedir(dir);
            archivo_abirto=false;
            return true;
        }
        return false;
    }
    return false;
}

uint8_t SD_contar_lineas_archivo(char *nombre){
    char result[100];
    sprintf(result, "%s/%s", MOUNT_POINT, nombre);
    FILE* f = fopen(result, "r");
    if (f == NULL) {
        printf("Error opening file\n");
        return 0;
    }
    char buffer[1024];
    int line_number = 1;
    while (fgets(buffer, sizeof(buffer), f)) {
        //printf("Line %d: %s", line_number, buffer);
        line_number++;
    }
    fclose(f);
    return (line_number-1);
}

bool SD_obtener_linea(char *texto, char *nombre_archivo, uint8_t linea){
    char result[100];
    sprintf(result, "%s/%s", MOUNT_POINT, nombre_archivo);
    FILE* f = fopen(result, "r");
    if (f == NULL) {
        printf("Error opening file\n");
        return false;
    }
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int line_number = 1;
    while (fgets(buffer, sizeof(buffer), f)) {
        //printf("Line %d: %s", line_number, buffer);
        if(line_number==linea){
            break;
        }else{
            line_number++;
        }
    }
    strcpy(texto, buffer);
    fclose(f);
    return true;
}

bool SD_buscar_enlist(char *nombre, bool siguiente){
    if(montado==true){
        if(siguiente==true){numero_list++;}else{numero_list--;}
        if(numero_list<1){numero_list=1;}
        uint8_t numeros_existentes=0;
        //Cuento la cantidad de archivos que existen
        // Abrir directorio raíz
        DIR* dir2=opendir("/sdcard");
        struct dirent* entry2;

        // Leer nombres de archivos
        while((entry2=readdir(dir2))!=NULL){
            numeros_existentes++;
        }
        if(numero_list>numeros_existentes){numero_list=numeros_existentes;}
        // Cerrar directorio
        closedir(dir2);

        //Cuento la cantidad de archivos que existen
        // Abrir directorio raíz
        DIR* dir=opendir("/sdcard");
        struct dirent* entry;

        bool retrono_int=false;
        for(uint8_t i=0; i<numero_list; i++){
            // Leer nombres de archivos
            if((entry=readdir(dir))!=NULL){
                if(entry->d_type==DT_REG){              //Si es tipo de archivo
                    //printf("Nombre de archivo: %s\n", entry->d_name);
                    //nombre=entry->d_name;
                    strcpy(nombre, entry->d_name);
                    retrono_int=true;
                    //ESP_LOGI(TAGGG, "Nombre de archivo: %s", entry->d_name);
                }
            }else{
                retrono_int=false;
            }
        }
        closedir(dir);
        if(retrono_int==false){strcpy(nombre, "NoFile");}
        return retrono_int;  
    } 
    return false;  
}

bool Sd_unmout_list(void){
    if(montado==true){
        esp_vfs_fat_sdmmc_unmount();
        montado=false;
        return true;
    }
    return false;
}


void SD_init(void){
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
        return;
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
        } else {
            ESP_LOGE(TAGGG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    
    ESP_LOGI(TAGGG, "Filesystem mounted");
    montado=true;

    // Card has been initialized, print its properties
    //sdmmc_card_print_info(stdout, card);
    /*
    // Abrir directorio raíz
    DIR* dir=opendir("/sdcard");
    struct dirent* entry;

    // Leer nombres de archivos
    while((entry=readdir(dir))!=NULL){
        if(entry->d_type==DT_REG){
            //printf("Nombre de archivo: %s\n", entry->d_name);
            ESP_LOGI(TAGGG, "Nombre de archivo: %s", entry->d_name);
        }
    }
    // Cerrar directorio
    closedir(dir);
    */
    //esp_vfs_fat_sdmmc_unmount();
}