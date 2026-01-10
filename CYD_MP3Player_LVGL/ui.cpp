//================================================================================
// MP3 Music Player for CYD - User Interface
// LVGL version: 9.2.2 and up
//================================================================================
#include "ui.h"
#include "json.hpp"
#include <string.h> // for strncpy(), strrchr()

////////////////////// GLOBAL VARIABLES /////////////////////
UI_State_t   ui_state;
UI_Control_t ui_control;
UI_Setting_t ui_setting = {
  .selectBacklight = 1, // 30 sec
};

////////////////////// LOCAL VARIABLES //////////////////////
#include "MP3Player.h"
static MP3Player  player;
static MP3Tags_t  id3tags;
static UI_State_t nextState;

//--------------------------------------------------------------------------------
// Auto saving flag
//--------------------------------------------------------------------------------
#define SAVE_SETTING   1
#define SAVE_DURATION  2
#define SAVE_FAVORITE  4
static uint8_t autoSaving = 0;

/////////////////////////// IMAGES //////////////////////////
#include "src/photos.h"

////////////////////////// UI LOOP //////////////////////////
#define ADDITIONAL_TASK_PERIOD 1000 // [msec]
#define DO_EVERY(period, prev) \
static uint32_t prev = 0; for (uint32_t now = millis(); now - prev >= period; prev = now)

////////////////////// LOCAL FUNCTIONS //////////////////////
//--------------------------------------------------------------------------------
// Display a covoer photo on SD or flash
//--------------------------------------------------------------------------------
static void display_photo(uint32_t playNo) {
  static constexpr lv_style_const_prop_t style_prop_photo[] = {
    LV_STYLE_CONST_SHADOW_WIDTH(10),
    LV_STYLE_CONST_SHADOW_OFFSET_Y(5),
    LV_STYLE_CONST_SHADOW_OPA(LV_OPA_40),
    LV_STYLE_CONST_PROPS_END
  };
  static LV_STYLE_CONST_INIT(style_photo, (void*)style_prop_photo);

  // Display an image file on SD card
  char buf[BUF_SIZE], *ptr;
  buf[0] = MY_FS_ARDUINO_SD_LETTER;
  buf[1] = ':';

  // Skip drive letter "S:"
  std::string dir = player.GetFilePath(playNo);
  strncpy(&buf[2], dir.c_str(), sizeof(buf) - 2);
  buf[sizeof(buf) - 1] = '\0';

  // @photo.jpg
  if (ptr = strrchr(buf, '/')) {
    strcpy(ptr + 1, ALBUM_PHOTO_FILE ALBUM_PHOTO_EXT);
    if (SD.exists(buf + 2)) {
      lv_image_set_src(ui_AlbumImage, buf);
      lv_obj_add_style(ui_AlbumImage, &style_photo, 0);
      return;
    }
  }

  // title.jpg
  if (ptr = strrchr(buf, '.')) {
    strncpy(ptr, ALBUM_PHOTO_EXT, sizeof(ALBUM_PHOTO_EXT));
    if (SD.exists(buf + 2)) {
      lv_image_set_src(ui_AlbumImage, buf);
      lv_obj_add_style(ui_AlbumImage, &style_photo, 0);
      return;
    }
  }

#ifdef _PHOTOS_H_
  #if true
    // Display an image file on flash ROM at random
    int pictNo = millis() % (N_PHOTOS - 1) + 1;
    lv_image_set_src(ui_AlbumImage, photos[pictNo]);
    lv_obj_add_style(ui_AlbumImage, &style_photo, 0);
  #else
    // Display an image specified by the number in @photo.txt
    int pictNo = player.GetPhotoNo(playNo);
    if (0 < pictNo && pictNo < N_PHOTOS) {
      lv_image_set_src(ui_AlbumImage, photos[pictNo]);
      lv_obj_add_style(ui_AlbumImage, &style_photo, 0);
    } else {
      lv_image_set_src    (ui_AlbumImage, &img_album);
      lv_obj_remove_style (ui_AlbumImage, &style_photo, 0);
    }
  #endif
#endif
}

