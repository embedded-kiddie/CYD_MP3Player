//================================================================================
// MP3 Music Player for CYD - Settings Screen
// LVGL version: 9.2.2 and up
//================================================================================
#include "ui.h"
#include <Arduino.h> // for millis()

//--------------------------------------------------------------------------------
// Instance of the screen widget
//--------------------------------------------------------------------------------
lv_obj_t *ui_ScreenSetting;

//--------------------------------------------------------------------------------
// Offset from the top / Roller width
//--------------------------------------------------------------------------------
#define TITLE_POS_Y   12
#define RADIO_POS_Y   24
#define TIMER_POS_Y   78
#define TIMER_WIDTH   100
#define TIMER_VISIBLE 3   // 3 or 2
#define BLTOOTH_POS_Y (108 + TIMER_VISIBLE * (CUSTOM_FONT_MEDIUM_HEIGHT * 1.2))

//--------------------------------------------------------------------------------
// Contents of dropdown list
//--------------------------------------------------------------------------------
typedef struct {
  const char      *text;
  const uint32_t  time[];
} DropdownTime_t;

static const DropdownTime_t backlight = {
  "Disable\n30 sec\n1 min\n2 min\n5 min",
  {
    0,              // Disable
    30 * 1000,      // 30 sec
    60 * 1000,      //  1 min
    60 * 1000 * 2,  //  2 min
    60 * 1000 * 5,  //  5 min
  }
};

static const DropdownTime_t sleeptime = {
  "Disable\n30 min\n60 min\n90 min\n120 min",
  {
    0,                // Disable
    60 * 1000 *  30,  //  30 min
    60 * 1000 *  60,  //  60 min
    60 * 1000 *  90,  //  90 min
    60 * 1000 * 120,  // 120 min
  }
};

void ui_setting_set_backlight(void) {
  ui_control.backlightTimer = backlight.time[ui_setting.selectBacklight];
}

void ui_setting_set_sleeptime(void) {
  ui_control.sleepTimer = sleeptime.time[ui_setting.selectSleepTimer];
}

//--------------------------------------------------------------------------------
// Set the pointer to the widget to NULL when its object is deleted
//--------------------------------------------------------------------------------
static void delete_cb(lv_event_t *e) {
  lv_obj_t **obj = (lv_obj_t **)lv_event_get_user_data(e);
  static constexpr lv_obj_t ** const adrs[] = {
    &ui_ScreenSetting,
  };

  for (int i = 0; i < sizeof(adrs) / sizeof(adrs[0]); i++) {
    if (obj == adrs[i]) {
      *obj = NULL;
      DBG_EXEC(printf("deleted: %d\n", i));
      return;
    }
  }
  DBG_EXEC(printf("deleted: 0x%x\n", obj));
}

//--------------------------------------------------------------------------------
// https://docs.lvgl.io/master/widgets/checkbox.html#checkboxes-as-radio-buttons
//--------------------------------------------------------------------------------
static void radio_event_handler(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED);

  uint8_t  *active_id = (uint8_t *)lv_event_get_user_data(e);
  lv_obj_t *container = (lv_obj_t*)lv_event_get_current_target(e);
  lv_obj_t *act_cb = lv_event_get_target_obj(e);
  lv_obj_t *old_cb = lv_obj_get_child(container, *active_id);

  // Do nothing if the container was clicked
  if (act_cb != container) {
    lv_obj_remove_state(old_cb, LV_STATE_CHECKED);  // Uncheck the previous radio button
    lv_obj_add_state   (act_cb, LV_STATE_CHECKED);  // Check the current radio button
    *active_id = (uint8_t)lv_obj_get_index(act_cb); // Update ui_setting.partition_id
  }
}

