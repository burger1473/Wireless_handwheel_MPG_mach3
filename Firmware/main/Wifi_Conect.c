/*=============================================================================
 * Author: Burgos Fabian, Casas Alejo, Rubiolo Bruno
 * Date: 01/10/2021 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
 *
 *ACLARACION: Esta libreria tiene 3 modos de funcionamiento:
 *
 *            Modo Estacion: Se conecta al Wifi declarado en Wifi_Conect.h
 *            Modo AP:       Genera una red Wifi con el nombre y contraseña declarados en Wifi_Conect.h
 *                           Genera un Web server con el codigo HTML establecido en html.h
 *            Modo DUAL:     Obtiene de la memoria EEPROM los datos de la red a la cual decea conectarse en modo estacion
 *                           Si no logra conectarse luego de x intentos, resetea el esp y en el proximo inicio genera un AP
 *                           Este AP genera un Web Server en 192.168.4.1 con el codigo HTML de html_conectar_wifi.h
 *                           Este codigo HTML es una web donde se puede ingresar la red y contraseña a la cual se quiere conectar en mdoo estacion
 *                           Cuando el usuario establece la red en la web generada, el esp32 almacena estos valores en la EEPROM y resetea el esp
 *                           Luego del reseteo, el micro comienza como Estacion e intenta conectarse a los datos antes ingresados en la EEPROM.
 *
 *            En este trabajo se utiliza este ultimo modo.
 *
 *IMPORTANTE: para evitar sobrecargar la memoria del micro, se utilizo ifdefined para que el comilador solo comile lo que necesite para el modo seleccionado.
 *===========================================================================*/


/*===================================================[ Inclusiones ]===============================================*/

#include "../include/Wifi_Conect.h"
#include "../include/ControlMPG.h"
#include "../include/Wifi_ServerTCP.h"

/*===================================================[Prototipos de funciones]===============================================*/
void iniciarWifi(void);

#if  defined (MODE_ESTACION) || defined (MODE_DUAL) //Si se declaro que esta en modo estacion o modo dual
void iniciarWifi_Modo_estacion(void);
static void eventHandler_station(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
#endif

#if  defined (MODE_AP) || defined (MODE_DUAL) //Si se declaro que esta en modo AP o modo dual
void iniciarWifi_Modo_AP(void);
static httpd_handle_t start_webserver(void);
static esp_err_t principal_get_handler(httpd_req_t *req);
static esp_err_t mostrarPagina(httpd_req_t *req);
static void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_softap(void);
#endif


/*===================================================[Variables]===============================================*/
static EventGroupHandle_t s_wifi_event_group; //FreeRTOS event group para señalar cuando está conectado
static int s_retry_num = 0;
const static char *TAG = "WIFI_debug";

#if  defined (MODE_ESTACION) || defined (MODE_DUAL)
int valor = 0;
#endif
#if  defined (MODE_AP) || defined (MODE_DUAL)
//establece a que funcion llamar cuando se tipea la pagina ip/
static const httpd_uri_t principal = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = principal_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL 
};
#endif

/*===================================================[Implementaciones]===============================================*/

/*========================================================================
Funcion: iniciarWifi
Descripcion: Inicia el wifi ya sea en modo AP o Estacion segun corresponda
Parametro de entrada: NULO
No retorna nada
========================================================================*/
void iniciarWifi(void){

#ifdef MODE_DUAL //Si se declaro que esta en modo DUAL
begin_eeprom(); //Inicializo la memoria EEPROM para luego utilizarla para obtener los datos a los cuales se decea coenctar en modo estacion
if(error_estacion==1){ //Si error_estacion es 1 (Esta variable es un tipo de variable que no se borra el dato al reiniciar el micro)
iniciarWifi_Modo_AP(); //Inicio en modo AP para configurar el SSID y password de conexion en modo Estacion
}else{ //Si error_estacion es cero (en estado por defecto comienza en este valor)
iniciarWifi_Modo_estacion(); //Inicia en modo Estacion, intenta conectarse a la red que esta guardada en la EEPROM
                             //Si no se puede conectar luego de x intentos, establece error_estacion en 1 y reinicia el micro
}
#endif

#ifdef MODE_ESTACION  //Si se declaro que esta en modo estacion, solo inicio el modo estacion
iniciarWifi_Modo_estacion();
#endif

#ifdef MODE_AP //Si se declaro que esta en modo AP, solo inicio el modo AP
iniciarWifi_Modo_AP();
#endif
}