//--------------------------------------------------------------------------------
// Control next/previous play or stop/continuous play
//--------------------------------------------------------------------------------
static bool play_next(bool next) {
  if (ui_ScreenPlayList) {
    ui_list_update_cell(ui_control.focusNo, false);
    ui_list_update_icon(ui_control.playNo,  false);
  }

  bool ret = true;
  if (ui_setting.favorite) {
    ret = player.NextSelected(next, (ui_setting.repeat ? true : false));
  } else {
    if (next) {
      player.PlayNext();
    } else {
      player.PlayPrev();
    }
  }

  bitClear(ui_setting.repeat, 7); // clear the bit that has been temporarily forced set

  ui_control.playNo = ui_control.focusNo = player.GetPlayNo();
  display_photo(ui_control.playNo);

  // Update ui_control and look of the play button
  lv_obj_set_state(ui_ButtonPlay, LV_STATE_CHECKED, true);

  if (ui_ScreenPlayList) {
    ui_list_update_play(ui_control.playNo, true);
  }

  return ret;
}

static void play_stop(void) {
  lv_obj_set_state(ui_ButtonPlay, LV_STATE_CHECKED, false);
  player.StopPlay();
}

//--------------------------------------------------------------------------------
// Update bar and label according to elapsed time
//--------------------------------------------------------------------------------
static void update_elapsed_time(void) {
  uint32_t duration = audioGetDuration();
  uint32_t elapsed  = audioGetElapsedTime();

  if (duration < elapsed) {
    duration = elapsed;
  }

  if (duration) {
    id3tags.meta.duration = duration;
  }

  lv_slider_set_range(ui_ElapsedBar, 0, duration);
  lv_slider_set_value(ui_ElapsedBar, elapsed, LV_ANIM_OFF);

  lv_label_set_text_fmt(ui_ElapsedStart, "%" LV_PRIu32 ":%02" LV_PRIu32, elapsed  / 60, elapsed  % 60);
  lv_label_set_text_fmt(ui_ElapsedEnd,   "%" LV_PRIu32 ":%02" LV_PRIu32, duration / 60, duration % 60);
}

//--------------------------------------------------------------------------------
// Save / Load / Reset settings in SD
//--------------------------------------------------------------------------------
static bool save_setting(void) {
  // Save partition
  File fd = SD.open(MP3_ROOT_PATH MP3_SETTING_FILE, FILE_WRITE);
  if (fd) {
    fd.write((uint8_t *)&ui_setting, sizeof(ui_setting));
    fd.close();
    return true;
  } else {
    player.SetError("can't save " MP3_ROOT_PATH MP3_SETTING_FILE);
    return false;
  }
}

static bool load_setting(void) {
  File fd = SD.open(MP3_ROOT_PATH MP3_SETTING_FILE, FILE_READ);
  if (!fd) {
    player.SetError("can't load " MP3_ROOT_PATH MP3_SETTING_FILE);
    return false;
  }

  // Save the sleep timer setting
  uint8_t sleepTimer = ui_setting.selectSleepTimer;

  fd.read((uint8_t *)&ui_setting, sizeof(ui_setting));
  fd.close();

  // Update the saved sleep timer setting
  ui_setting.selectSleepTimer = sleepTimer;

  // Check if the partition exists
  char buf[BUF_SIZE];
  uint8_t partition_max = 0;
  for (int i = 1; i <= PARTITION_MAX; i++) {
    snprintf(buf, sizeof(buf), MP3_ROOT_PATH PARTITION_PATH, i);
    buf[sizeof(buf) - 1] = '\0';
    if (SD.exists(buf)) {
      partition_max = i;
    } else {
      break;
    }
  }

  bool save = false;

  if (ui_setting.partition_max != partition_max) {
    ui_setting.partition_max = partition_max;
    save = true;
  }

  // Just in case
  if (ui_setting.partition_id > ui_setting.partition_max) {
    ui_setting.partition_id = 0;
    save = true;
  }

  if (save && save_setting() == false) {
    return false;
  }

  // Change the root folder with the current partition
  if (ui_setting.partition_max && ui_setting.partition_id) {
    snprintf(buf, sizeof(buf), PARTITION_PATH, ui_setting.partition_id);
    buf[sizeof(buf) - 1] = '\0';
    player.SetSubDir(buf);
  } else {
    player.SetSubDir("");
  }

  // Update UI
  lv_obj_set_state(ui_Repeat,   LV_STATE_CHECKED, ui_setting.repeat  );
  lv_obj_set_state(ui_Shuffle,  LV_STATE_CHECKED, ui_setting.shuffle );
  lv_obj_set_state(ui_Favorite, LV_STATE_CHECKED, ui_setting.favorite);

  // Rewind
  ui_control.playNo = ui_control.focusNo = 0;

  return true;
}

