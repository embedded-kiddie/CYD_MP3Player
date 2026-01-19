//================================================================================
// MP3 Music Player for CYD - Main Screen
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//  Released under the GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html)
//  LVGL version: 9.2.2 and up
//================================================================================
#include "ui.h"

//--------------------------------------------------------------------------------
// Widgets' Instances in the screen
//--------------------------------------------------------------------------------
lv_obj_t *ui_ScreenMain;
lv_obj_t *ui_MusicTitle;
lv_obj_t *ui_ElapsedStart;
lv_obj_t *ui_ElapsedEnd;
lv_obj_t *ui_ButtonPlay;
lv_obj_t *ui_ElapsedBar;
lv_obj_t *ui_Volume;
lv_obj_t *ui_AlbumImage;
lv_obj_t *ui_Repeat;
lv_obj_t *ui_Shuffle;
lv_obj_t *ui_Favorite;

//--------------------------------------------------------------------------------
// Initialize screen
//--------------------------------------------------------------------------------
void ui_ScreenMain_screen_init(void) {
  ui_ScreenMain = lv_obj_create(NULL);
  lv_obj_set_scrollbar_mode(ui_ScreenMain, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_bg_color(ui_ScreenMain, UI_COLOR_BACKGROUND, 0);

  /////////////////// Album Cover Picture ///////////////////
  ui_AlbumImage = lv_image_create(ui_ScreenMain);
  lv_image_set_src(ui_AlbumImage, &img_album);
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_X(0),
#if SHOW_ARROW_BUTTON
      LV_STYLE_CONST_Y(LV_PCT_Y(-24)),
#else
      LV_STYLE_CONST_Y(LV_PCT_Y(-26)),
#endif
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common, (void*)style_prop_common);
    lv_obj_add_style(ui_AlbumImage, &style_common, 0);
  }

