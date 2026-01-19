//================================================================================
// MP3 Music Player for CYD - Playist Screen
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//  Released under the GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html)
//  LVGL version: 9.2.2 and up
//================================================================================
#include "ui.h"

//--------------------------------------------------------------------------------
// Instance of the screen widget
//--------------------------------------------------------------------------------
lv_obj_t *ui_ScreenPlayList;

#include "MP3Player.h"
extern void ui_get_id3tags(uint32_t track_id, MP3Tags_t &tags); // Defined in ui.cpp (it needs MP3Player.h)

/////////////////////////// MACROS //////////////////////////
#define LIST_LABEL_MARGIN         100 // Left(50) + Right(50)
#define LIST_SLIDER_PADDING       3   // Slider Padding: 4px
#define LIST_SLIDER_WIDTH         5   // Slider Width: 5px
#define LIST_SLIDER_SCALE         10  // Scale factor for precise slider movement
#define LIST_UPDATE_SCROLL_POS    5   // Scroll Position to update playlist to add/remove cells
#define CELL_VISIBLE_MAX          6   // Number of visible cells (SCREEN_HEIGHT <= CELL_VISIBLE_MAX * CELL_OUTLINE_HEIGHT)
#define CELL_VISIBLE_SPARE        2   // Add 1 cell each to the top and the bottom
#define CELL_OUTLINE_HEIGHT       54  // SCREEN_HEIGHT / CELL_VISIBLE_MAX + 1
#define CELL_CONTENT_HEIGHT       52  // CELL_OUTLINE_HEIGHT - BORDER TOP(1px) - OUTLINE BUTTOM(1px)
#define CELL_HEART_SIZE           15  // img_heart_{on|off}_small.header.{w|h}
#define CELL_BORDER_COLOR         { .blue = 0x5c, .green = 0x5c, .red = 0x5c }
#define CELL_OUTLINE_COLOR        { .blue = 0x44, .green = 0x44, .red = 0x44 }

////////////////////// STATIC VARIABLES /////////////////////
static lv_obj_t *play_list;
static lv_obj_t *slider;
static bool update_scroll_running = false;