static bool reset_setting(void) {
  // Stop playback before saving settings to avoid conflict with SD access
  play_stop();
  player.DeleteNodeTree();
  player.ClearAudioFiles();
  return save_setting();
}

//--------------------------------------------------------------------------------
// Update data to be automatically saved when playback reaches the end of file
//--------------------------------------------------------------------------------
static bool auto_saving(void) {
  if (autoSaving) {
    bool pause = false;
    if (player.IsPlaying()) {
      pause = true;
      player.PauseResume();
    }

    // Update the playback duration at the end of file
    if (autoSaving & SAVE_DURATION) {
      uint32_t playNo = player.GetPlayNo();
      ui_list_update_duration(playNo, id3tags.meta.duration);
      if (player.PutMetaData(playNo, &id3tags.meta)) {
        autoSaving ^= SAVE_DURATION;
      }
    }

    // Update all favorites that have been modified during playback
    if (autoSaving & SAVE_FAVORITE) {
      if (player.UpdateMetaData()) {
        autoSaving ^= SAVE_FAVORITE;
      }
    }

    // Update favorite and repeat when they are changed during playback
    if (autoSaving & SAVE_SETTING) {
      if (save_setting()) {
        autoSaving ^= SAVE_SETTING;
      }
    }

    if (pause) {
      player.PauseResume();
    }

    if (autoSaving) {
      autoSaving = 0; // Avoid infinite loop
      return false;
    }
  }

  return true;
}

