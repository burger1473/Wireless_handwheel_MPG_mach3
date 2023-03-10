/*=============================================================================
 *                           !!!!IMPORTANTE!!!!!
 * Algunos de los pines ADC2 se utilizan como pines de sujeción (GPIO 0, 2, 15), por lo que no se pueden utilizar libremente. Tal es el caso de los siguientes kits de desarrollo oficiales:
 *
 * ESP32 DevKitC: GPIO 0 no se puede utilizar debido a circuitos de programación automática externos.
 *
 * ESP-WROVER-KIT: GPIO 0, 2, 4 y 15 no se pueden utilizar debido a conexiones externas para diferentes propósitos.
 *
 * Dado que el módulo ADC2 también es utilizado por el Wi-Fi, solo uno de ellos podría obtener la preferencia cuando se usan juntos, lo que significa que adc2_get_raw() puede bloquearse hasta que el Wi-Fi se detenga, y viceversa.
 *
 * Calibracion e informacion en https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#adc-attenuation
 *===========================================================================*/


/*==================[ Inclusiones ]============================================*/
#include "../include/ADC.h"

/*==================[Prototipos de funciones]======================*/
void config_adc(uint8_t adc, uint8_t pin, adc_bits_width_t resoluncion, adc_atten_t atenuacion);
int Leer_adc(uint8_t adc, uint8_t pin, uint8_t promedio, adc_bits_width_t resoluncion);

/*==================[Implementaciones]=================================*/

/*========================================================================
Funcion: config_adc
Descripcion:Configura el adc canal 1 o 2 segun se determine
Parametro de entrada: 
                        uint8_t adc: variable para indicar el canal a configurar
                                        1
                                        2
                        uint8_t pin: pin del canal al cual se quiere configurar
                                     para canal 1:
                                                ADC1_CHANNEL_0 --> GPIO36
                                                ADC1_CHANNEL_3 --> GPIO39
                                                ADC1_CHANNEL_4 --> GPIO32
                                                ADC1_CHANNEL_5 --> GPIO33
                                                ADC1_CHANNEL_6 --> GPIO34
                                                ADC1_CHANNEL_7 --> GPIO35
                                     para canal 2:
                                                ADC2_CHANNEL_0 --> GPIO4
                                                ADC2_CHANNEL_2 --> GPIO2
                                                ADC2_CHANNEL_3 --> GPIO15
                                                ADC2_CHANNEL_4 --> GPIO13
                                                ADC2_CHANNEL_5 --> GPIO12
                                                ADC2_CHANNEL_6 --> GPIO14
                                                ADC2_CHANNEL_7 --> GPIO27
                                                ADC2_CHANNEL_8 --> GPIO25
                                                ADC2_CHANNEL_9 --> GPIO26

                        adc_bits_width_t resoluncion: indica la resolucion de la medicion
                                                    ADC_WIDTH_BIT_DEFAULT //El ancho de bits predeterminado (máximo) del ADC de la versión actual. También puede obtener el ancho de bits máximo mediante SOC_ADC_MAX_BITWIDTH definido en soc_caps.h
                                                    ADC_WIDTH_9Bit
                                                    ADC_WIDTH_10Bit
                                                    ADC_WIDTH_11Bit
                                                    ADC_WIDTH_12Bit
                        adc_atten_t atenuacion: Parámetro de atenuación ADC. Diferentes parámetros determinan el rango del ADC
                                                Vref es el voltaje de referencia utilizado internamente por los ADC ESP32 para medir el voltaje de entrada. Los ADC ESP32 pueden medir voltajes analógicos de 0 V a Vref. Entre los diferentes chips, el Vref varía, la mediana es 1,1 V. Para convertir voltajes mayores que Vref, los voltajes de entrada se pueden atenuar antes de ingresar a los ADC. Hay 4 opciones de atenuación disponibles, cuanto mayor sea la atenuación, mayor será el voltaje de entrada medible.
                                             Debido a las características del ADC, los resultados más precisos se obtienen dentro de los siguientes rangos de voltaje aproximados:
                                             ADC_ATTEN_0db     -->  100 mV ~ 950 mV
                                             ADC_ATTEN_2_5db   -->  100 mV ~ 1250 mV
                                             ADC_ATTEN_6db     -->  150 mV ~ 1750 mV
                                             ADC_ATTEN_11db    -->  150 mV ~ 2450 mV

                                             La atenuación de 0dB (ADC_ATTEN_0db) da un voltaje de escala completa 1.1V
                                             La atenuación de 2.5dB (ADC_ATTEN_2_5db) da un voltaje de escala completa de 1.5V
                                             La atenuación de 6dB (ADC_ATTEN_6db) proporciona un voltaje de escala completa de 2,2 V
                                             La atenuación de 11dB (ADC_ATTEN_11db) proporciona un voltaje de escala completa de 3.9V (vea la nota a continuación)

                                             El voltaje de escala completa es el voltaje correspondiente a una lectura máxima (dependiendo del ancho de bit configurado de ADC1, este valor es: 4095 para 12 bits, 2047 para 11 bits, 1023 para 10 bits, 511 para 9 bits).
                                             A una atenuación de 11dB, el voltaje máximo está limitado por VDD_A, no el voltaje de escala completa.

No retorna nada
========================================================================*/
void config_adc(uint8_t adc, uint8_t pin, adc_bits_width_t resoluncion, adc_atten_t atenuacion){
    if(adc==1){
        adc_gpio_init (ADC_UNIT_1, pin);
        adc1_config_width(resoluncion); //configuro la resoluncion del adc1
        adc1_config_channel_atten (pin, atenuacion); //Canal 1 
    }

    if(adc==2){
      adc2_config_channel_atten (pin, atenuacion); //Canal 2 
    }

}


