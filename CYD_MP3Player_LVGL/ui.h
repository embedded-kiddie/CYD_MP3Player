//================================================================================
// MP3 Music Player for CYD - User Interface
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//  Released under the GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html)
//  LVGL version: 9.2.2 and up
//================================================================================
#ifndef _UI_H_
#define _UI_H_

#include <lvgl.h>
#include "config.h"
#include "debug.h"

#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH in lv_conf.h should be 16bit"
#endif

/////////////////////// CUSTOM FONTS ////////////////////////
#if USE_CUSTOM_FONTS
#define CUSTOM_FONT_SMALL   noto_sans_jp_4bit_jis1_12
#define CUSTOM_FONT_MEDIUM  noto_sans_jp_4bit_jis1_14
#else
#define CUSTOM_FONT_SMALL   lv_font_montserrat_12
#define CUSTOM_FONT_MEDIUM  lv_font_montserrat_14
#endif
#define CUSTOM_FONT_SMALL_HEIGHT  17  // For font size: 12px
#define CUSTOM_FONT_MEDIUM_HEIGHT 22  // For font size: 14px
LV_FONT_DECLARE(CUSTOM_FONT_SMALL);
LV_FONT_DECLARE(CUSTOM_FONT_MEDIUM);

/////////// ARROW BUTTONS TO NAVIGATE TO SCREENS ////////////
#define SHOW_ARROW_BUTTON false

///////// SCREEN SIZE / COORDINATE / COLOR / FONT ///////////
#define SCREEN_WIDTH      TFT_WIDTH
#define SCREEN_HEIGHT     TFT_HEIGHT
#define LV_PCT_X(x)       (SCREEN_WIDTH  * (x) / 100) // for LV_STYLE_CONST_X
#define LV_PCT_Y(y)       (SCREEN_HEIGHT * (y) / 100) // for LV_STYLE_CONST_Y
#define ICON_OFFSET_R     (SCREEN_WIDTH / 2 - 24) // need LV_ALIGN_CENTER
#define ICON_OFFSET_L     (24 - SCREEN_WIDTH / 2) // need LV_ALIGN_CENTER

#define UI_COLOR_BACKGROUND   { .blue = 0xf5, .green = 0xf5, .red = 0xf5 }  // lv_color_hex(0xf5f5f5)
#define UI_COLOR_SLIDER       { .blue = 0x00, .green = 0x00, .red = 0x00 }  // lv_color_hex(0x000000)
#define UI_COLOR_CHECKBOX     { .blue = 0xff, .green = 0x40, .red = 0x40 }  // lv_color_hex(0x4040ff)
#define UI_COLOR_LIST_DEFAULT { .blue = 0x7f, .green = 0x5a, .red = 0x5a }  // lv_color_hex(0x5a5a7f)
#define UI_COLOR_LIST_PRESSED { .blue = 0x65, .green = 0x49, .red = 0x4c }  // lv_color_hex(0x4c4965)
#define UI_COLOR_LIST_SHADOW  { .blue = 0x4b, .green = 0x2d, .red = 0x2d }  // lv_color_hex(0x2d2d4b)
#define UI_COLOR_LIST_ARTIST  { .blue = 0xbe, .green = 0xb0, .red = 0xb1 }  // lv_color_hex(0xb1b0be)
#define UI_COLOR_LIST_SLIDER  { .blue = 0xee, .green = 0xee, .red = 0xee }  // lv_color_hex(0xeeeeee)

////////////////////////// UI STATE /////////////////////////
typedef enum {
  UI_STATE_INIT,
  UI_STATE_START,
  UI_STATE_PLAY,
  UI_STATE_STOP,
  UI_STATE_PAUSE,
  UI_STATE_RESUME,
  UI_STATE_NEXT,
  UI_STATE_PREV,
  UI_STATE_AWAKE,
  UI_STATE_SLEEP,
  UI_STATE_BLOFF,
  UI_STATE_ID3,
  UI_STATE_EOF,
  UI_STATE_CLEAR,
  UI_STATE_RESET,
  UI_STATE_ALBUM,
  UI_STATE_IDLE,
  UI_STATE_ERROR,
} UI_State_t;