//--------------------------------------------------------------------------------
// Scan SD card for audio files and create a playlist
//--------------------------------------------------------------------------------
static bool create_playlist(void) {
  // Setup UI setting
  if (load_setting() == false) {
    return false;
  }

  // Scan SD card for album folders
  if (player.ScanAlbumDirs()) {
    // Load album list
    ui_album_load((void*)player.m_tree);

    // Scan audio files base on album list
    if (player.ScanAudioFiles(ui_setting.partition_id, ui_setting.shuffle)) {
      ui_set_playNo(ui_control.playNo);
      return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------
// Check if the currently playing audio file is favorite
//--------------------------------------------------------------------------------
static bool check_favorite(void) {
  return !ui_setting.favorite || player.IsPlaying() || player.IsSelected();
}

//--------------------------------------------------------------------------------
// Asynchronous function to reduce delays during screen transitions
//--------------------------------------------------------------------------------
static void stop_async(void *user_data) {
  if (ui_state != UI_STATE_IDLE) {
    ui_state = UI_STATE_STOP;
  }
}

//--------------------------------------------------------------------------------
// Helper function for screen transition
//--------------------------------------------------------------------------------
static void change_screen(lv_obj_t ** screen, lv_screen_load_anim_t fademode, void (*screen_init)(void)) {
  if(*screen == NULL) {
    screen_init();
  }
  lv_screen_load_anim(*screen, fademode, 500, 0, false);
}

////////////////////// GLOBAL FUNCTIONS /////////////////////
//********************************************************************************
// SCREEN: ui_ScreenMain event handlers
//********************************************************************************
void ui_event_ScreenMain(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_GESTURE);

  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

  if (dir == LV_DIR_RIGHT) {
    change_screen(&ui_ScreenAlbumList, LV_SCR_LOAD_ANIM_MOVE_RIGHT, &ui_ScreenAlbumList_screen_init);
  }

  else if (dir == LV_DIR_LEFT) {
    change_screen(&ui_ScreenPlayList, LV_SCR_LOAD_ANIM_MOVE_LEFT, &ui_ScreenPlayList_screen_init);
    ui_list_focus_playing(player.GetPlayNo());
  }

  else if (dir == LV_DIR_TOP || dir == LV_DIR_BOTTOM) {
    lv_screen_load_anim_t anim = (dir == LV_DIR_TOP ? LV_SCR_LOAD_ANIM_MOVE_TOP : LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    change_screen(&ui_ScreenSetting, anim, &ui_ScreenSetting_screen_init);
  }
}

void ui_event_GoToAlbumList(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  change_screen(&ui_ScreenAlbumList, LV_SCR_LOAD_ANIM_MOVE_RIGHT, &ui_ScreenAlbumList_screen_init);
}

void ui_event_GoToPlayList(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  change_screen(&ui_ScreenPlayList, LV_SCR_LOAD_ANIM_MOVE_LEFT, &ui_ScreenPlayList_screen_init);
}

void ui_event_GoToSettings(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  change_screen(&ui_ScreenSetting, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, &ui_ScreenSetting_screen_init);
}

void ui_event_Favorite(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  lv_obj_t *obj = lv_event_get_target_obj(e);
  ui_setting.favorite = (lv_obj_get_state(obj) & LV_STATE_CHECKED ? true : false);

  // If unable to save, save at idle state
  if (player.IsPlaying()) {
    autoSaving |= SAVE_SETTING;
  } else if (save_setting() == false) {
    ui_state = UI_STATE_ERROR;
  }
}

void ui_event_Repeat(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  lv_obj_t *obj = lv_event_get_target_obj(e);
  ui_setting.repeat = (uint8_t)(lv_obj_get_state(obj) & LV_STATE_CHECKED ? true : false);

  // If unable to save, save at idle state
  if (player.IsPlaying()) {
    autoSaving |= SAVE_SETTING;
  } else if (save_setting() == false) {
    ui_state = UI_STATE_ERROR;
  }
}

void ui_event_Shuffle(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  lv_obj_t *obj = lv_event_get_target_obj(e);
  ui_setting.shuffle = (lv_obj_get_state(obj) & LV_STATE_CHECKED ? true : false);
  player.StopPlay();
  player.ClearAudioFiles();

  if (save_setting()) {
    ui_state = UI_STATE_START;
  } else {
    ui_state = UI_STATE_ERROR;
  }
}

void ui_event_ButtonPlay(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  lv_state_t state = lv_obj_get_state(ui_ButtonPlay);
  ui_state = (state & LV_STATE_CHECKED ? UI_STATE_RESUME : UI_STATE_PAUSE);
}

void ui_event_ButtonNext(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  ui_state = UI_STATE_NEXT;
  bitSet(ui_setting.repeat, 7); // Force to set the bit temporarily
}

void ui_event_ButtonPrev(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  ui_state = UI_STATE_PREV;
  bitSet(ui_setting.repeat, 7); // Force to set the bit temporarily
}

void ui_event_VolumeMax(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  int vol = lv_slider_get_value(ui_Volume) + 1;
  vol = constrain(vol, MP3_VOLUME_MIN, MP3_VOLUME_MAX);
  lv_slider_set_value(ui_Volume, vol, LV_ANIM_OFF);
  player.SetVolume(vol);
}

void ui_event_VolumeMin(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  int vol = lv_slider_get_value(ui_Volume) - 1;
  vol = constrain(vol, MP3_VOLUME_MIN, MP3_VOLUME_MAX);
  lv_slider_set_value(ui_Volume, vol, LV_ANIM_OFF);
  player.SetVolume(vol);
}

void ui_event_Volume(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED);

  int vol = lv_slider_get_value(ui_Volume);
  player.SetVolume(vol);
}

void ui_event_ElapsedBar(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED);

  if (player.IsPlaying()) {
    audioSetElapsedTime(lv_slider_get_value(ui_ElapsedBar));
  }
}

//********************************************************************************
// SCREEN: ui_ScreenAlbumList event handlers
//********************************************************************************
void ui_event_ScreenAlbumList(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  DBG_ASSERT(
    event_code == LV_EVENT_CLICKED ||
    event_code == LV_EVENT_GESTURE ||
    event_code == LV_EVENT_SCREEN_LOADED ||
    event_code == LV_EVENT_SCREEN_UNLOADED
  );

  if (event_code == LV_EVENT_CLICKED) {
    change_screen(&ui_ScreenMain, LV_SCR_LOAD_ANIM_MOVE_LEFT, &ui_ScreenMain_screen_init);
  }

  else if (event_code == LV_EVENT_GESTURE) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_RIGHT || dir == LV_DIR_LEFT) {
      lv_screen_load_anim_t anim = (dir == LV_DIR_RIGHT ? LV_SCR_LOAD_ANIM_MOVE_RIGHT : LV_SCR_LOAD_ANIM_MOVE_LEFT);
      change_screen(&ui_ScreenMain, anim, &ui_ScreenMain_screen_init);
    }
  }

  else if (event_code == LV_EVENT_SCREEN_LOADED) {
    // Increase free memory
    // lv_fs_clear_cache(); // sdfs.{h|cpp}

    // Create album list (No need to access SD card)
    ui_album_create((void*)player.m_tree);

    // Stop playback to avoid conflict with SD access (async required)
    lv_async_call(stop_async, NULL);
  }

  else if (event_code == LV_EVENT_SCREEN_UNLOADED) {
    ui_ScreenAlbumList_screen_deinit();
    ui_state = UI_STATE_CLEAR;
  }
}