/*========================================================================
Funcion: config_ad
Descripcion:Configura el adc canal 1 o 2 segun se determine
Parametro de entrada: 
                        uint8_t adc: variable para indicar el canal a configurar
                                        1
                                        2
                        uint8_t pin: pin del canal al cual se quiere configurar
                                     para canal 1:
                                                ADC1_CHANNEL_0 --> GPIO36 vp
                                                ADC1_CHANNEL_3 --> GPIO39 vn
                                                ADC1_CHANNEL_4 --> GPIO32 xtal32
                                                ADC1_CHANNEL_5 --> GPIO33 xtal32
                                                ADC1_CHANNEL_6 --> GPIO34
                                                ADC1_CHANNEL_7 --> GPIO35
                                     para canal 2:
                                                ADC2_CHANNEL_0 --> GPIO4
                                                ADC2_CHANNEL_2 --> GPIO2
                                                ADC2_CHANNEL_3 --> GPIO15
                                                ADC2_CHANNEL_4 --> GPIO13
                                                ADC2_CHANNEL_5 --> GPIO12
                                                ADC2_CHANNEL_6 --> GPIO14
                                                ADC2_CHANNEL_7 --> GPIO27
                                                ADC2_CHANNEL_8 --> GPIO25
                                                ADC2_CHANNEL_9 --> GPIO26
                                                
                        uint8_t promedio: indica la cantidad de muestras que se quiere realizar para obtener el promedio de las mediciones (actua como filtro)
                        adc_bits_width_t resoluncion: indica la resolucion de la medicion
                                                    ADC_WIDTH_BIT_DEFAULT //El ancho de bits predeterminado (máximo) del ADC de la versión actual. También puede obtener el ancho de bits máximo mediante SOC_ADC_MAX_BITWIDTH definido en soc_caps.h
                                                    ADC_WIDTH_9Bit
                                                    ADC_WIDTH_10Bit
                                                    ADC_WIDTH_11Bit
                                                    ADC_WIDTH_12Bit


No retorna nada
========================================================================*/
int Leer_adc(uint8_t adc, uint8_t pin, uint8_t promedio, adc_bits_width_t resoluncion){
    int valor=0;
    if(adc==1){
        int lectura=0;
        for(int i=0; i<promedio; i++){
            lectura= lectura + adc1_get_raw(pin);
        }
        valor=lectura/promedio;
    }

    if(adc==2){
       int lectura=0;
        for(int i=0; i<promedio; i++){
            int temp=0;
            adc2_get_raw(pin, resoluncion, &temp);
            lectura= lectura + temp;
        }
        valor=lectura/promedio;
    }
    return valor;
}