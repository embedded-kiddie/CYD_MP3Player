//================================================================================
// MP3 Music Player for CYD using LVGL
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//  Released under the GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html)
//
// # README FIRST
//  ## Software Requirement
//  1. Required platform package
//    - esp32 by Espressif Systems version 2.0.17
//      - Note that version 3.x deprecates I2S audio and produces a clicking noise.
//    - Select "ESP32 Dev Module" as a board package
//    - In the "Tools" menu, select the following:
//      - Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
//      - Upload Speed: "460800" (mac) or "921600" (win)
//
//  2. Required 3rd party libraries
//    - LVGL by kisvegabor (version: 9.2.2 and up)
//    - LovyanGFX by lovyan03 (version 1.2.7 and up)
//    - SdFat by Bill Greiman (version 2.3.0)
//    - ArduinoJson by Benoit Blanchon (version 7.4.2)
//
//  ### Important Notice
//  To handle long filenames and multibyte characters, uncomment the definition
//  of `USE_UTF8_LONG_NAMES` in "libraries/SdFat/src/SdFatConfig.h" as below:
//
//    // To try UTF-8 encoded filenames.
//    #define USE_UTF8_LONG_NAMES 1
//
//  3. Configuring LVGL
//    - To configure "lv_conf.h" for this application, see "./lv_conf/README.md".
//
//  4. Configuring MP3Player
//    - Open "./config.h" and define symbols from 1. to 7. as you wish.
//
//  ## Harcware Requiremtnt
//  This software is optimized for "ESP32-2432S028R" (aka Cheap Yellow Display).
//  Connect a speaker, or an external DAC via the connector(s) on this board.
//  For detail, see the follwing resources:
//  - https://github.com/hexeguitar/ESP32_TFT_PIO#audio-amp-gain-mod
//  - https://macsbug.wordpress.com/2022/08/20/web-radio-esp32-2432s028-i2s/
//================================================================================
#include "ui.h"

// Set to your screen resolution and rotation
#define TFT_HOR_RES   TFT_WIDTH     // Portrait orientation default width  (defined in ui.h)
#define TFT_VER_RES   TFT_HEIGHT    // Portrait orientation default height (defined in ui.h)

// LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes
#define DRAW_BUF_N_DIVS 15  // 2 (75KB), 10 (15KB), 15 (10KB)
#define DRAW_BUF_SIZE   (TFT_HOR_RES * TFT_VER_RES / DRAW_BUF_N_DIVS * (LV_COLOR_DEPTH / 8))

// If the drawing buffer does not fit in the .bss segment, set the following to true:
#define USE_HEAP_MALLOC false
#if USE_HEAP_MALLOC
static uint8_t* draw_buf;
#else
static uint8_t draw_buf[DRAW_BUF_SIZE];
#endif

//----------------------------------------------------------------------
// Graphics library
//----------------------------------------------------------------------
#include "gfx.hpp"

//----------------------------------------------------------------------
// LVGL required functions
//----------------------------------------------------------------------
#if LV_USE_LOG != 0
static void my_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

// Use Arduino millis() as tick source
static uint32_t my_tick(void) {
#if SAVE_SEQUENCIAL_BMP
  return millis() - _skip;
#else
  return millis();
#endif
}

void setup() {
#if (!USE_CALIBRATED) || (DEBUG_AUDIO) || (DEBUG & 2)
  Serial.begin(115200);
  delay(1000);
#endif

  RGB_LED_OFF();
  tft_init();
  lv_init();

  // Set a tick source so that LVGL will know how much time elapsed.
  lv_tick_set_cb(my_tick);

  // Register print function for debugging
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  lv_display_t *disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_add_event_cb(disp, resolution_changed_event_cb, LV_EVENT_RESOLUTION_CHANGED, NULL);
  lv_display_set_rotation(disp, (lv_display_rotation_t)TFT_ROTATION);
  lv_display_set_flush_cb(disp, my_disp_flush);

#if USE_HEAP_MALLOC
  draw_buf = (uint8_t*)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
#endif
  lv_display_set_buffers(disp, draw_buf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Initialize the input device driver
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); // Touchpad should have POINTER type
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init();
}

void loop() {
  lv_timer_handler(); // Let the GUI do its work

  UI_State_t state = ui_loop();
  if (state == UI_STATE_SLEEP || state == UI_STATE_BLOFF) {
    update_device_state(state);
  }
}