//********************************************************************************
// SCREEN: ui_ScreenPlayList event handlers
//********************************************************************************
void ui_event_ScreenPlayList(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  DBG_ASSERT(
    event_code == LV_EVENT_GESTURE ||
    event_code == LV_EVENT_SCREEN_UNLOADED
  );

  if (event_code == LV_EVENT_GESTURE) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    lv_screen_load_anim_t anim = dir == LV_DIR_RIGHT ? LV_SCR_LOAD_ANIM_MOVE_RIGHT : LV_SCR_LOAD_ANIM_MOVE_LEFT;
    change_screen(&ui_ScreenMain, anim, &ui_ScreenMain_screen_init);
  }

  else if (event_code == LV_EVENT_SCREEN_UNLOADED) {
    ui_ScreenPlayList_screen_deinit();
  }
}

void ui_event_PlayList_Play(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  ui_state = (ui_state == UI_STATE_PLAY ? UI_STATE_PAUSE : UI_STATE_RESUME);
  lv_obj_set_state(ui_ButtonPlay, LV_STATE_CHECKED, ui_state == UI_STATE_PAUSE ? false : true);
}

void ui_event_PlayList_Heart(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  MP3Meta_t meta;
  uint32_t track_id = (uint32_t)lv_event_get_user_data(e);
  player.GetMetaData(track_id, &meta);

  lv_obj_t *obj = (lv_obj_t*)lv_event_get_current_target(e);
  meta.selected = ui_list_get_heart_state(track_id);

  // If unable to save, save at idle state
  if (!player.PutMetaData(track_id, &meta)) {
    autoSaving |= SAVE_FAVORITE;
  }
}

//********************************************************************************
// SCREEN: ui_ScreenSetting event handlers
//********************************************************************************
void ui_event_ScreenSetting(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  DBG_ASSERT(
    event_code == LV_EVENT_CLICKED ||
    event_code == LV_EVENT_GESTURE ||
    event_code == LV_EVENT_SCREEN_UNLOADED
  );

  if (event_code == LV_EVENT_CLICKED) {
    change_screen(&ui_ScreenMain, LV_SCR_LOAD_ANIM_MOVE_TOP, &ui_ScreenMain_screen_init);
  }

  else if (event_code == LV_EVENT_GESTURE) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    lv_screen_load_anim_t anim = (dir == LV_DIR_TOP ? LV_SCR_LOAD_ANIM_MOVE_TOP : LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    change_screen(&ui_ScreenMain, anim, &ui_ScreenMain_screen_init);
  }

  else if (event_code == LV_EVENT_SCREEN_UNLOADED) {
    ui_ScreenSetting_screen_deinit();

    if (ui_setting.partition_max) {
      // Update the partition ID if a different ID is selected
      uint8_t partition_id = *(uint8_t*)lv_event_get_user_data(e);
      if (ui_setting.partition_id != partition_id) {
        ui_setting.partition_id = partition_id;

        // Stop playback before saving settings
        ui_state = UI_STATE_RESET;
      }
    }
  }
}