/*===================================================[Si esta en modo estacion o dual]===============================================*/

#if  defined (MODE_ESTACION) || defined (MODE_DUAL)
/*========================================================================
Funcion: iniciarWifi_Modo_estacion
Descripcion: Inicia el wifi en modo estacion
Parametro de entrada: NULO
No retorna nada
========================================================================*/
void iniciarWifi_Modo_estacion(void)
{
    esp_err_t ret = nvs_flash_init(); //Inicializa el almacenamiento no volatil (NVS)
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { //Verifica que se inicie correctamente
      ESP_ERROR_CHECK(nvs_flash_erase());//Si no se inicio, limpia la memoria no volatil
      ret = nvs_flash_init();            //Intenta iniciarlo nuevamente
    }
    ESP_ERROR_CHECK(ret);               //Verifica si hay errores

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");                 //Especifigo en el logger que estoy en modo Estacion
    s_wifi_event_group = xEventGroupCreate();          //Crea un nuevo grupo de eventos RTOS y devuelve un identificador mediante el cual se puede hacer referencia al grupo de eventos recién creado (event_groups.c)
    ESP_ERROR_CHECK(esp_netif_init());                 //Inicializa el stack TCP/IP 
    ESP_ERROR_CHECK(esp_event_loop_create_default()); //Crea un event loop

    //Crea WIFI STA por defecto. En caso de cualquier error de inicialización, esta API se cancela.
    //La API crea el objeto esp_netif con la configuración predeterminada de la estación WiFi, adjunta el netif a wifi y registra los controladores de wifi predeterminados.
    esp_netif_create_default_wifi_sta(); 

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //Parámetros de configuración del WiFi stack por defecto, por defecto establece Puerto=80
    
    //Inicia el recurso WiFi Alloc para el controlador WiFi, como la estructura de control WiFi, el búfer RX / TX, la estructura WiFi NVS, etc.
    //También inicia la tarea WiFi.
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); //Inicia el controlador Wifi con los datos de configuraciones generados antes

    esp_event_handler_instance_t instance_any_id, instance_got_ip;

    //Registra una instancia del controlador de eventos en el bucle predeterminado.
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,            //Especifica que es evento de WIFI
                                                        ESP_EVENT_ANY_ID,      //ID del evento (sin ID en este caso)
                                                        &eventHandler_station, //Direccion de la funcion a la que debe invocar en cada evento generado
                                                        NULL,                  //Argumento para enviar a la funcion
                                                        &instance_any_id));    //Es posible registrar el mismo controlador de eventos varias veces y produce distintos objetos de instancia. Si no es necesario cancelar el registro, pero el controlador debe eliminarse cuando se elimina el bucle de eventos, la instancia puede ser NULL.

    //Registra una instancia del controlador de eventos en el bucle predeterminado.
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,               //Especifica que es evento de IP
                                                        IP_EVENT_STA_GOT_IP,    //ID del evento
                                                        &eventHandler_station,  //Direccion de la funcion a la que debe invocar en cada evento generado
                                                        NULL,                   //Argumento para enviar a la funcion
                                                        &instance_got_ip));     //Es posible registrar el mismo controlador de eventos varias veces y produce distintos objetos de instancia. Si no es necesario cancelar el registro, pero el controlador debe eliminarse cuando se elimina el bucle de eventos, la instancia puede ser NULL.

   //En lo siguiente se configura el ssid y pass de la red wifi a la cual debe conectarse
   //para esto se pueden compilar dos codigo, uno para modo dual y otro para modo Estacion

   //Si esta en modo dual, obtiene los datos desde la memoria EEPROM
    #ifdef MODE_DUAL
        wifi_config_t wifi_config = {
            .sta = {
                .ssid = "",          //Nombre de la red vacio
                .password = "",      //Contraseña vacio
                .threshold.authmode = WIFI_AUTH_WPA2_PSK, //tipo de autenticación
                .pmf_cfg = {
                    .capable = true, //Anuncia la compatibilidad con Protected Management Frame (PMF). El dispositivo preferirá conectarse en modo PMF si otro dispositivo también anuncia la capacidad PMF.
                    .required = false //Anuncia que se requiere Protected Management Frame (PMF). El dispositivo no se asociará a dispositivos que no sean compatibles con PMF.
                },
            },
        };

        char * p=read_array_eeprom("SSID"); //Obtiene el array almacenado en la llave "SSID" de la memoria la eeprom
        for (int i=0;i<=strlen(p)-1;i++){   //Reemplazamos los caracteres '+' por espacio, ya que los espacios se reciben como '+' por medio del sitio web de configuracion y por eso se guardan con '+' en lugar de ' ' en la eeprom
                if (p[i]=='+')              //Si es el caracter '+'
                    p[i]=' ';               //Se cambia por un espacio, es decir ' '
	        }
        strcpy(&WIFI_SSID, p);              //Copia el valor leido de la EEPROM en WIFI_SSID
        free (p);                           //Libera la memoria utilizada en P
        char * j=read_array_eeprom("PASS"); //Obtiene el array almacenado en la llave "PASS" de la memoria la eeprom
        if(j[0]=='+'){
            strcpy(&WIFI_PASS, "");              //Copia el valor leido de la EEPROM en WIFI_PASS
        }else{
            strcpy(&WIFI_PASS, j);              //Copia el valor leido de la EEPROM en WIFI_PASS
        }
        free (j);                                       //Libera la memoria utilizada en j
        strcpy(&wifi_config.sta.ssid, &WIFI_SSID);     //Copia WIFI_SSID en la variable de la estructura wifi_config donde corresponde
        strcpy(&wifi_config.sta.password, &WIFI_PASS);//Copia WIFI_SSID en la variable de la estructura wifi_config donde corresponde
    #endif

    //Si esta en modo Estacion, obtiene los datos desde los defines de Wifi_conect.h
    #ifdef MODE_ESTACION
        wifi_config_t wifi_config = {
            .sta = {
                .ssid = WIFI_SSID,          //Nombre de la red declarada en el define de Wifi_conect.h
                .password = WIFI_PASS,      //Contraseña declarada en el define de Wifi_conect.h
                .threshold.authmode = WIFI_AUTH_WPA2_PSK, //tipo de autenticación
                .pmf_cfg = {
                    .capable = true, //Anuncia la compatibilidad con Protected Management Frame (PMF). El dispositivo preferirá conectarse en modo PMF si otro dispositivo también anuncia la capacidad PMF.
                    .required = false //Anuncia que se requiere Protected Management Frame (PMF). El dispositivo no se asociará a dispositivos que no sean compatibles con PMF.
                },
            },
        };
    #endif

    ServerTCP_configwifi();

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) ); //Configura el modo STA (tambien puede ser AP o STA+AP)
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) ); //Configura el WiFI segun el modo seleccionado
    ESP_ERROR_CHECK(esp_wifi_start() ); //Inicia el Wifi
    
    //ServerTCP_configmDNS();
    

    /*
    Espera hasta que se establezca la conexión (WIFI_CONNECTED_BIT)
    o la conexión falle para el número máximo de reintentos (WIFI_FAIL_BIT).
    Los bits son establecidos por eventHandler ()
    */

    //Lee bits dentro de un grupo de eventos RTOS , ingresando opcionalmente al estado Bloqueado (con un tiempo de espera) para esperar a que se establezca un bit o grupo de bits.
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,  //Especifico el grupo de eventos (el que se genero antes)
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,                 //Especifico los bist que espero
            pdFALSE,                                            //xClearOnExit
            pdFALSE,                                            //xWaitForAllBits
            portMAX_DELAY);                                     //Ticks que espera bloqueado, en este caso espera indefinidamente

    /* xEventGroupWaitBits() devuelve los bits antes de que se devolviera la llamada,
    por lo que podemos probar qué evento sucedió realmente. */
    if (bits & WIFI_CONNECTED_BIT) {  //Si se conecto
        ESP_LOGI(TAG, "Conectado al AP SSID:%s password:%s",
                 WIFI_SSID, WIFI_PASS);
        ControlMPG_init();
    } else if (bits & WIFI_FAIL_BIT) { //Si fallo la conexion
        ESP_LOGI(TAG, "Fallo al conectar al AP SSID:%s, password:%s",
                 WIFI_SSID, WIFI_PASS);
        //Si fallo la conexion y esta en modo DUAL, establesco error_estacion en 1 y reinicio el micro para que comience en modo AP para configurar el wifi
        #ifdef MODE_DUAL
            ESP_LOGI(TAG,"Reiniciando micro para que comience como modo AP para configurar WIFI");
            error_estacion=1;
            esp_restart(); //Reinicio ESP
        #endif

    } else { //Si no fue ninguno de los eventos declarados anteriormente
        ESP_LOGE(TAG, "Evento inesperado");
    }

    /* El evento no se procesará después de cancelar el registro. */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip)); //Anula el registro del controlador de bucle de eventos del sistema.
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));  //Anula el registro del controlador de bucle de eventos del sistema.
    vEventGroupDelete(s_wifi_event_group); //Elimina el grupo de evento.
}



