#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <esp_task_wdt.h>

#include <DHT.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"
#include "soil_moisture.h"

#define WDT_TIMEOUT 10    // define a 10 seconds WDT (Watch Dog Timer)

DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure client;
Adafruit_MQTT_Client mqtt(&client, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWD);

Adafruit_MQTT_Publish topic_soil_moisture = Adafruit_MQTT_Publish(&mqtt,  TOPIC_SOIL_MOISTURE);
Adafruit_MQTT_Publish topic_temperature = Adafruit_MQTT_Publish(&mqtt,  TOPIC_TEMPERATURE);
Adafruit_MQTT_Publish topic_humidity = Adafruit_MQTT_Publish(&mqtt,  TOPIC_HUMIDITY);

void setup(){
  Serial.begin(115200);
  setCpuFrequencyMhz(80);

  /* --------- Sleep Configuration  --------- */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);  // Timer for the wakeup of the ESP
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  /* ------------------------------- */

  /* -------- WatchDog configuration --------*/
  esp_task_wdt_init(WDT_TIMEOUT, true);      // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                    // add current thread to WDT watch
  /* ------------------------------- */

  /* ------ WIFI connection -------- */
  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  /* ------------------------------- */
  
  /* ------ MQTT connection -------- */
  client.setCACert(_adafruit_root_certificate);
  printf("Connecting to MQTT... ");
  while (mqtt.connect() != 0) { // connect will return 0 for connected
      printf(".");
      delay(1000);
  }
  printf("MQTT Connected! \n");
  /* ------------------------------- */

  /* ------ ADC configure ---------- */
  soil_moisture_configure();
  /* ------------------------------- */

  dht.begin();
  esp_task_wdt_reset();            // Added to repeatedly reset the Watch Dog Timer
}

void loop(){
  float percent = get_soil_moisture_percent();
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  topic_soil_moisture.publish(percent);
  topic_humidity.publish(humidity);
  topic_temperature.publish(temperature);

  printf("Soil Moisture : %f%%\n", percent);
  printf("Temperature: %f°C, Humidity: %f%%\n", temperature, humidity);
  
  esp_task_wdt_reset();
  delay(500);
  esp_deep_sleep_start();
}