void ui_event_Setting_Backlight(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED);

  lv_obj_t * obj = lv_event_get_target_obj(e);
  ui_setting.selectBacklight = lv_roller_get_selected(obj);
  ui_setting_set_backlight();

  // If unable to save, save at idle state
  if (player.IsPlaying()) {
    autoSaving |= SAVE_SETTING;
  } else if (save_setting() == false) {
    ui_state = UI_STATE_ERROR;
  }
}

void ui_event_Setting_SleepTimer(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED);

  lv_obj_t * obj = lv_event_get_target_obj(e);
  ui_setting.selectSleepTimer = lv_roller_get_selected(obj);
  ui_setting_set_sleeptime();
  ui_control.sleepStart = millis();
}

//--------------------------------------------------------------------------------
// Redraw the display panel when waking up from sleep
//--------------------------------------------------------------------------------
void ui_redisplay(void) {
  lv_display_trigger_activity(NULL);
  lv_screen_load(lv_screen_active());
}

//--------------------------------------------------------------------------------
// Start to play with the specified track
//--------------------------------------------------------------------------------
void ui_set_playNo(uint32_t track_id) {
  // Start the specified track to play
  player.SetPlayNo(track_id);

  // Update ui_control
  ui_control.playNo = ui_control.focusNo = track_id;
  display_photo(track_id);

  // Update the look of the play button
  if (ui_state != UI_STATE_PLAY) {
    lv_obj_set_state(ui_ButtonPlay, LV_STATE_CHECKED, true);
    ui_state = UI_STATE_PLAY;
  }
}

//--------------------------------------------------------------------------------
// Get the latest information on MP3Player
//--------------------------------------------------------------------------------
const uint32_t ui_get_playNo(void) {
  return player.GetPlayNo();
}

const uint32_t ui_get_counts(void) {
  return player.GetCounts();
}

//--------------------------------------------------------------------------------
// Get ID3 tags (title, album, artist) from the file specified by id
//--------------------------------------------------------------------------------
void ui_get_id3tags(uint32_t track_id, MP3Tags_t &tags) {
  player.GetID3Tags(track_id, tags);
}

//--------------------------------------------------------------------------------
// Optional functions for audio-I2S (defined in CYD_Audio.h as a weak function)
// Note: These functions will be executed in the context of CORE 1.
//--------------------------------------------------------------------------------
void audio_id3data(const char *info) {
  // Avoid a race condition with ui_state set by audio_eof_mp3()
  if (ui_state == UI_STATE_PLAY) {
    char *p;
    if (p = strstr(info, "Title: ")) {
      id3tags.title = p + 7;
    } else
    if (p = strstr(info, "Artist: ")) {
      id3tags.artist = p + 8;
    } else
    if (p = strstr(info, "Album: ")) {
      id3tags.album = p + 7;
      ui_state = UI_STATE_ID3;
    }
  }
}

void audio_eof_mp3(const char *info) {
  // Note: When the Elapse bar is operated by hand, it will be shifted.
  MP3Meta_t meta;
  player.GetMetaData(player.GetPlayNo(), &meta);
  if (meta.duration == 0 || abs(meta.duration - id3tags.meta.duration) >= 5 /* [sec] */) {
    autoSaving |= SAVE_DURATION;
  }

  // Transition to UI_STATE_EOF once
  ui_state = UI_STATE_EOF;

  // Determine the next state
  if (!player.IsLastSong(ui_setting.favorite) || ui_setting.repeat) {
    nextState = UI_STATE_NEXT;
  } else {
    nextState = UI_STATE_STOP;
  }
}