/*========================================================================
Funcion: eventHandler_station
Descripcion: Controla los eventos sucedidos en modo estacion y realiza lo 
             que corresponde con cada evento en especifico
Parametro de entrada: 
                    void* arg= argumentos que recibe el evento
                    esp_event_base_t event_base= tipo de evento sucedio
                    int32_t event_id= id que recibe el evento
                    void* event_data= dato que recibe el evento
No retorna nada
========================================================================*/
static void eventHandler_station(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) { //Si fue un evento de Wifi y el id se refiere a un evento de inicio de modo estacion (Logro conectarse al SSID especificado)
        esp_wifi_connect(); //conecta el ESP al AP elegido (Wifi elegido)
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) { //Si fue un evento de Wifi y el id se refiere a un evento de desconexion de modo estacion (No logro conectarse al SSID especificado)
        if (s_retry_num < MAXIMUM_RETRY) { //Si no se supero el numero de intentos establecido
            esp_wifi_connect();            //Vuelve a intentar conectar el Wifi
            s_retry_num++;                 //Incrementa la cantidad de intenos
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT); //Si supero la cantidad de intentos, llamo al grupo de eventos y le mando el bit de fallo
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) { //Si es un evento de IP y el id se refiere a la ip del modo estacion
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;         //Obtiene el dato que entrega el evento
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));         //Transforma el tipo de dato de la ip a string y lo muestra en el logger
        s_retry_num = 0;                                                    //Reinicia el contador de intentos de conexion
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);         //Llama al grupo de eventos en el evento wifi y envia el bit de conexion.
    }
}
#endif


