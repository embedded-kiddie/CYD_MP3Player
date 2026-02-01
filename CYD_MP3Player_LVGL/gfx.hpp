//================================================================================
// Display panel and grapics library configuration
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//================================================================================
#include "config.h"
#include "peripherals.h"

#if (GFX_LIBRARY_TYPE == USE_LOVYANGFX)
//----------------------------------------------------------------------
// LovyanGFX configuration
//----------------------------------------------------------------------
#if (GFX_DISPLAY_TYPE <= CYD_2432S028R_2USB)
  #if USE_AUTODETECT
    #define LGFX_AUTODETECT
    #include <LovyanGFX.h>
  #else
    #include "LGFX_CYD_2432S028R.hpp"
  #endif
#else
  #include "LGFX_ELECROW_2432R.hpp"
#endif

static LGFX tft;

// Backlight control
#define GFX_BLON()  tft.wakeup()
#define GFX_BLOFF() tft.sleep()

// Calibrate touch panel for LovyanGFX (optional)
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

// LVGL calls it when a rendered image needs to copied to the display
static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushPixelsDMA((lgfx::rgb565_t *)px_map, w * h);  // { startWrite(); writePixelsDMA(data, len); endWrite(); }

  // Call it to tell LVGL you are ready
  lv_display_flush_ready(disp);
}

static void tft_init(void) {
  tft.init();
  tft.initDMA();
  tft.setColorDepth(16);  // Set to 16-bit RGB565

  if (tft.touch()) {
    if (USE_CALIBRATED) {
      // The following is equivalent to the `_touch_instance` setting in `LGFX_CYD_2432S028R.hpp`.
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

#elif (GFX_LIBRARY_TYPE == USE_TFT_ESPI)
//----------------------------------------------------------------------
// TFT_eSPI configuration
//----------------------------------------------------------------------
#include <TFT_eSPI.h>

static TFT_eSPI tft = TFT_eSPI();

// Backlight control
#define GFX_BLON()  { pinMode(TFT_BL, OUTPUT); digitalWrite(TFT_BL,  TFT_BACKLIGHT_ON); }
#define GFX_BLOFF() { pinMode(TFT_BL, OUTPUT); digitalWrite(TFT_BL, !TFT_BACKLIGHT_ON); }

// https://github.com/Bodmer/TFT_eSPI/tree/master/examples/Generic/Touch_calibrate
static void calibrate_touch(uint16_t cal[5]) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 20);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Touch corners in order");
  tft.calibrateTouch(cal, TFT_MAGENTA, TFT_BLACK, 15);

  Serial.print("\n// Touch calibration parameters for TFT_eSPI\n");
  Serial.print("uint16_t cal[5] = { ");
  for (int i = 0; i < 5; ++i) {
    Serial.print(cal[i]);
    Serial.print(i < 4 ? ", " : " }\n");
  }
}

// Display Refresh
static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)px_map, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

static void tft_init(void) {
  // Start the tft display
  tft.init();
//tft.initDMA(true);  // garbage lines are displayed
  tft.setRotation(TFT_ROTATION);

  // Touch calibration parameters for TFT_eSPI
  if (USE_CALIBRATED) {
    uint16_t cal[5] = { 285, 3500, 200, 3500, 2 };
    tft.setTouch(cal);
  } else {
    uint16_t cal[5];
    calibrate_touch(cal);
    tft.setTouch(cal);
  }
}

#endif // LovyanGFX or TFT_eSPI

//----------------------------------------------------------------------
// Common functions independent of LCD type
//----------------------------------------------------------------------
static bool is_awake = true;

// Sleep/Awake peripherals control
static void update_device_state(UI_State_t state) {
  switch (state) {
    case UI_STATE_SLEEP:
      shutdown_peripherals();
      esp_deep_sleep_start();
      break;
    case UI_STATE_AWAKE:
      if (is_awake == false) {
        GFX_BLON();
        ui_redisplay();
        is_awake = true;
      }
      break;
    case UI_STATE_BLOFF:
    default: // may be UI_STATE_SLEEP
      if (is_awake == true) {
        GFX_BLOFF();
        is_awake = false;
      }
      break;
  }
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