static constexpr lv_style_const_prop_t style_cell_prop[] = {
  LV_STYLE_CONST_BG_OPA(LV_OPA_TRANSP),
  LV_STYLE_CONST_LAYOUT(LV_LAYOUT_GRID),
  LV_STYLE_CONST_PAD_TOP(5),
  LV_STYLE_CONST_PAD_RIGHT(20),
  LV_STYLE_CONST_MARGIN_TOP(1),
  LV_STYLE_CONST_MARGIN_BOTTOM(1),
  LV_STYLE_CONST_OUTLINE_COLOR(CELL_OUTLINE_COLOR),
  LV_STYLE_CONST_OUTLINE_WIDTH(1),
  LV_STYLE_CONST_BORDER_WIDTH(2),
  LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_TOP),
  LV_STYLE_CONST_BORDER_COLOR(CELL_BORDER_COLOR),
  LV_STYLE_CONST_PROPS_END
};
static constexpr lv_style_const_prop_t style_cell_pressed_prop[] = {
  LV_STYLE_CONST_BG_OPA(LV_OPA_COVER),
  LV_STYLE_CONST_BG_COLOR(UI_COLOR_LIST_DEFAULT),
  LV_STYLE_CONST_PROPS_END
};
static constexpr lv_style_const_prop_t style_cell_checked_prop[] = {
  LV_STYLE_CONST_BG_OPA(LV_OPA_COVER),
  LV_STYLE_CONST_BG_COLOR(UI_COLOR_LIST_DEFAULT),
  LV_STYLE_CONST_PROPS_END
};
static constexpr lv_style_const_prop_t style_title_prop[] = {
  LV_STYLE_CONST_WIDTH(SCREEN_WIDTH - LIST_LABEL_MARGIN),
  LV_STYLE_CONST_HEIGHT(CUSTOM_FONT_MEDIUM_HEIGHT),
  LV_STYLE_CONST_TEXT_FONT(&CUSTOM_FONT_MEDIUM),
  LV_STYLE_CONST_TEXT_COLOR(UI_COLOR_BACKGROUND),
  LV_STYLE_CONST_PROPS_END
};
static constexpr lv_style_const_prop_t style_artist_prop[] = {
  LV_STYLE_CONST_WIDTH(SCREEN_WIDTH - LIST_LABEL_MARGIN),
  LV_STYLE_CONST_HEIGHT(CUSTOM_FONT_SMALL_HEIGHT),
  LV_STYLE_CONST_PAD_RIGHT(5),
  LV_STYLE_CONST_TEXT_FONT(&CUSTOM_FONT_SMALL),
  LV_STYLE_CONST_TEXT_COLOR(UI_COLOR_LIST_ARTIST),
  LV_STYLE_CONST_PROPS_END
};
static constexpr lv_style_const_prop_t style_time_prop[] = {
  LV_STYLE_CONST_TEXT_FONT(&CUSTOM_FONT_SMALL),
  LV_STYLE_CONST_TEXT_COLOR(UI_COLOR_BACKGROUND),
  LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(style_cell,          (void*)style_cell_prop);
static LV_STYLE_CONST_INIT(style_cell_pressed,  (void*)style_cell_pressed_prop);
static LV_STYLE_CONST_INIT(style_cell_checked,  (void*)style_cell_checked_prop);
static LV_STYLE_CONST_INIT(style_title,         (void*)style_title_prop);
static LV_STYLE_CONST_INIT(style_artist,        (void*)style_artist_prop);
static LV_STYLE_CONST_INIT(style_time,          (void*)style_time_prop);

static constexpr int32_t col_dsc[] = { LV_GRID_CONTENT, LV_GRID_FR(1), CELL_HEART_SIZE, LV_GRID_TEMPLATE_LAST };
static constexpr int32_t row_dsc[] = { CUSTOM_FONT_MEDIUM_HEIGHT, CUSTOM_FONT_SMALL_HEIGHT, LV_GRID_TEMPLATE_LAST };

////////////////////// STATIC FUNCTIONS /////////////////////
//--------------------------------------------------------------------------------
// Gets the object specified by the track ID
//--------------------------------------------------------------------------------
static lv_obj_t *get_cell_obj(uint32_t track_id) {
  if (play_list && ui_control.top <= track_id && track_id <= ui_control.end) {
    return lv_obj_get_child(play_list, track_id - ui_control.top);
  } else {
    return NULL;
  }
}

static lv_obj_t *get_heart_obj(uint32_t track_id) {
  lv_obj_t *cell = get_cell_obj(track_id);
  if (cell) {
    return lv_obj_get_child(cell, 4); // 4: Heart Checkbox
  } else {
    return NULL;
  }
}

//--------------------------------------------------------------------------------
// Callback function to handle user interactions
//--------------------------------------------------------------------------------
static void event_handler(lv_event_t* e) {
  uint32_t track_id = (uint32_t)lv_event_get_user_data(e);

  lv_point_t p;
  lv_indev_get_point(lv_indev_get_act(), &p);

  // If the clicked coordinates within the play/pause icon area...
  if (p.x <= img_list_play.header.w) {
    // If currently playing...
    if (ui_control.playNo == track_id) {
      // Toggle pause/resume
      ui_event_PlayList_Play(e);
    }
    // Else select the new track to play
    else {
      ui_list_update_play(ui_control.playNo,  false);
      ui_list_update_cell(ui_control.focusNo, false);
      ui_list_update_play(track_id, true);
      ui_set_playNo(track_id);
    }
  }
  // Else just change the focused cell
  else {
    ui_list_update_cell(ui_control.focusNo, false);
    ui_control.focusNo = track_id;
    ui_list_update_cell(track_id, true);

    // If clicked coordinates within the heart icon area...
    p.x = SCREEN_WIDTH - p.x;
    if (img_heart_on_small.header.w <= p.x && p.x <= img_heart_on_small.header.w * 3) {
      lv_obj_t *obj = get_heart_obj(track_id);
      const void* src = lv_image_get_src(obj);
      lv_image_set_src(obj, src == (const void*)&img_heart_on_small ? &img_heart_off_small : &img_heart_on_small);

      // Update meta data
      ui_event_PlayList_Heart(e);
    }
  }
}

//--------------------------------------------------------------------------------
// Create a new cell and add it to the playlist
//--------------------------------------------------------------------------------
static lv_obj_t *add_list_cell(lv_obj_t* parent, uint32_t track_id) {
  lv_obj_t* cell = lv_obj_create(parent);
  lv_obj_remove_style_all       (cell);
  lv_obj_set_size               (cell, lv_pct(100), CELL_CONTENT_HEIGHT);
  lv_obj_set_grid_dsc_array     (cell, col_dsc, row_dsc);
  lv_obj_add_style              (cell, &style_cell, 0);
  lv_obj_add_style              (cell, &style_cell_pressed, LV_STATE_PRESSED);
  lv_obj_add_style              (cell, &style_cell_checked, LV_STATE_CHECKED);
  lv_obj_add_event_cb           (cell, event_handler, LV_EVENT_CLICKED, (void*)track_id);
  lv_obj_remove_flag            (cell, LV_OBJ_FLAG_SCROLLABLE); // Stop sliding horizontally

  MP3Tags_t tags;
  ui_get_id3tags(track_id, tags);

  ///////////////////// 0: Play Button //////////////////////
  lv_obj_t* obj = lv_image_create(cell);
  lv_image_set_src              (obj, &img_list_play);
  lv_obj_set_grid_cell          (obj, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 2);

  ///////////////////// 1: Title Label //////////////////////
  obj = lv_label_create(cell);
  lv_label_set_text             (obj, tags.title.c_str());
  lv_label_set_long_mode        (obj, LV_LABEL_LONG_DOT);
  lv_obj_set_grid_cell          (obj, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_add_style              (obj, &style_title, 0);

  ///////////////////// 2: Artist Label /////////////////////
  obj = lv_label_create(cell);
  lv_label_set_text_fmt         (obj, "%s / %s", tags.artist.c_str(), tags.album.c_str());
  lv_label_set_long_mode        (obj, LV_LABEL_LONG_DOT);
  lv_obj_set_grid_cell          (obj, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_add_style              (obj, &style_artist, 0);

  ///////////////////// 3: Time Label ///////////////////////
  obj = lv_label_create(cell);
  lv_label_set_text_fmt         (obj, "%" LV_PRIu32 ":%02" LV_PRIu32, tags.meta.duration / 60UL, tags.meta.duration % 60UL);
  lv_obj_set_grid_cell          (obj, LV_GRID_ALIGN_END, 2, 1, LV_GRID_ALIGN_END, 0, 2);
  lv_obj_add_style              (obj, &style_time, 0);

  //////////////////// 4: Heart Checkbox ////////////////////
  obj = lv_image_create(cell);
  lv_image_set_src              (obj, tags.meta.selected ? &img_heart_on_small : &img_heart_off_small);
  lv_obj_set_grid_cell          (obj, LV_GRID_ALIGN_END, 2, 1, LV_GRID_ALIGN_START, 0, 2);

  return cell;
}

//--------------------------------------------------------------------------------
// Overwrite the existing cell data
//--------------------------------------------------------------------------------
static void put_list_cell(lv_obj_t* obj, uint32_t track_id) {
  MP3Tags_t tags;
  ui_get_id3tags(track_id, tags);

  lv_image_set_src      (lv_obj_get_child(obj, 0), &img_list_play);
  lv_label_set_text     (lv_obj_get_child(obj, 1), tags.title.c_str());
  lv_label_set_text_fmt (lv_obj_get_child(obj, 2), "%s / %s", tags.artist.c_str(), tags.album.c_str());
  lv_label_set_text_fmt (lv_obj_get_child(obj, 3), "%" LV_PRIu32 ":%02" LV_PRIu32, tags.meta.duration / 60UL, tags.meta.duration % 60UL);
  lv_image_set_src      (lv_obj_get_child(obj, 4), tags.meta.selected ? &img_heart_on_small : &img_heart_off_small);

  // Update user data
  lv_obj_remove_event   (obj, 0);
  lv_obj_add_event_cb   (obj, event_handler, LV_EVENT_CLICKED, (void*)track_id);

  // Reset cell style
  ui_list_update_cell   (track_id, false);
}

//--------------------------------------------------------------------------------
// Update the scroll bar position
//--------------------------------------------------------------------------------
static void update_slider(lv_obj_t *obj) {
  lv_obj_update_layout(obj);

  // Set the top and bottom visible cells
  int32_t top = ui_control.top * LIST_SLIDER_SCALE + (lv_obj_get_scroll_top   (obj) * LIST_SLIDER_SCALE) / CELL_OUTLINE_HEIGHT;
  int32_t end = ui_control.end * LIST_SLIDER_SCALE - (lv_obj_get_scroll_bottom(obj) * LIST_SLIDER_SCALE) / CELL_OUTLINE_HEIGHT;

  lv_bar_set_start_value(slider, top, LV_ANIM_OFF);
  lv_bar_set_value      (slider, end, LV_ANIM_OFF);
  lv_obj_update_layout  (slider);
}

//--------------------------------------------------------------------------------
// Update the playlist in view according to the scrolling
//--------------------------------------------------------------------------------
static void update_scroll(lv_obj_t *obj) {
  // Do not re-enter this function when `lv_obj_scroll_by` triggers this callback again.
  if (update_scroll_running) return;
  update_scroll_running = true;

  // Scroll DOWN (Swipe UP)
  while (ui_control.end < ui_get_counts() - 1 && lv_obj_get_scroll_bottom(obj) <= LIST_UPDATE_SCROLL_POS) {
    ++ui_control.end;
//  if (ui_control.end - ui_control.top >= CELL_VISIBLE_MAX) {
      ++ui_control.top;
//  }
    lv_obj_t *top = lv_obj_get_child(obj, 0);
    lv_obj_move_to_index(top, -1);
    put_list_cell(top, ui_control.end);
    lv_obj_scroll_by(obj, 0, lv_obj_get_height(top), LV_ANIM_OFF);
    lv_obj_update_layout(obj);
    DBG_EXEC({
      show_ui_control();
    });
  }

  // Scroll UP (Swipe DOWN)
  while (ui_control.top > 0 && lv_obj_get_scroll_top(obj) <= LIST_UPDATE_SCROLL_POS) {
    --ui_control.top;
//  if (ui_control.end - ui_control.top >= CELL_VISIBLE_MAX) {
      --ui_control.end;
//  }
    lv_obj_t *end = lv_obj_get_child(obj, -1);
    lv_obj_move_to_index(end, 0);
    put_list_cell(end, ui_control.top);
    lv_obj_scroll_by(obj, 0, -lv_obj_get_height(end), LV_ANIM_OFF);
    lv_obj_update_layout(obj);
    DBG_EXEC({
      show_ui_control();
    });
  }

  // Always less than equal CELL_VISIBLE_MAX + CELL_VISIBLE_SPARE cells are allocated
  DBG_ASSERT(ui_control.end - ui_control.top + 1 <= CELL_VISIBLE_MAX + CELL_VISIBLE_SPARE);

  update_slider(obj);
  ui_list_update_cell(ui_control.focusNo, true);
  ui_list_update_icon(ui_control.playNo,  true);

  DBG_EXEC({
    dump_play_list();
  });

  update_scroll_running = false;
}

//--------------------------------------------------------------------------------
// Callback function for scroll event
//--------------------------------------------------------------------------------
static void scroll_cb(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  DBG_ASSERT(event_code == LV_EVENT_SCROLL);
  DBG_ASSERT(play_list == lv_event_get_target_obj(e));
  update_scroll(play_list);
}

//--------------------------------------------------------------------------------
// Scroll the list to ensure the specified cell is visible
//--------------------------------------------------------------------------------
static void scroll_to_view(void* track_id) {
  lv_obj_t *cell = get_cell_obj((uint32_t)track_id);
  if (cell) {
    lv_obj_scroll_to_view(cell, LV_ANIM_ON);
  }
}

//--------------------------------------------------------------------------------
// Set the widget pointer to NULL when the object is deleted
//--------------------------------------------------------------------------------
static void delete_cb(lv_event_t *e) {
  lv_obj_t **obj = (lv_obj_t **)lv_event_get_user_data(e);
  static constexpr lv_obj_t ** const adrs[] = {
    &ui_ScreenPlayList,
    &play_list,
    &slider,
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
// Create cells from top to end in the list
//--------------------------------------------------------------------------------
static void create_init_cells(void) {
  #define MIN(a, b) ((a) < (b) ? (a) : (b))
  int top = ui_control.top;
  int end = ui_get_counts() - 1;

  // Adjust top/end
  if (top + CELL_VISIBLE_MAX + CELL_VISIBLE_SPARE < end) {
    end = top + CELL_VISIBLE_MAX + CELL_VISIBLE_SPARE - 1;
  } else {
    if ((top = end - CELL_VISIBLE_MAX) < 0) {
      top = 0;
    }
  }

  // Update top/end
  ui_control.top = top;
  ui_control.end = end;

  while (top <= end) {
    add_list_cell(play_list, top++);
  }

  update_slider(play_list); // update scroll bar
  ui_list_update_cell  (ui_control.focusNo, true);
  ui_list_update_icon  (ui_control.playNo,  true);
}

////////////////////// GLOBAL FUNCTIONS /////////////////////
void ui_list_update_icon(uint32_t track_id, bool state) {
  lv_obj_t* cell = get_cell_obj(track_id);
  if (cell) {
    lv_obj_t* icon = lv_obj_get_child(cell, 0);
    if (state) {
      lv_image_set_src(icon, &img_list_pause);
    } else {
      lv_image_set_src(icon, &img_list_play);
    }
  }
}

void ui_list_update_cell(uint32_t track_id, bool state) {
  lv_obj_t* cell = get_cell_obj(track_id);
  if (cell) {
    lv_obj_t* title  = lv_obj_get_child(cell, 1);
    lv_obj_t* artist = lv_obj_get_child(cell, 2);

    if (state) {
      lv_label_set_long_mode(title,  LV_LABEL_LONG_SCROLL_CIRCULAR);
      lv_label_set_long_mode(artist, LV_LABEL_LONG_SCROLL_CIRCULAR);
      lv_obj_add_state(cell, LV_STATE_CHECKED);
    } else {
      lv_label_set_long_mode(title,  LV_LABEL_LONG_DOT);
      lv_label_set_long_mode(artist, LV_LABEL_LONG_DOT);
      lv_obj_remove_state(cell, LV_STATE_CHECKED);
    }
  }
}

void ui_list_update_play(uint32_t track_id, bool state) {
  lv_obj_t* cell = get_cell_obj(track_id);
  if (cell) {
    lv_obj_t* icon   = lv_obj_get_child(cell, 0);
    lv_obj_t* title  = lv_obj_get_child(cell, 1);
    lv_obj_t* artist = lv_obj_get_child(cell, 2);

    if (state) {
      lv_image_set_src(icon, &img_list_pause);
      lv_label_set_long_mode(title,  LV_LABEL_LONG_SCROLL_CIRCULAR);
      lv_label_set_long_mode(artist, LV_LABEL_LONG_SCROLL_CIRCULAR);
      lv_obj_add_state(cell, LV_STATE_CHECKED);
      lv_async_call(scroll_to_view, (void*)track_id);
    } else {
      lv_image_set_src(icon, &img_list_play);
      lv_label_set_long_mode(title,  LV_LABEL_LONG_DOT);
      lv_label_set_long_mode(artist, LV_LABEL_LONG_DOT);
      lv_obj_remove_state(cell, LV_STATE_CHECKED);
    }
  }
}

//--------------------------------------------------------------------------------
// Focus the specified cell when it is out of range
//--------------------------------------------------------------------------------
void ui_list_focus_playing(uint32_t track_id) {
  DBG_ASSERT(play_list);

  lv_obj_t* cell = get_cell_obj(track_id);
  if (cell) {
    lv_obj_scroll_to_view(cell, LV_ANIM_OFF);
    update_slider(play_list);
  } else {
    // Remove all cells from the playlist
    lv_obj_clean(play_list);

    // Add new cells according to the specified id
    ui_control.focusNo = ui_control.top = ui_control.end = track_id;
    create_init_cells();

    scroll_to_view((void*)track_id);
    update_slider(play_list);
  }
}

//--------------------------------------------------------------------------------
// Updates the duration to the specified cell
//--------------------------------------------------------------------------------
void ui_list_update_duration(uint32_t track_id, uint32_t duration) {
  lv_obj_t* cell = get_cell_obj(track_id);
  if (cell) {
    lv_obj_t* time_label = lv_obj_get_child(cell, 3);
    lv_label_set_text_fmt(time_label, "%" LV_PRIu32 ":%02" LV_PRIu32, duration / 60, duration % 60);
  }
}

//--------------------------------------------------------------------------------
// Get the state of heart icon (true: on, false: off)
//--------------------------------------------------------------------------------
bool ui_list_get_heart_state(uint32_t track_id) {
  lv_obj_t *obj = get_heart_obj(track_id);
  if (obj) {
    const void *src = lv_image_get_src(obj);
    return (src == (const void*)&img_heart_on_small ? true : false);
  } else {
    return false;
  }
}

//--------------------------------------------------------------------------------
// This initialization is executed once
//--------------------------------------------------------------------------------
void ui_ScreenPlayList_screen_init(void) {
  if (ui_ScreenPlayList == NULL) {
    ui_ScreenPlayList = lv_obj_create(NULL);
    lv_obj_remove_flag        (ui_ScreenPlayList, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color (ui_ScreenPlayList, UI_COLOR_LIST_PRESSED, 0);
    lv_obj_set_style_bg_opa   (ui_ScreenPlayList, 255,                   0);
  }

  lv_obj_add_event_cb(ui_ScreenPlayList, ui_event_ScreenPlayList, LV_EVENT_GESTURE, NULL);
  lv_obj_add_event_cb(ui_ScreenPlayList, ui_event_ScreenPlayList, LV_EVENT_SCREEN_UNLOADED, NULL);

  // Create an empty container for playlist
  if (play_list == NULL) {
    play_list = lv_obj_create(ui_ScreenPlayList);
    lv_obj_remove_style_all   (play_list);
    lv_obj_set_size           (play_list, lv_pct(100), lv_pct(100));
    lv_obj_set_align          (play_list, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow      (play_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb       (play_list, scroll_cb, LV_EVENT_SCROLL, NULL);
  }

  // Creating a slider as an alternative to a scrollbar
  if (slider == NULL) {
    slider = lv_bar_create(ui_ScreenPlayList);
    lv_obj_align                (slider, LV_ALIGN_TOP_RIGHT, -2, 0);
    lv_obj_set_size             (slider, LIST_SLIDER_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_pad_bottom (slider, LIST_SLIDER_PADDING,  (uint32_t)LV_STATE_DEFAULT | (uint32_t)LV_PART_MAIN);
    lv_obj_set_style_bg_color   (slider, UI_COLOR_LIST_SLIDER, (uint32_t)LV_STATE_DEFAULT | (uint32_t)LV_PART_INDICATOR);
    lv_bar_set_mode             (slider, LV_BAR_MODE_RANGE);
    lv_bar_set_orientation      (slider, LV_BAR_ORIENTATION_VERTICAL);
    lv_bar_set_range            (slider, ((int32_t)ui_get_counts() - 1) * LIST_SLIDER_SCALE, -1);
  }

  // Add a callback when an object is deleted
  lv_obj_add_event_cb(ui_ScreenPlayList,  delete_cb, LV_EVENT_DELETE, (void*)&ui_ScreenPlayList);
  lv_obj_add_event_cb(play_list,          delete_cb, LV_EVENT_DELETE, (void*)&play_list);
  lv_obj_add_event_cb(slider,             delete_cb, LV_EVENT_DELETE, (void*)&slider);

  // Add new cells and set focus according to the ui_control
  create_init_cells();
}

void ui_ScreenPlayList_screen_deinit(void) {
  if (ui_ScreenPlayList) {
    lv_obj_delete_async(ui_ScreenPlayList);
  }
}

#if   DEBUG
//--------------------------------------------------------------------------------
// Debug functions
//--------------------------------------------------------------------------------
size_t get_cell_count(void) {
  return lv_obj_get_child_count(play_list);
}

void show_ui_control(void) {
  if (play_list) {
    MP3Tags_t tags[2];
    ui_get_id3tags(ui_control.top, tags[0]);
    ui_get_id3tags(ui_control.end, tags[1]);

    printf("top:%3d (pos: %d) \"%s\", end:%3d (pos: %d) \"%s\", playNo: %d, count: %d/%d\n",
      ui_control.top,  lv_obj_get_scroll_top   (play_list), tags[0].title.c_str(),
      ui_control.end,  lv_obj_get_scroll_bottom(play_list), tags[1].title.c_str(),
      ui_get_playNo(), lv_obj_get_child_count  (play_list), ui_get_counts()
    );
  }
}

void dump_play_list(void) {
  if (play_list) {
    const int n = lv_obj_get_child_count(play_list);
    for (int i = 0; i < n; i++) {
      MP3Tags_t tags;
      ui_get_id3tags(ui_control.top + i, tags);

      printf("key: %3d, saved: %d, selected: %d, duration: %3d, artist: %s, album: %s, title: %s\n",
        ui_control.top + i, tags.meta.saved, tags.meta.selected, tags.meta.duration,
        tags.artist.c_str(), tags.album.c_str(), tags.title.c_str()
      );
    }
  }
}
#endif // Debug functions