/*===================================================[Si esta en modo AP o dual]===============================================*/
#if  defined (MODE_AP) || defined (MODE_DUAL)

/*========================================================================
Funcion: iniciarWifi_Modo_AP
Descripcion: Inicia el modo AP.
Parametro de entrada: NULL
No retorna nada
========================================================================*/
void iniciarWifi_Modo_AP(void){
    esp_err_t ret = nvs_flash_init(); //Inicializa la memoria no volatil
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { //Si ocurrio un error en la inicializacion
      ESP_ERROR_CHECK(nvs_flash_erase()); //Limpia la memora flash no volatil
      ret = nvs_flash_init();             //Vuelve a intentar inicializar la memoria no volatil
    }

    ESP_ERROR_CHECK(ret);                 //Verifica si existen errores

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");    //Indico en el logger que estoy en modo AP
    wifi_init_softap();                   //Inicio el WIFI en modo AP

    static httpd_handle_t server = NULL;  //Variable para almacener el handle del server,Tipo static para poder acceder desde otra funciones

    esp_netif_init(); //Inicialice la pila de TCP / IP subyacente. (esp_netif.h)
 
    //Registra controladores de eventos para detener el servidor cuando se desconecta Wi-Fi y reinicia al conectarse
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server)); //Establece que cuando sucede un evento del tipo obtencion IP en server, llame a la funcion connect_handler
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server)); //Establece que cuando sucede un evento del tipo desconexion IP en server, llame a la funcion disconnect_handler
 
    server = start_webserver(); //Inicia el servidor por primera vez
}




/*========================================================================
Funcion: wifi_event_handler
Descripcion: Evento de wifi
Parametro de entrada: 
                    void* arg= argumentos que recibe el evento
                    esp_event_base_t event_base= tipo de evento sucedio
                    int32_t event_id= id que recibe el evento
                    void* event_data= dato que recibe el evento
No retorna nada
========================================================================*/
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {                                               //Si el evento es de conexion en AP
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;       //Obtiene las datos recibidos
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);                                              //Muestra la mac
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {                                     //Si el evento es de desxonecion de AP
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data; //Obtiene las datos recibidos
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);                                              //Muestra la mac
    }
}