//////////////////// UI STATE CONTROLLER ////////////////////
void ui_init(void) {
  ui_setting_set_backlight();
  ui_setting_set_sleeptime();
  ui_ScreenMain_screen_init();
  lv_screen_load(ui_ScreenMain);

  audioInit();
  ui_state = UI_STATE_INIT;
}

//--------------------------------------------------------------------------------
// A finite state machine that controls the overall operation
// The steady state can be either "UI_STATE_PLAY" or "UI_STATE_IDLE", 
// anything else is just a transient state that works as a command.
//--------------------------------------------------------------------------------
UI_State_t ui_loop(void) {
  switch (ui_state) {
    case UI_STATE_INIT:
      if (player.begin(MP3_ROOT_PATH, MP3_VOLUME_INI)) {
        lv_slider_set_value(ui_Volume, MP3_VOLUME_INI, LV_ANIM_OFF);
        ui_state = UI_STATE_START;
      } else {
        ui_state = UI_STATE_ERROR;
      }
      break;
    case UI_STATE_START:
      if (create_playlist()) {
        ui_state = UI_STATE_PLAY;
      } else {
        ui_state = UI_STATE_ERROR;
      }
      break;
    case UI_STATE_PLAY:
      if (!check_favorite()) {
        ui_state = UI_STATE_NEXT;
      } else if (!player.AutoPlay()) {
        ui_state = UI_STATE_ERROR;
      }
      break;
    case UI_STATE_STOP:
      play_stop();
      ui_state = UI_STATE_IDLE;
      break;
    case UI_STATE_PAUSE:
      player.PauseResume();
      ui_state = UI_STATE_IDLE;
      break;
    case UI_STATE_RESUME:
      player.PauseResume();
      ui_state = UI_STATE_PLAY;
      break;
    case UI_STATE_NEXT:
      ui_state = play_next(true)  ? UI_STATE_PLAY : UI_STATE_STOP;
      break;
    case UI_STATE_PREV:
      ui_state = play_next(false) ? UI_STATE_PLAY : UI_STATE_STOP;
      break;
    case UI_STATE_ID3:
      lv_label_set_text_fmt(ui_MusicTitle, "%s %s / %s / %s",
        LV_SYMBOL_AUDIO, id3tags.title.c_str(), id3tags.artist.c_str(), id3tags.album.c_str()
      );
      ui_state = UI_STATE_PLAY;
      break;
    case UI_STATE_EOF:
      ui_state = auto_saving() ? nextState : UI_STATE_ERROR;
      break;
    case UI_STATE_RESET:
      ui_state = reset_setting() ? UI_STATE_START : UI_STATE_ERROR;
      break;
    case UI_STATE_CLEAR:
      play_stop();
      player.ClearAudioFiles();
      ui_state = UI_STATE_START;
      break;
    case UI_STATE_ERROR:
      lv_label_set_text_fmt(ui_MusicTitle, "%s %s", LV_SYMBOL_WARNING, player.GetError());
      ui_state = UI_STATE_IDLE;
    case UI_STATE_IDLE:
    default:
      if (!auto_saving()) {
        ui_state = UI_STATE_ERROR;
      }
      break;
  }

  UI_State_t ret = UI_STATE_AWAKE;

  // Additional periodic task
  DO_EVERY(ADDITIONAL_TASK_PERIOD, task1Time) {
    // update elapsed time
    if (player.IsPlaying()) {
      update_elapsed_time();
    }

    // deep sleep
    if (ui_setting.selectSleepTimer) {
      if (millis() - ui_control.sleepStart > ui_control.sleepTimer) {
        ret = ui_state = UI_STATE_SLEEP; // enter deep sleep
      }
    }

    // Backlight control according to the duration of non-operation
    if (ui_setting.selectBacklight && ret == UI_STATE_AWAKE) {
      if (lv_disp_get_inactive_time(NULL) > ui_control.backlightTimer) {
        ret = UI_STATE_BLOFF; // turn backlight off
      }
    }
  }

  return ret;
}