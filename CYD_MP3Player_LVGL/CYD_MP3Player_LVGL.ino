//================================================================================
// MP3 Music Player for CYD using LVGL
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//  Released under the MIT license (https://opensource.org/license/mit)
//
// # README FIRST
//  ## Software Requirement
//  1. Required platform package
//    - esp32 by Espressif Systems version 2.0.17
//      - Note that version 3.x deprecates I2S audio and produces a clicking noise.
//    - Select "ESP32 Dev Module" as a board package
//    - In the "Tools" menu, select the following:
//      - Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
//      - Upload Speed: "460800"
//
//  2. Required 3rd party libraries
//    - LVGL by kisvegabor (version: 9.2.2 and up)
//    - LovyanGFX by lovyan03 (version 1.2.7)
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
#include "peripherals.h"

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
// LovyanGFX configuration
//----------------------------------------------------------------------
#if USE_AUTODETECT
#define LGFX_AUTODETECT
#include <LovyanGFX.h>
#else
#include "LGFX_ESP32_2432S028R_CYD.hpp"
#endif // USE_AUTODETECT

static LGFX tft;

//----------------------------------------------------------------------
// Calibrate touch panel for LovyanGFX (optional)
//----------------------------------------------------------------------
static void calibrate_touch(uint16_t cal[8]) {
  tft.clear(TFT_BLACK);                   // Clear screen with background color
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set foreground color, background color
  tft.setFont(&fonts::Font2);             // https://lang-ship.com/blog/files/LovyanGFX/font/

  // Draw guide text on the screen.
  tft.setTextDatum(textdatum_t::middle_center);
  tft.drawString  ("Touch the arrow tips in order", tft.width() >> 1, tft.height() >> 1);
  tft.setTextDatum(textdatum_t::top_left);

  uint16_t fg = TFT_WHITE;
  uint16_t bg = TFT_BLACK;

  // Electronic Paper Display
  if (tft.isEPD()) {
    std::swap(fg, bg);
  }

  // You will need to calibrate by touching the four corners of the screen.
  tft.calibrateTouch(cal, fg, bg, std::max(tft.width(), tft.height()) >> 3);

  Serial.print("\nconst uint16_t cal[8] = { ");
  for (int i = 0; i < 8; i++) {
    Serial.printf("%d%s", cal[i], (i < 7 ? ", " : " };\n"));
  }
}

static void tft_init(void) {
  tft.init();
  tft.initDMA();
  tft.setColorDepth(16);  // Set to 16-bit RGB565

  if (tft.touch()) {
    if (USE_CALIBRATED) {
      // The following is equivalent to the `_touch_instance` setting in `LGFX_ESP32_2432S028R_CYD.hpp`.
      // The `cal[8]` can be replaced with the result displayed on the serial monitor after calibration.
      const uint16_t cal[8] = {
        240,   // x_min
        3700,  // y_min
        240,   // x_min
        200,   // y_max
        3800,  // x_max
        3700,  // y_min
        3800,  // x_max
        200    // y_max
      };
      tft.setTouchCalibrate((uint16_t*)cal);
    } else {
      uint16_t cal[8];
      calibrate_touch(cal);
      tft.setTouchCalibrate((uint16_t*)cal);
    }
  } else {
    Serial.println("Touch device not found.");
  }
}

//----------------------------------------------------------------------
// Display sleep/wakeup
//----------------------------------------------------------------------
static bool is_awake = true;

static void update_device_state(UI_State_t state) {
  switch (state) {
    case UI_STATE_SLEEP:
      shutdown_peripherals();
      esp_deep_sleep_start();
      break;
    case UI_STATE_AWAKE:
      tft.wakeup();
      ui_redisplay();
      is_awake = true;
      break;
    case UI_STATE_BLOFF:
    default: // may be UI_STATE_SLEEP
      tft.sleep();
      is_awake = false;
      break;
  }
}

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

// LVGL calls it when a rendered image needs to copied to the display
static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushPixelsDMA((lgfx::rgb565_t *)px_map, w * h);  // { startWrite(); writePixelsDMA(data, len); endWrite(); }

  // Call it to tell LVGL you are ready
  lv_display_flush_ready(disp);
}

// Read the touchpad
static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  uint16_t x, y;
  bool touched = tft.getTouch(&x, &y);

  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  else if (!is_awake) {
    update_device_state(UI_STATE_AWAKE);
    delay(200); // Prevent unintended events from firing
    data->state = LV_INDEV_STATE_RELEASED;
  }

  else {
    data->state = LV_INDEV_STATE_PRESSED;

    switch (tft.getRotation()) {
      case LV_DISPLAY_ROTATION_0:
        data->point.x = x;
        data->point.y = y;
        break;
      case LV_DISPLAY_ROTATION_90:
        data->point.x = y;
        data->point.y = TFT_VER_RES - x;
        break;
      case LV_DISPLAY_ROTATION_180:
        data->point.x = TFT_HOR_RES - x;
        data->point.y = TFT_VER_RES - y;
        break;
      case LV_DISPLAY_ROTATION_270:
        data->point.x = TFT_HOR_RES - y;
        data->point.y = x;
        break;
    }

//  Serial.printf("x: %d (%d), y: %d (%d)\n", data->point.x, x, data->point.y, y);
  }
}

static void resolution_changed_event_cb(lv_event_t *e) {
  lv_display_t *disp = (lv_display_t *)lv_event_get_target(e);
  lv_display_rotation_t rot = lv_display_get_rotation(disp);

  // Handle rotation
  switch (rot) {
    case LV_DISPLAY_ROTATION_0:
      tft.setRotation(0); // Portrait orientation
      break;
    case LV_DISPLAY_ROTATION_90:
      tft.setRotation(1); // Landscape orientation
      break;
    case LV_DISPLAY_ROTATION_180:
      tft.setRotation(2); // Portrait orientation, flipped
      break;
    case LV_DISPLAY_ROTATION_270:
      tft.setRotation(3); // Landscape orientation, flipped
      break;
  }
}

// Use Arduino millis() as tick source
static uint32_t my_tick(void) {
#if SAVE_SEQUENCIAL_BMP
  return millis() - _skip;
#else
  return millis();
#endif
}

void setup() {
#if (!USE_CALIBRATED) || (DEBUG & 2)
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
  if (state == UI_STATE_SLEEP || (state == UI_STATE_BLOFF && is_awake == true)) {
    update_device_state(state);
  }
}