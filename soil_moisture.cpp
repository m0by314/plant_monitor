#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soil_moisture.h"


/* --------- COEF -----------------
* Calculate with dry point (500, 100) and wet point (2100, 0)  
* 500 is the voltage in mV obtained by placing the sensor in water and 2100 is the voltage of the sensor in air
*/
#define A   -0.063  // fonction ax+b 
#define B   131.25


#define NB_MESURE 5 

static esp_adc_cal_characteristics_t adc1_chars;

/*  ----------- ADC 1 PIN ---------------------
    ADC1_CHANNEL_0 = 0,  ADC1 channel 0 is GPIO36 
    ADC1_CHANNEL_1,      ADC1 channel 1 is GPIO37 
    ADC1_CHANNEL_2,      ADC1 channel 2 is GPIO38 
    ADC1_CHANNEL_3,      ADC1 channel 3 is GPIO39 
    ADC1_CHANNEL_4,      ADC1 channel 4 is GPIO32 
    ADC1_CHANNEL_5,      ADC1 channel 5 is GPIO33 
    ADC1_CHANNEL_6,      ADC1 channel 6 is GPIO34 
    ADC1_CHANNEL_7,      ADC1 channel 7 is GPIO35 
*/


void soil_moisture_configure() {
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_12Bit, 0, &adc1_chars);
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
};

float get_soil_moisture_percent() {
  int adc_value = 0;

  for (int i=0; i < NB_MESURE; i++){
    adc_value += adc1_get_raw(ADC1_CHANNEL_0);
    vTaskDelay(500/ portTICK_PERIOD_MS);
  }

  int avg = (adc_value / NB_MESURE);
  uint32_t voltage = esp_adc_cal_raw_to_voltage(avg, &adc1_chars);
  float percent = A * voltage + B;
  
  return percent;
}