///////////////////////// UI SCREEN /////////////////////////
typedef enum {
  UI_SCREEN_MAIN,
  UI_SCREEN_PLAY_LIST,
  UI_SCREEN_ALBUM_LIST,
  UI_SCREEN_SETTING,
} UI_Screen_t;

//////////////////// SETTINGS / CONTROLL ////////////////////
typedef struct {
  uint8_t   repeat;           // true: repeat
  bool      favorite;         // true: a heart mark
  bool      shuffle;          // true: shuffle, false: in sequence
  uint8_t   partition_max;    // maximum partitions   (0, ...PARTITION_MAX)
  uint8_t   partition_id;     // current partition ID (0, ...partition_max)
  uint8_t   selectBacklight;  // 0: 30sec, 1: 1min, ...
  uint8_t   selectSleepTimer; // 0: 30min, 1: 1hour, ...
  uint8_t   spare;
} UI_Setting_t;

typedef struct {
  uint16_t  top;
  uint16_t  end;
  uint16_t  playNo;
  uint16_t  focusNo;
  uint32_t  backlightTimer;
  uint32_t  sleepStart;
  uint32_t  sleepTimer;
} UI_Control_t;

extern UI_State_t   ui_state;
extern UI_Setting_t ui_setting;
extern UI_Control_t ui_control;

/////////////////// SCREEN: ui_ScreenMain ///////////////////
extern lv_obj_t *ui_ScreenMain;
extern lv_obj_t *ui_MusicTitle;
extern lv_obj_t *ui_ElapsedStart;
extern lv_obj_t *ui_ElapsedEnd;
extern lv_obj_t *ui_ButtonPlay;
extern lv_obj_t *ui_ElapsedBar;
extern lv_obj_t *ui_Volume;
extern lv_obj_t *ui_AlbumImage;
extern lv_obj_t *ui_Repeat;
extern lv_obj_t *ui_Shuffle;
extern lv_obj_t *ui_Favorite;
void ui_ScreenMain_screen_init(void);
void ui_event_ScreenMain    (lv_event_t *e);
void ui_event_ButtonPlay    (lv_event_t *e);
void ui_event_Volume        (lv_event_t *e);
void ui_event_ElapsedBar    (lv_event_t *e);
void ui_event_GoToAlbumList (lv_event_t *e);
void ui_event_GoToPlayList  (lv_event_t *e);
void ui_event_GoToSettings  (lv_event_t *e);
void ui_event_Favorite      (lv_event_t *e);
void ui_event_Repeat        (lv_event_t *e);
void ui_event_Shuffle       (lv_event_t *e);
void ui_event_ButtonNext    (lv_event_t *e);
void ui_event_ButtonPrev    (lv_event_t *e);
void ui_event_VolumeMax     (lv_event_t *e);
void ui_event_VolumeMin     (lv_event_t *e);

void ui_init(void);
UI_State_t ui_loop(void);
void ui_redisplay (void);
void ui_set_playNo(uint32_t playNo);
const uint32_t ui_get_playNo(void);
const uint32_t ui_get_counts(void);

///////////////// SCREEN: ui_ScreenPlayList /////////////////
extern lv_obj_t *ui_ScreenPlayList;
void ui_ScreenPlayList_screen_init  (void);
void ui_ScreenPlayList_screen_deinit(void);
void ui_event_ScreenPlayList(lv_event_t *e);
void ui_event_PlayList_Play (lv_event_t *e);
void ui_event_PlayList_Heart(lv_event_t *e);
void ui_list_update_icon    (uint32_t track_id, bool state);
void ui_list_update_cell    (uint32_t track_id, bool state);
void ui_list_update_play    (uint32_t track_id, bool state);
void ui_list_focus_playing  (uint32_t track_id);
void ui_list_update_duration(uint32_t track_id, uint32_t duration);
bool ui_list_get_heart_state(uint32_t track_id);