/*========================================================================
Funcion: wifi_init_softap
Descripcion: Inicia el WIFI en modo AP
Parametro de entrada: NULL
No retorna nada
========================================================================*/
void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());                //Inicia la pila de TCP / IP subyacente. y Verifica error
    ESP_ERROR_CHECK(esp_event_loop_create_default()); //Crea evento en bucle loop y verifica errores.
    esp_netif_create_default_wifi_ap();               //Crea AP WIFI predeterminado. En caso de cualquier error de inicialización, esta API se cancela.

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //Configuro el wifi con valores por defecto
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                //Inicio el wifi con las configuraciones

    //Establece que cuando sucede un evento del tipo WIFI_EVENT llame a la funcion wifi_event_handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,  
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));


    wifi_config_t wifi_config = {                     //Arreglo de configuracion
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID_AP,             //Configuro SSID establecido en define dentro Wifi_Conect.h
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID_AP), //Tamaño del SSID
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,          //Selecciono canal establecido en define dentro Wifi_Conect.h
            .password = EXAMPLE_ESP_WIFI_PASS_AP,         //Configuro contraseña de la red, este dato esta establecido en define dentro Wifi_Conect.h
            .max_connection = EXAMPLE_MAX_STA_CONN,       //Indico la cantidad maxima de conexiones posibles, establecido en define dentro Wifi_Conect.h
            .authmode = WIFI_AUTH_WPA_WPA2_PSK            //Indico el tipo de autenticacion
        },
    };

    if (strlen(EXAMPLE_ESP_WIFI_PASS_AP) == 0) {          //Si la contaseña establecida en define dentro Wifi_Conect.h es nula
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;         //Creo red abierta
    }
    
    nvs_flash_init();
    tcpip_adapter_init();
    //esp_event_loop_init(wifi_event_handler, NULL);
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    //esp_wifi_init(&wifi_init_config);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);

 
    ServerTCP_configwifi();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));               //Selecciono el wifi en modo AP y verifico errores
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config)); //Configuro el AP y verifico errores
    ESP_ERROR_CHECK(esp_wifi_start());                              //Cominezo el Wifi en modo AP
    ServerTCP_configmDNS();
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID_AP, EXAMPLE_ESP_WIFI_PASS_AP, EXAMPLE_ESP_WIFI_CHANNEL);
}




/*========================================================================
Funcion: start_webserver
Descripcion: Comienza el servidor web
Parametro de entrada: NULL
Retorna:
        Puntero del evento del servidor
        Null
========================================================================*/
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;                   //Variable que apunta al evento del server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); //Configura con valores por defecto al server
 
    // Comienza el server httpd
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {      //Si el server comienza correctamente
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &principal); //Registro el evento cuando se llama al server con la url principal ("/") Esto se establecio en este archivo en el aparado de variables, arriba de todo (principal)
        ServerTCP_configmDNS();
        ServerTCP_mDNS_addService(80);
        ControlMPG_init();
        return server; //Retorno el puntero del evento del servidor
    }
 
    ESP_LOGI(TAG, "Error starting server!");
    return NULL; //Si fallo, retorno NULL
}
 



/*========================================================================
Funcion: stop_webserver
Descripcion: Para el servidor web
Parametro de entrada: NULL
No retorna nada
========================================================================*/
static void stop_webserver(httpd_handle_t server)
{
    httpd_stop(server); // Frena el server httpd
}
 


/*========================================================================
Funcion: disconnect_handler
Descripcion: Evento de desconexion
Parametro de entrada: 
                    void* arg= argumentos que recibe el evento
                    esp_event_base_t event_base= tipo de evento sucedio
                    int32_t event_id= id que recibe el evento
                    void* event_data= dato que recibe el evento
No retorna nada
========================================================================*/
static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg; //Recive el puntero del server
    if (*server) {                                  //Si existe el handle server
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);                    //Para el servidor Web
        *server = NULL;                             //Establece el handle como nulo
    }
}
 


/*========================================================================
Funcion: connect_handler
Descripcion: Evento de conexion
Parametro de entrada: 
                    void* arg= argumentos que recibe el evento
                    esp_event_base_t event_base= tipo de evento sucedio
                    int32_t event_id= id que recibe el evento
                    void* event_data= dato que recibe el evento
No retorna nada
========================================================================*/
static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg; //Recive el puntero del server
    if (*server == NULL) {                          //Si existe el handle server
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();                //Establece el handle como el valor que retorna start_webserver e inicia el servidor web
    }
}


