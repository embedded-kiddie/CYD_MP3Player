//================================================================================
// Definition of on-board peripherals for the ESP32-2432S028R variants
//================================================================================
#pragma once

#define LED_RED   4     // CYD_ELECROW_ILI9341: IO4_PORT_AD
#define LED_GREEN 16    // CYD_ELECROW_ILI9341: IO16_RXD2 on UART1 terminal
#define LED_BLUE  17    // CYD_ELECROW_ILI9341: IO17_TXD2 on UART1 terminal

#define LED_RED_OFF()   (digitalWrite(LED_RED,   1))
#define LED_RED_ON()    (digitalWrite(LED_RED,   0))
#define LED_GREEN_OFF() (digitalWrite(LED_GREEN, 1))
#define LED_GREEN_ON()  (digitalWrite(LED_GREEN, 0))
#define LED_BLUE_OFF()  (digitalWrite(LED_BLUE,  1))
#define LED_BLUE_ON()   (digitalWrite(LED_BLUE,  0))

#define LED_RGB_OFF() \
  LED_RED_OFF();      \
  LED_GREEN_OFF();    \
  LED_BLUE_OFF()
#define LED_RGB_ON()  \
  LED_RED_ON();       \
  LED_GREEN_ON();     \
  LED_BLUE_ON()

#define RGB_LED_ON() {        \
  pinMode(LED_RED,   OUTPUT); \
  pinMode(LED_GREEN, OUTPUT); \
  pinMode(LED_BLUE,  OUTPUT); \
  LED_RGB_ON();               \
}
#define RGB_LED_OFF() {       \
  pinMode(LED_RED,   OUTPUT); \
  pinMode(LED_GREEN, OUTPUT); \
  pinMode(LED_BLUE,  OUTPUT); \
  LED_RGB_OFF();              \
}

// In some CYD variants, it's necessary to resolve the conflict between
// the audio amplifier IC (SC8002B) shutdown signal and the RGB red LED
#if defined(CYD_FREENOVE_ST7789) || defined(CYD_FREENOVE_ILI9341) || defined(CYD_ELEGOO_ILI9341)
  #define AUDIO_EN  4
  #undef  LED_RED
  #define LED_RED   22
#endif

// Enabling/Disabling the audio amplifier IC (SC8002B)
#ifdef AUDIO_EN
  #define AUDIO_ON() {            \
    pinMode(AUDIO_EN, OUTPUT);    \
    digitalWrite(AUDIO_EN, LOW);  \
  }
  #define AUDIO_OFF() {           \
    digitalWrite(AUDIO_EN, HIGH); \
  }
#else
  #define AUDIO_ON()
  #define AUDIO_OFF()
#endif

//--------------------------------------------------------------------------------
// Peripheral circuit settings
//--------------------------------------------------------------------------------
#define init_peripherals() {  \
  RGB_LED_OFF();              \
  AUDIO_ON();                 \
}

// A function to activate the sleep timer (to restart, press the reset button or cycle the power)
// https://github.com/espressif/esp-idf/blob/master/components/esp_hw_support/include/esp_sleep.h#L58-L97
// #include "esp_sleep.h"    // ESP_PD_DOMAIN_XXX for esp_sleep_pd_config()
// #include "esp_bt.h"       // esp_bt_controller_disable(), esp_bt_controller_deinit()
// #include "esp_bt_main.h"  // esp_bluedroid_disable(), esp_bluedroid_deinit()
// #include "esp_wifi.h"     // esp_wifi_stop()
#define shutdown_peripherals() {  \
  RGB_LED_OFF();                  \
  AUDIO_OFF();                    \
/*esp_bluedroid_disable();        \
  esp_bluedroid_deinit();         \
  esp_bt_controller_disable();    \
  esp_bt_controller_deinit();     \
  esp_wifi_stop();                \
  WiFi.disconnect(true);          \
  WiFi.mode(WIFI_OFF);            \
  esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO,    ESP_PD_OPTION_OFF); \
  esp_sleep_pd_config(ESP_PD_DOMAIN_MODEM,      ESP_PD_OPTION_OFF); \
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); \
*/esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL); \
}
// avoid "warning: backslash-newline at end of file"