//--------------------------------------------------------------------------------
// Initialize / Deinitialize widgets
//--------------------------------------------------------------------------------
void ui_ScreenSetting_screen_init(void) {
  static uint8_t partition_id; // Return value when LV_EVENT_SCREEN_UNLOADED is fired

  if (ui_ScreenSetting == NULL) {
    partition_id = ui_setting.partition_id;
    ui_ScreenSetting = lv_obj_create(NULL);
    lv_obj_set_style_bg_color (ui_ScreenSetting, UI_COLOR_BACKGROUND, 0);
    lv_obj_add_event_cb       (ui_ScreenSetting, ui_event_ScreenSetting, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb       (ui_ScreenSetting, ui_event_ScreenSetting, LV_EVENT_SCREEN_UNLOADED, (void*)&partition_id);
    lv_obj_add_event_cb       (ui_ScreenSetting, delete_cb, LV_EVENT_DELETE, (void*)&ui_ScreenSetting);

    ///////////////////////// Partition //////////////////////////
    lv_obj_t *obj = lv_label_create(ui_ScreenSetting);
    lv_obj_set_pos(obj, LV_PCT_X(5), TITLE_POS_Y),
    lv_label_set_text_static(obj, "Partition");

    static constexpr lv_style_const_prop_t style_prop_partition[] = {
      LV_STYLE_CONST_X(LV_PCT_X(0)),
      LV_STYLE_CONST_Y(RADIO_POS_Y + 26),
      LV_STYLE_CONST_HEIGHT(LV_SIZE_CONTENT),
      LV_STYLE_CONST_WIDTH(LV_PCT_X(100)),
      LV_STYLE_CONST_PAD_TOP(5),
      LV_STYLE_CONST_PAD_LEFT(20),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(5),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_BG_OPA(0),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_partition, (void*)style_prop_partition);

    lv_obj_t *container = lv_obj_create(ui_ScreenSetting);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_add_style    (container, &style_partition, (uint32_t)LV_PART_MAIN | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_event_cb (container, radio_event_handler, LV_EVENT_VALUE_CHANGED, (void*)&partition_id);

    static constexpr lv_style_const_prop_t style_prop_radio[] = {
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_radio_check[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(NULL),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_radio,       (void*)style_prop_radio      );
    static LV_STYLE_CONST_INIT(style_radio_check, (void*)style_prop_radio_check);

    for (int i = 0; i <= PARTITION_MAX; i++) {
      obj = lv_checkbox_create(container);
      lv_checkbox_set_text_static (obj, "");
      lv_obj_add_flag             (obj, LV_OBJ_FLAG_EVENT_BUBBLE); // Propagate the events to the container
      lv_obj_add_style            (obj, &style_radio,       (uint32_t)LV_PART_INDICATOR);
      lv_obj_add_style            (obj, &style_radio_check, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);

      if (i == ui_setting.partition_id) {
        lv_obj_add_state          (obj, LV_STATE_CHECKED);
      } else {
        lv_obj_remove_state       (obj, LV_STATE_CHECKED);
      }
      if (i > ui_setting.partition_max) {
        lv_obj_add_state          (obj, LV_STATE_DISABLED);
      } else {
        lv_obj_remove_state       (obj, LV_STATE_DISABLED);
      }
    }

    // Positions should be set individually due to the proportional fonts
    static const char *label[] = {"All", "1", "2", "3", "4", "5"};
    static const uint32_t px[] = {   21,  64,  99, 135, 169, 207};
    for (int i = 0; i <= PARTITION_MAX; i++) {
      obj = lv_label_create(ui_ScreenSetting);
      lv_obj_set_pos          (obj, px[i], LV_PCT_Y(4) + RADIO_POS_Y);
      lv_label_set_text_static(obj, label[i]);
    }

    ////////////////////// Backlight Label ///////////////////////
    obj = lv_label_create(ui_ScreenSetting);
    lv_obj_set_pos          (obj, LV_PCT_X(5), LV_PCT_Y(4) + TIMER_POS_Y);
    lv_label_set_text_static(obj, "Backlight Off");

    ///////////////////// Backlight Dropdown /////////////////////
    obj = lv_roller_create(ui_ScreenSetting);
    lv_obj_set_pos          (obj, LV_PCT_X(5), LV_PCT_Y(12) + TIMER_POS_Y);
    lv_obj_set_style_width  (obj, TIMER_WIDTH, 0);
    lv_obj_add_event_cb     (obj, ui_event_Setting_Backlight, LV_EVENT_VALUE_CHANGED, NULL);
    lv_roller_set_options   (obj, backlight.text, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected  (obj, ui_setting.selectBacklight, LV_ANIM_OFF);
    lv_roller_set_visible_row_count(obj, TIMER_VISIBLE);

    ////////////////////// Sleep Timer Label /////////////////////
    obj = lv_label_create(ui_ScreenSetting);
    lv_obj_set_pos          (obj, LV_PCT_X(57), LV_PCT_Y(4) + TIMER_POS_Y);
    lv_label_set_text_static(obj, "Sleep Timer");

    //////////////////// Sleep Timer Dropdown ////////////////////
    obj = lv_roller_create(ui_ScreenSetting);
    lv_obj_set_pos          (obj, SCREEN_WIDTH - TIMER_WIDTH - LV_PCT_X(5), LV_PCT_Y(12) + TIMER_POS_Y);
    lv_obj_set_style_width  (obj, TIMER_WIDTH, 0);
    lv_obj_add_event_cb     (obj, ui_event_Setting_SleepTimer, LV_EVENT_VALUE_CHANGED, NULL);
    lv_roller_set_options   (obj, sleeptime.text, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected  (obj, ui_setting.selectSleepTimer, LV_ANIM_OFF);
    lv_roller_set_visible_row_count(obj, TIMER_VISIBLE);

#if   false
    //////////////////// Bluetooth Devices ///////////////////////
    obj = lv_label_create(ui_ScreenSetting);
    lv_obj_set_pos          (obj, LV_PCT_X(5), LV_PCT_Y(12) + BLTOOTH_POS_Y);
    lv_label_set_text_static(obj, "Bluetooth Devices");
#endif

#if SHOW_ARROW_BUTTON || true
    ///////////////////////// Arrow Icon /////////////////////////
    {
      static constexpr lv_style_const_prop_t style_prop_common[] = {
        LV_STYLE_CONST_WIDTH(27),
        LV_STYLE_CONST_HEIGHT(27),
        LV_STYLE_CONST_X(LV_PCT_X(0)),
        LV_STYLE_CONST_Y(LV_PCT_Y(44)),
        LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
        LV_STYLE_CONST_PROPS_END
      };
      static constexpr lv_style_const_prop_t style_prop_default[] = {
        LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_down),
        LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
        LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
        LV_STYLE_CONST_BORDER_WIDTH(0),
        LV_STYLE_CONST_PAD_TOP(8),
        LV_STYLE_CONST_PAD_RIGHT(0),
        LV_STYLE_CONST_PAD_BOTTOM(0),
        LV_STYLE_CONST_PAD_LEFT(8),
        LV_STYLE_CONST_PROPS_END
      };
      static constexpr lv_style_const_prop_t style_prop_pressed[] = {
        LV_STYLE_CONST_PAD_TOP(10),
        LV_STYLE_CONST_PAD_LEFT(10),
        LV_STYLE_CONST_PROPS_END
      };
      static constexpr lv_style_const_prop_t style_prop_checked[] = {
        LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_down),
        LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
        LV_STYLE_CONST_PROPS_END
      };
      static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
      static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
      static LV_STYLE_CONST_INIT(style_pressed, (void*)style_prop_pressed);
      static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

      lv_obj_t *obj = lv_checkbox_create(ui_ScreenSetting);
      lv_checkbox_set_text_static(obj, "");
      lv_obj_add_style    (obj, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
      lv_obj_add_style    (obj, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
      lv_obj_add_style    (obj, &style_pressed, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
      lv_obj_add_style    (obj, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
      lv_obj_add_event_cb (obj, ui_event_ScreenSetting, LV_EVENT_CLICKED, NULL);
    }
#endif // SHOW_ARROW_BUTTON
  }
}

void ui_ScreenSetting_screen_deinit(void) {
  if (ui_ScreenSetting) {
    lv_obj_delete_async(ui_ScreenSetting);
  }
}