/*========================================================================
Funcion: connect_handler
Descripcion: Evento de controlador HTTP GET
Parametro de entrada: 
                    httpd_req_t *req= recibe el handler del server
Retorna             ESP_OK
========================================================================*/
static esp_err_t principal_get_handler(httpd_req_t *req)
{
   ESP_LOGI(TAG, "Pagina principal");
   return mostrarPagina(req);
}




/*========================================================================
Funcion: http_404_error_handler
Descripcion: Evento de controlador de errores 404 en HTTP GET 
Parametro de entrada: 
                    httpd_req_t *req= recibe el handler del server
                    httpd_err_code_t err= recibe el codigo de error del server http
Retorna             ESP_OK / ESP_FAIL
========================================================================*/
/* Este controlador permite probar la funcionalidad personalizada de manejo de errores 
*desde el lado del cliente. Para eso, cuando se envía una solicitud PUT 0 a URI / ctrl,
*las URI / principal y / echo no se registran y se registra el siguiente controlador de errores 
*personalizado http_404_error_handler (). Posteriormente, cuando se solicita / principal o / echo,
*se invoca este controlador de errores personalizado que, después de enviar un mensaje de error al 
*cliente, cierra el socket subyacente (cuando el URI solicitado es / echo) o lo mantiene abierto 
*(cuando el URI solicitado es / principal). Esto permite al cliente inferir si el controlador de 
*errores personalizado está funcionando como se esperaba al observar el estado del socket.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/ URI is not available");
        return ESP_OK; //Devuelve ESP_OK para mantener abierto el socket subyacente
    } 
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}





/*========================================================================
Funcion: mostrarPagina
Descripcion: Muestra la pagina html en la direccion correspondiente
Parametro de entrada: 
                    httpd_req_t *req= recibe el handler del server
Retorna             ESP_OK / ESP_FAIL
========================================================================*/
static esp_err_t mostrarPagina(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;
 
    //Obtiene la longitud de la cadena del header y asigna memoria con un byte adicional para la terminación null
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) { // Copia el string en el búfer y verifica que no de error
            ESP_LOGI(TAG, "Header => Host: %s", buf);
        }
        free(buf);
    }
 
    //Obtiene la longitud Query string de la URL solitada para obtener los parametros get
    buf_len = httpd_req_get_url_query_len(req) + 1; //Obtiene la longitud de la cadena del Query y asigna memoria con un byte adicional para la terminación null
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) { // Copia el string en el búfer y verifica que no de error
            ESP_LOGI(TAG, "Found URL query => %s", buf);

            #ifdef MODE_DUAL   //Si esta en modo dual, obtiene los parametros de SSID y PASS enviados desde el HTML "html_conectar_wifi.h"
            char param1[100];  //Arreglo para almacenar parametro 1 (SSID)
            char param2[100];  //Arreglo para almacenar parametro 2 (pass)
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "Red", param1, sizeof(param1)) == ESP_OK) { //Si existe el get Red, lo amacena en param1
                ESP_LOGI(TAG, "Nombre de la red =%s", param1);
            }
             if (httpd_query_key_value(buf, "password", param2, sizeof(param2)) == ESP_OK) { //Si existe el get password, lo amacena en param2
                ESP_LOGI(TAG, "Contraseña de la red =%s", param2);
            }
            write_array_eeprom("SSID", param1); //Escribo la SSID recibida desde el sitio web de configuracion en la eeprom
            write_array_eeprom("PASS", param2); //Escribo la PASS recibida desde el sitio web de configuracion en la eeprom
            
            error_estacion=0;                   //Reseteo el error_estacion=0 para que en el proximo reincio, intente conectar la estacion con los nuevos valores de la eeprom
            ESP_LOGI(TAG, "Resetenado micro en 15 Seg");
            vTaskDelay(15000 / portTICK_PERIOD_MS);
            esp_restart();                      //Reinicio Esp
            #endif
        }
        free(buf);                              //Libero memoria utilizada en buf
    }   
 
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1"); //Establecer algunos encabezados personalizados
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2"); //Establecer algunos encabezados personalizados
 
    httpd_resp_send_chunk(req,HTML,strlen(HTML)); // Envío la respuesta con encabezados personalizados y conjuntos de cuerpo html definido en "Wifi_conectar_wifi.h" como una cadena hacia el servidor
 
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) { //Después de enviar la respuesta HTTP, se pierden los encabezados de solicitud HTTP anteriores. Comprueba si los encabezados de solicitud HTTP se pueden leer ahora.
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}
#endif