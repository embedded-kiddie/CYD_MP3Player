//================================================================================
// ESP32 onboard built-in peripherals
// Target board: ESP32 Dev Module / ESP32-2432S028R CYD
// https://github.com/espressif/arduino-esp32/tree/master/variants/esp32
// https://github.com/espressif/arduino-esp32/tree/master/variants/jczn_2432s028r
//================================================================================
#ifndef _PERIPHERALS_H
#define _PERIPHERALS_H

// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/GPIO/BlinkRGB/BlinkRGB.ino
#ifdef RGB_BUILTIN
#define RGB_LED_OFF() {           \
  pinMode(RGB_BUILTIN, OUTPUT);   \
  digitalWrite(RGB_BUILTIN, LOW); \
}

// "ESP32-2432S028R CYD" on 3.x
#elif defined(CYD_LED_RGB_OFF)
#define RGB_LED_ON()  CYD_LED_RGB_ON()
#define RGB_LED_OFF() CYD_LED_RGB_OFF()

// "ESP32 Dev Module" on 2.0.17
#else
#define CYD_LED_RED   4
#define CYD_LED_GREEN 16
#define CYD_LED_BLUE  17

#define CYD_LED_RED_OFF()   (digitalWrite(CYD_LED_RED,   1))
#define CYD_LED_RED_ON()    (digitalWrite(CYD_LED_RED,   0))
#define CYD_LED_GREEN_OFF() (digitalWrite(CYD_LED_GREEN, 1))
#define CYD_LED_GREEN_ON()  (digitalWrite(CYD_LED_GREEN, 0))
#define CYD_LED_BLUE_OFF()  (digitalWrite(CYD_LED_BLUE,  1))
#define CYD_LED_BLUE_ON()   (digitalWrite(CYD_LED_BLUE,  0))
#define CYD_LED_RGB_OFF() \
  CYD_LED_RED_OFF();      \
  CYD_LED_GREEN_OFF();    \
  CYD_LED_BLUE_OFF()
#define CYD_LED_RGB_ON()  \
  CYD_LED_RED_ON();       \
  CYD_LED_GREEN_ON();     \
  CYD_LED_BLUE_ON()
#define CYD_LED_WHITE_OFF() CYD_LED_RGB_OFF()
#define CYD_LED_WHITE_ON()  CYD_LED_RGB_ON()

#define RGB_LED_ON() {            \
  pinMode(CYD_LED_RED,   OUTPUT); \
  pinMode(CYD_LED_GREEN, OUTPUT); \
  pinMode(CYD_LED_BLUE,  OUTPUT); \
  CYD_LED_RGB_ON();               \
}
#define RGB_LED_OFF() {           \
  pinMode(CYD_LED_RED,   OUTPUT); \
  pinMode(CYD_LED_GREEN, OUTPUT); \
  pinMode(CYD_LED_BLUE,  OUTPUT); \
  CYD_LED_RGB_OFF();              \
}

#endif // RGB_BUILTIN or CYD_LED_RGB_OFF

// https://github.com/espressif/esp-idf/blob/master/components/esp_hw_support/include/esp_sleep.h#L52-L102
// #include "esp_sleep.h"    // ESP_PD_DOMAIN_XXX for esp_sleep_pd_config()
// #include "esp_bt.h"       // esp_bt_controller_disable(), esp_bt_controller_deinit()
// #include "esp_bt_main.h"  // esp_bluedroid_disable(), esp_bluedroid_deinit()
// #include "esp_wifi.h"     // esp_wifi_stop()
#define shutdown_peripherals() {  \
  RGB_LED_OFF();                  \
/*esp_bluedroid_disable();        \
  esp_bluedroid_deinit();         \
  esp_bt_controller_disable();    \
  esp_bt_controller_deinit();     \
  esp_wifi_stop();                \
  WiFi.disconnect(true);          \
  WiFi.mode(WIFI_OFF);            \
  esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO,    ESP_PD_OPTION_OFF); \
  esp_sleep_pd_config(ESP_PD_DOMAIN_MODEM,      ESP_PD_OPTION_OFF); \
*/esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); \
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);         \
}

#endif // _PERIPHERALS_H