#if SHOW_ARROW_BUTTON
  /////////////////////// Arrow Icon ////////////////////////
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(27),
      LV_STYLE_CONST_HEIGHT(27),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default_R[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_right),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(8),
      LV_STYLE_CONST_PAD_TOP(8),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default_L[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_left),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(8),
      LV_STYLE_CONST_PAD_TOP(8),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default_U[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_up),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_TOP(8),
      LV_STYLE_CONST_PAD_LEFT(16),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked_R[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_right),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked_L[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_left),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked_U[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_up),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_pressed[] = {
      LV_STYLE_CONST_PAD_LEFT(10),
      LV_STYLE_CONST_PAD_TOP(10),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,    (void*)style_prop_common   );
    static LV_STYLE_CONST_INIT(style_default_R, (void*)style_prop_default_R);
    static LV_STYLE_CONST_INIT(style_default_L, (void*)style_prop_default_L);
    static LV_STYLE_CONST_INIT(style_default_U, (void*)style_prop_default_U);
    static LV_STYLE_CONST_INIT(style_checked_R, (void*)style_prop_checked_R);
    static LV_STYLE_CONST_INIT(style_checked_L, (void*)style_prop_checked_L);
    static LV_STYLE_CONST_INIT(style_checked_U, (void*)style_prop_checked_U);
    static LV_STYLE_CONST_INIT(style_pressed,   (void*)style_prop_pressed  );

    //////////////////// Move to Playlist ////////////////////
    lv_obj_t *obj = lv_checkbox_create(ui_ScreenMain);
    lv_checkbox_set_text_static(obj, "");
    lv_obj_set_style_x  (obj, ICON_OFFSET_R+1,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_set_style_y  (obj, LV_PCT_Y(-27),    (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_common,    (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_default_R, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_checked_R, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
    lv_obj_add_style    (obj, &style_pressed,   (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_event_cb (obj, ui_event_GoToPlayList, LV_EVENT_CLICKED, NULL);

    //////////////////// Move to Album List ////////////////////
    obj = lv_checkbox_create(ui_ScreenMain);
    lv_checkbox_set_text_static(obj, "");
    lv_obj_set_style_x  (obj, ICON_OFFSET_L-1,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_set_style_y  (obj, LV_PCT_Y(-27),    (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_common,    (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_default_L, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_checked_L, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
    lv_obj_add_style    (obj, &style_pressed,   (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_event_cb (obj, ui_event_GoToAlbumList, LV_EVENT_CLICKED, NULL);

    //////////////////// Move to Settings /////////////////////
    obj = lv_checkbox_create(ui_ScreenMain);
    lv_checkbox_set_text_static(obj, "");
    lv_obj_set_style_x  (obj, 0,                (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_set_style_y  (obj, LV_PCT_Y(-44),    (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_common,    (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_default_U, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style    (obj, &style_checked_U, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
    lv_obj_add_style    (obj, &style_pressed,   (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_event_cb (obj, ui_event_GoToSettings, LV_EVENT_CLICKED, NULL);
  }
#endif // SHOW_ARROW_BUTTON

  /////////////////////// Music Title ///////////////////////
  ui_MusicTitle = lv_label_create(ui_ScreenMain);
  lv_label_set_long_mode  (ui_MusicTitle, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text_static(ui_MusicTitle, "Music Title / Artist / Album Title");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_HEIGHT(LV_SIZE_CONTENT),
      LV_STYLE_CONST_WIDTH(LV_PCT_X(90)),
      LV_STYLE_CONST_X(0),
      LV_STYLE_CONST_Y(LV_PCT_Y(2)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_TEXT_FONT(&CUSTOM_FONT_MEDIUM),
      LV_STYLE_CONST_TEXT_ALIGN(LV_TEXT_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common, (void*)style_prop_common);
    lv_obj_add_style(ui_MusicTitle, &style_common, 0);
  }

  /////////////////////// Start Time ////////////////////////
  ui_ElapsedStart = lv_label_create(ui_ScreenMain);
  lv_label_set_text_static(ui_ElapsedStart, "0:00");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_HEIGHT(LV_SIZE_CONTENT),
      LV_STYLE_CONST_WIDTH(LV_SIZE_CONTENT),
      LV_STYLE_CONST_X(ICON_OFFSET_L),
      LV_STYLE_CONST_Y(LV_PCT_Y(13)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common, (void*)style_prop_common);
    lv_obj_add_style(ui_ElapsedStart, &style_common, 0);
  }

  /////////////////////// Finish Time ///////////////////////
  ui_ElapsedEnd = lv_label_create(ui_ScreenMain);
  lv_label_set_text_static(ui_ElapsedEnd, "0:00");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_HEIGHT(LV_SIZE_CONTENT),
      LV_STYLE_CONST_WIDTH(LV_SIZE_CONTENT),
      LV_STYLE_CONST_X(ICON_OFFSET_R),
      LV_STYLE_CONST_Y(LV_PCT_Y(13)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common, (void*)style_prop_common);
    lv_obj_add_style(ui_ElapsedEnd, &style_common, 0);
  }

  ////////////////////// Elapsed Time ///////////////////////
  ui_ElapsedBar = lv_slider_create(ui_ScreenMain);
  lv_slider_set_value       (ui_ElapsedBar, 0, LV_ANIM_OFF);
  lv_obj_remove_flag        (ui_ElapsedBar, LV_OBJ_FLAG_GESTURE_BUBBLE);
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_HEIGHT(6),
      LV_STYLE_CONST_WIDTH(LV_PCT_X(60)),
      LV_STYLE_CONST_X(0),
      LV_STYLE_CONST_Y(LV_PCT_Y(13)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_indicator[] = {
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_SLIDER),
      LV_STYLE_CONST_BG_OPA(255),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_knob[] = {
      LV_STYLE_CONST_BG_OPA(0),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,    (void*)style_prop_common   );
    static LV_STYLE_CONST_INIT(style_indicator, (void*)style_prop_indicator);
    static LV_STYLE_CONST_INIT(style_knob,      (void*)style_prop_knob     );

    lv_obj_add_style(ui_ElapsedBar, &style_common,    (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_ElapsedBar, &style_indicator, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_ElapsedBar, &style_knob,      (uint32_t)LV_PART_KNOB      | (uint32_t)LV_STATE_DEFAULT);
  }

  ////////////////////// Bluetooth Icon /////////////////////
  {
    lv_obj_t *obj = lv_image_create(ui_ScreenMain);
    lv_image_set_src(obj, &img_bluetooth_off);

    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_X(ICON_OFFSET_L),
      LV_STYLE_CONST_Y(LV_PCT_Y(-44)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_IMAGE_OPA(64),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common, (void*)style_prop_common);
    lv_obj_add_style(obj, &style_common, 0);
  }

  /////////////////////// Repeat Icon ///////////////////////
  ui_Repeat = lv_checkbox_create(ui_ScreenMain);
  lv_checkbox_set_text_static (ui_Repeat, "");
  lv_obj_add_state            (ui_Repeat, ui_setting.repeat ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(25),
      LV_STYLE_CONST_HEIGHT(25),
      LV_STYLE_CONST_X(ICON_OFFSET_R),
      LV_STYLE_CONST_Y(LV_PCT_Y(-10)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_repeat),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BG_IMAGE_OPA(64),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(8),
      LV_STYLE_CONST_PAD_TOP(8),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_pressed[] = {
      LV_STYLE_CONST_PAD_LEFT(10),
      LV_STYLE_CONST_PAD_TOP(10),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_repeat),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_BG_IMAGE_OPA(255),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
    static LV_STYLE_CONST_INIT(style_pressed, (void*)style_prop_pressed);
    static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

    lv_obj_add_style(ui_Repeat, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_Repeat, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_Repeat, &style_pressed, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_style(ui_Repeat, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  }

  ////////////////////// Shuffle Icon ///////////////////////
  ui_Shuffle = lv_checkbox_create(ui_ScreenMain);
  lv_checkbox_set_text_static (ui_Shuffle, "");
  lv_obj_add_state            (ui_Shuffle, ui_setting.shuffle ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(25),
      LV_STYLE_CONST_HEIGHT(25),
      LV_STYLE_CONST_X(ICON_OFFSET_L),
      LV_STYLE_CONST_Y(LV_PCT_Y(-10)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_shuffle),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BG_IMAGE_OPA(64),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(8),
      LV_STYLE_CONST_PAD_TOP(8),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_pressed[] = {
      LV_STYLE_CONST_PAD_LEFT(10),
      LV_STYLE_CONST_PAD_TOP(10),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_shuffle),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_BG_IMAGE_OPA(255),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
    static LV_STYLE_CONST_INIT(style_pressed, (void*)style_prop_pressed);
    static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

    lv_obj_add_style(ui_Shuffle, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_Shuffle, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_Shuffle, &style_pressed, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_style(ui_Shuffle, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  }

  ////////////////// Favorite (Heart) Icon //////////////////
  ui_Favorite = lv_checkbox_create(ui_ScreenMain);
  lv_checkbox_set_text_static(ui_Favorite, "");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(25),
      LV_STYLE_CONST_HEIGHT(25),
      LV_STYLE_CONST_X(ICON_OFFSET_R),
      LV_STYLE_CONST_Y(LV_PCT_Y(-44)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_heart_off),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(8),
      LV_STYLE_CONST_PAD_TOP(8),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_pressed[] = {
      LV_STYLE_CONST_PAD_LEFT(10),
      LV_STYLE_CONST_PAD_TOP(10),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_heart_on),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
    static LV_STYLE_CONST_INIT(style_pressed, (void*)style_prop_pressed);
    static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

    lv_obj_add_style(ui_Favorite, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_Favorite, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_Favorite, &style_pressed, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_style(ui_Favorite, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  }

  /////////////////////// Play Button ///////////////////////
  ui_ButtonPlay = lv_checkbox_create(ui_ScreenMain);
  lv_checkbox_set_text_static(ui_ButtonPlay, "");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(50),
      LV_STYLE_CONST_HEIGHT(50),
      LV_STYLE_CONST_X(LV_PCT_X(0)),
      LV_STYLE_CONST_Y(LV_PCT_Y(27)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_play),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(34),
      LV_STYLE_CONST_PAD_TOP(34),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_pause),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
    static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

    lv_obj_add_style(ui_ButtonPlay, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_ButtonPlay, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_ButtonPlay, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  }

  /////////////////////// Next Button ///////////////////////
  lv_obj_t *ui_ButtonNext = lv_checkbox_create(ui_ScreenMain);
  lv_checkbox_set_text_static(ui_ButtonNext, "");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(45),
      LV_STYLE_CONST_HEIGHT(45),
      LV_STYLE_CONST_X(LV_PCT_X(27)),
      LV_STYLE_CONST_Y(LV_PCT_Y(28)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_skip_next),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(24),
      LV_STYLE_CONST_PAD_TOP(24),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_pressed[] = {
      LV_STYLE_CONST_PAD_LEFT(26),
      LV_STYLE_CONST_PAD_TOP(26),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_skip_next),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
    static LV_STYLE_CONST_INIT(style_pressed, (void*)style_prop_pressed);
    static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

    lv_obj_add_style(ui_ButtonNext, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_ButtonNext, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_ButtonNext, &style_pressed, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_style(ui_ButtonNext, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  }

  ///////////////////// Previous Button /////////////////////
  lv_obj_t *ui_ButtonPrev = lv_checkbox_create(ui_ScreenMain);
  lv_checkbox_set_text_static(ui_ButtonPrev, "");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(45),
      LV_STYLE_CONST_HEIGHT(45),
      LV_STYLE_CONST_X(LV_PCT_X(-26)),
      LV_STYLE_CONST_Y(LV_PCT_Y(28)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_skip_prev),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(24),
      LV_STYLE_CONST_PAD_TOP(24),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_pressed[] = {
      LV_STYLE_CONST_PAD_LEFT(26),
      LV_STYLE_CONST_PAD_TOP(26),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_skip_prev),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
    static LV_STYLE_CONST_INIT(style_pressed, (void*)style_prop_pressed);
    static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

    lv_obj_add_style(ui_ButtonPrev, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_ButtonPrev, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_ButtonPrev, &style_pressed, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_style(ui_ButtonPrev, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  }

  ///////////////////// Volume (+) Icon /////////////////////
  lv_obj_t *ui_VolumeMax = lv_checkbox_create(ui_ScreenMain);
  lv_checkbox_set_text_static(ui_VolumeMax, "");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(40),
      LV_STYLE_CONST_HEIGHT(40),
      LV_STYLE_CONST_X(ICON_OFFSET_R),
      LV_STYLE_CONST_Y(LV_PCT_Y(43)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_vol_max),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(22),
      LV_STYLE_CONST_PAD_TOP(22),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_pressed[] = {
      LV_STYLE_CONST_PAD_LEFT(24),
      LV_STYLE_CONST_PAD_TOP(24),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_vol_max),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
    static LV_STYLE_CONST_INIT(style_pressed, (void*)style_prop_pressed);
    static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

    lv_obj_add_style(ui_VolumeMax, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_VolumeMax, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_VolumeMax, &style_pressed, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_style(ui_VolumeMax, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  }

  ///////////////////// Volume (-) Icon /////////////////////
  lv_obj_t *ui_VolumeMin = lv_checkbox_create(ui_ScreenMain);
  lv_checkbox_set_text_static(ui_VolumeMin, "");
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_WIDTH(40),
      LV_STYLE_CONST_HEIGHT(40),
      LV_STYLE_CONST_X(ICON_OFFSET_L+2),
      LV_STYLE_CONST_Y(LV_PCT_Y(43)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_vol_min),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
      LV_STYLE_CONST_BORDER_WIDTH(0),
      LV_STYLE_CONST_PAD_LEFT(22),
      LV_STYLE_CONST_PAD_TOP(22),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_pressed[] = {
      LV_STYLE_CONST_PAD_LEFT(24),
      LV_STYLE_CONST_PAD_TOP(24),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_checked[] = {
      LV_STYLE_CONST_BG_IMAGE_SRC(&img_vol_min),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);
    static LV_STYLE_CONST_INIT(style_pressed, (void*)style_prop_pressed);
    static LV_STYLE_CONST_INIT(style_checked, (void*)style_prop_checked);

    lv_obj_add_style(ui_VolumeMin, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_VolumeMin, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_VolumeMin, &style_pressed, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
    lv_obj_add_style(ui_VolumeMin, &style_checked, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  }

  ////////////////////// Volume Slider //////////////////////
  ui_Volume = lv_slider_create(ui_ScreenMain);
  lv_slider_set_range(ui_Volume, 0, 21); // MP3_VOLUME_MAX (21) is defined in MP3Player.h
  lv_slider_set_value(ui_Volume, 0, LV_ANIM_OFF);
  lv_obj_remove_flag (ui_Volume, LV_OBJ_FLAG_GESTURE_BUBBLE);
  {
    static constexpr lv_style_const_prop_t style_prop_common[] = {
      LV_STYLE_CONST_HEIGHT(8),
      LV_STYLE_CONST_WIDTH(LV_PCT_X(49)),
      LV_STYLE_CONST_X(0),
      LV_STYLE_CONST_Y(LV_PCT_Y(43)),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_CENTER),
      LV_STYLE_CONST_PROPS_END
    };
    static constexpr lv_style_const_prop_t style_prop_default[] = {
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_SLIDER),
      LV_STYLE_CONST_BG_OPA(255),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_common,  (void*)style_prop_common );
    static LV_STYLE_CONST_INIT(style_default, (void*)style_prop_default);

    lv_obj_add_style(ui_Volume, &style_common,  (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_Volume, &style_default, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
    lv_obj_add_style(ui_Volume, &style_default, (uint32_t)LV_PART_KNOB      | (uint32_t)LV_STATE_DEFAULT);
  }

  lv_obj_add_event_cb(ui_Favorite,    ui_event_Favorite,    LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_Repeat,      ui_event_Repeat,      LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_Shuffle,     ui_event_Shuffle,     LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_ButtonPlay,  ui_event_ButtonPlay,  LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_ButtonNext,  ui_event_ButtonNext,  LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_ButtonPrev,  ui_event_ButtonPrev,  LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_VolumeMax,   ui_event_VolumeMax,   LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_VolumeMin,   ui_event_VolumeMin,   LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_Volume,      ui_event_Volume,      LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(ui_ElapsedBar,  ui_event_ElapsedBar,  LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(ui_ScreenMain,  ui_event_ScreenMain,  LV_EVENT_GESTURE, NULL);
}