// Debug functions
extern __attribute__((weak)) size_t get_cell_count(void);
extern __attribute__((weak)) void show_ui_control(void);
extern __attribute__((weak)) void dump_play_list(void);

//////////////// SCREEN: ui_ScreenAlbumList /////////////////
extern lv_obj_t *ui_ScreenAlbumList;
void ui_event_ScreenAlbumList(lv_event_t *e);
void ui_ScreenAlbumList_screen_init  (void);
void ui_ScreenAlbumList_screen_deinit(void);
void ui_album_create(void *root, bool update);
void ui_album_load  (void *root);

// Debug functions
extern __attribute__((weak)) size_t count_exposed_nodes(void);
extern __attribute__((weak)) size_t count_album_list(void);
extern __attribute__((weak)) void show_album_list(void);
extern __attribute__((weak)) void dump_album_list(void);
extern __attribute__((weak)) void dump_preorder(void);

///////////////// SCREEN: ui_ScreenSetting //////////////////
extern lv_obj_t *ui_ScreenSetting;
void ui_event_ScreenSetting(lv_event_t *e);
void ui_ScreenSetting_screen_init  (void);
void ui_ScreenSetting_screen_deinit(void);
void ui_event_Setting_Backlight (lv_event_t *e);
void ui_event_Setting_SleepTimer(lv_event_t *e);
void ui_setting_set_backlight(void);
void ui_setting_set_sleeptime(void);

////////////////////////// IMAGES ///////////////////////////
LV_IMAGE_DECLARE(img_album);            // assets/icons/img_album.png
LV_IMAGE_DECLARE(img_heart_off);        // assets/icons/img_heart_off.png
LV_IMAGE_DECLARE(img_heart_on);         // assets/icons/img_heart_on.png
LV_IMAGE_DECLARE(img_repeat);           // assets/icons/img_repeat.png
LV_IMAGE_DECLARE(img_shuffle);          // assets/icons/img_shuffle.png
LV_IMAGE_DECLARE(img_play);             // assets/icons/img_play.png
LV_IMAGE_DECLARE(img_pause);            // assets/icons/img_pause.png
LV_IMAGE_DECLARE(img_skip_next);        // assets/icons/img_skip_next.png
LV_IMAGE_DECLARE(img_skip_prev);        // assets/icons/img_skip_prev.png
LV_IMAGE_DECLARE(img_vol_max);          // assets/icons/img_vol_max.png
LV_IMAGE_DECLARE(img_vol_min);          // assets/icons/img_vol_min.png
LV_IMAGE_DECLARE(img_menu_up);          // assets/icons/img_menu_up.png
LV_IMAGE_DECLARE(img_menu_down);        // assets/icons/img_menu_down.png
LV_IMAGE_DECLARE(img_menu_right);       // assets/icons/img_menu_right.png
LV_IMAGE_DECLARE(img_menu_left);        // assets/icons/img_menu_left.png
LV_IMAGE_DECLARE(img_bluetooth_on);     // assets/icons/img_bluetooth_on.png
LV_IMAGE_DECLARE(img_bluetooth_off);    // assets/icons/img_bluetooth_off.png
LV_IMAGE_DECLARE(img_heart_off_small);  // assets/icons/img_heart_off_small.png
LV_IMAGE_DECLARE(img_heart_on_small);   // assets/icons/img_heart_on_small.png
LV_IMAGE_DECLARE(img_list_play);        // assets/icons/img_list_play.png
LV_IMAGE_DECLARE(img_list_pause);       // assets/icons/img_list_pause.png
LV_IMAGE_DECLARE(img_checkbox);         // assets/icons/img_checkbox.png
LV_IMAGE_DECLARE(img_checked);          // assets/icons/img_checked.png
LV_IMAGE_DECLARE(img_symbol_down);      // assets/icons/img_symbol_down.png
LV_IMAGE_DECLARE(img_symbol_right);     // assets/icons/img_symbol_right.png

#endif // _UI_H_