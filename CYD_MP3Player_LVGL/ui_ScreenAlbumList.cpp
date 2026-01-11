//================================================================================
// MP3 Music Player for CYD - Album List Screen
// LVGL version: 9.2.2 and up
//================================================================================
#include "ui.h"
#include "tree.hpp"
#include "json.hpp"
#include <stdlib.h>   // strtoul()
#include <functional> // std::hash

/////////////// Album list configuration file ///////////////
#define ALBUM_LIST_CONF ALBUM_LIST_PATH ALBUM_LIST_FILE

//--------------------------------------------------------------------------------
// Instance of the screen widget
//--------------------------------------------------------------------------------
lv_obj_t *ui_ScreenAlbumList;

////////////////// Styles for list widget ///////////////////
#define INFO_LABEL_COLOR    { .blue = 0x88, .green = 0x88, .red = 0x88 }
#define CELL_COLOR_OUTLINE  { .blue = 0xe4, .green = 0xe0, .red = 0xe4 }
#define CELL_COLOR_NODE     lv_color_hex(0xf4f4f4)
#define CELL_COLOR_LEAF     lv_color_hex(0xffffff)
#define CELL_HEIGHT_SMALL   31  // For CUSTOM_FONT_SMALL
#define CELL_HEIGHT_MEDIUM  34  // For CUSTOM_FONT_MEDIUM
#define CELL_OFFSET_NODE    6   // Offset for node text
#define CELL_OFFSET_LEAF    10  // Offset for leaf text
#define CELL_PADDING_LEFT   6   // Padding left in pixels
#define CELL_PADDING_BORDER 8   // Padding top/bottom in pixels
#define CELL_VISIBLE_MAX    7   // Number of visible cells in the album list
#define CELL_VISIBLE_SPARE  2   // Add 1 cell each to the top and the bottom
#define CELL_UPDATE_SCROLL  6   // Scroll Position to update list to add/remove cells
#define ALBUM_LIST_HEIGHT   220 // Height of the album list (CELL_HEIGHT_SMALL * CELL_VISIBLE_MAX + alpha)
#define FOLDING_DURATION    250 // Folding animation duration

#if   true
#define DROPDOWN_LIST_WIDTH 100
#define DROPDOWN_LIST_X     LV_PCT_X(42)    // Dropdown List
#define DROPDOWN_LIST_Y     LV_PCT_Y(2)+1   // Dropdown List
#define TITLE_LABEL_X       LV_PCT_X(5)     // Title Label
#define TITLE_LABEL_Y       LV_PCT_Y(5)     // Title Label
#define TOGGLE_BUTTON_X     LV_PCT_X(6)     // Button Matrix
#define TOGGLE_BUTTON_Y     LV_PCT_Y(14)    // Button Matrix
#define KEYPAD_BUTTON_X     LV_PCT_X(42)    // Button Matrix
#define KEYPAD_BUTTON_Y     LV_PCT_Y(14)    // Button Matrix
#else
#define DROPDOWN_LIST_WIDTH 90
#define DROPDOWN_LIST_X     LV_PCT_X(5)     // Dropdown List
#define DROPDOWN_LIST_Y     LV_PCT_Y(2)+1   // Dropdown List
#define TITLE_LABEL_X       LV_PCT_X(7)     // Title Label
#define TITLE_LABEL_Y       LV_PCT_Y(16)    // Title Label
#define TOGGLE_BUTTON_X     LV_PCT_X(44)    // Button Matrix
#define TOGGLE_BUTTON_Y     LV_PCT_Y(14)    // Button Matrix
#define KEYPAD_BUTTON_X     LV_PCT_X(44)    // Button Matrix
#define KEYPAD_BUTTON_Y     LV_PCT_Y(3)     // Button Matrix
#endif

#define ALBUM_LIST_X        LV_PCT_X(5)     // List Container
#define ALBUM_LIST_Y        LV_PCT_Y(24)    // List Container
#define MOVE_TO_UP_X        LV_PCT_X(87)    // Move to Main
#define MOVE_TO_UP_Y        LV_PCT_Y(4)     // Move to Main
#define MOVE_TO_RIGHT_X     LV_PCT_X(87)    // Move to Setting
#define MOVE_TO_RIGHT_Y     LV_PCT_Y(14)+3  // Move to Setting

typedef struct {
  String    name; // name in dropdown list
  size_t    hash; // hash of json document
} AlbumList_t;

typedef struct {
  Node    *root;      // root of the node tree (player.m_tree)
  int     top;        // node key at the top of the album list
  int     end;        // node key at the end of the album list
  int     count;      // number of the cells in the album list
  int     n_nodes;    // total number of the nodes in tree
  int     list_id;    // ID of the selected album list
  std::vector<AlbumList_t> list;
} AlbumControl_t;

typedef struct {
  int   n_folded;   // number of folded   nodes in tree
  int   n_selected; // number of selected leafs in tree
  int   n_files;    // number of selected audio files
} AlbumInfo_t;

//--------------------------------------------------------------------------------
// Local variables and prototype
//--------------------------------------------------------------------------------
static lv_obj_t *album_list;
static lv_obj_t *album_info;
static lv_obj_t *keypad_panel;
static lv_obj_t *dropdown_list;
static lv_obj_t *keypad_buttons;
static AlbumControl_t album_control;
static bool update_scroll_running = false;

static void event_handler(lv_event_t *e);
static void draw_image_cb(lv_event_t *e);

//--------------------------------------------------------------------------------
// Get the node pointer from the cell's userdata
//--------------------------------------------------------------------------------
static inline Node *get_node(lv_obj_t *cell) {
  return (Node*)lv_obj_get_user_data(cell);
}

//--------------------------------------------------------------------------------
// Count / Update open cells, selected nodes and selected audio files
//--------------------------------------------------------------------------------
static void count_checked(Node *node, AlbumInfo_t *info) {
  if (node->meta.type == TYPE_NODE) {
    // Is the node open?
    if (node->meta.checked == NODE_FOLDED) {
      info->n_folded++;
    }
    for (auto &n : node->children) {
      count_checked(n, info);
    }
  }
  // Is the leaf selected?
  else if (node->meta.checked == LEAF_SELECTED) {
    info->n_selected++;
    info->n_files += node->n_files;
  }
}

static AlbumInfo_t get_album_info(void) {
  AlbumInfo_t info = {0,};
  for (auto &n : album_control.root->children) {
    count_checked(n, &info);
  }
  return info;
}

//--------------------------------------------------------------------------------
// Update cells' position in the list
//--------------------------------------------------------------------------------
static inline void update_album_control(lv_obj_t *list) {
  lv_obj_update_layout(list);
  album_control.top = get_node(lv_obj_get_child(list,  0))->key;
  album_control.end = get_node(lv_obj_get_child(list, -1))->key;
  album_control.count = lv_obj_get_child_count(list);
}

//--------------------------------------------------------------------------------
// Update status label and keypad buttons
//--------------------------------------------------------------------------------
static void update_album_info(lv_obj_t *label) {
  AlbumInfo_t info = get_album_info();
  lv_label_set_text_fmt(label, "Selected album: %d, files: %d", info.n_selected, info.n_files);

  if (album_control.list_id == 0) {
    // Disable 'save' and 'trash'
    lv_buttonmatrix_set_button_ctrl(keypad_buttons, 1, LV_BUTTONMATRIX_CTRL_DISABLED);  // LV_SYMBOL_SAVE
    lv_buttonmatrix_set_button_ctrl(keypad_buttons, 2, LV_BUTTONMATRIX_CTRL_DISABLED);  // LV_SYMBOL_TRASH
  }

  else {
    JsonDocument doc;
    JsonTree::tree2json(album_control.root, doc);

    std::hash<std::string> makeHash;
    size_t hash = makeHash(doc.as<std::string>()); // 1.2[msec]

    if (hash == album_control.list[album_control.list_id].hash) {
      lv_buttonmatrix_set_button_ctrl  (keypad_buttons, 1, LV_BUTTONMATRIX_CTRL_DISABLED);  // LV_SYMBOL_SAVE
      lv_buttonmatrix_clear_button_ctrl(keypad_buttons, 2, LV_BUTTONMATRIX_CTRL_DISABLED);  // LV_SYMBOL_TRASH
    } else {
      lv_buttonmatrix_clear_button_ctrl(keypad_buttons, 1, LV_BUTTONMATRIX_CTRL_DISABLED);  // LV_SYMBOL_SAVE
      lv_buttonmatrix_clear_button_ctrl(keypad_buttons, 2, LV_BUTTONMATRIX_CTRL_DISABLED);  // LV_SYMBOL_TRASH
    }
  }
}

//--------------------------------------------------------------------------------
// Setup cell styles and properties
//--------------------------------------------------------------------------------
static void setup_cell_styles(lv_obj_t *cell, Node *node) {
  static constexpr lv_style_const_prop_t style_prop_cell[] = {
    LV_STYLE_CONST_ALIGN(LV_ALIGN_LEFT_MID),
    LV_STYLE_CONST_TEXT_FONT(&CUSTOM_FONT_SMALL),
    LV_STYLE_CONST_PAD_TOP(CELL_PADDING_BORDER),
    LV_STYLE_CONST_PAD_BOTTOM(CELL_PADDING_BORDER),
    LV_STYLE_CONST_OUTLINE_COLOR(CELL_COLOR_OUTLINE),
    LV_STYLE_CONST_OUTLINE_WIDTH(1),
    LV_STYLE_CONST_PROPS_END
  };
  static LV_STYLE_CONST_INIT(style_cell, (void*)(style_prop_cell));

  // Common styles
  if (lv_obj_get_event_count(cell) == 0) {
    lv_obj_add_style        (cell, &style_cell, (uint32_t)LV_PART_MAIN);
    lv_label_set_long_mode  (cell, LV_LABEL_LONG_CLIP); // LV_LABEL_LONG_DOT, LV_LABEL_LONG_SCROLL_CIRCULAR
    lv_obj_add_flag         (cell, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag         (cell, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    lv_obj_add_event_cb     (cell, draw_image_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_event_cb     (cell, event_handler, LV_EVENT_CLICKED, NULL);
  }

  // Individual styles
  NodeMeta_t *meta = &node->meta;
  lv_obj_set_style_height   (cell, (meta->depth > 1 && meta->hidden ? 0 : CELL_HEIGHT_SMALL), LV_PART_MAIN);
  lv_obj_set_style_bg_color (cell, (meta->type == TYPE_NODE ? CELL_COLOR_NODE : CELL_COLOR_LEAF), LV_PART_MAIN);
  lv_obj_set_style_pad_left (cell, (meta->depth * CELL_PADDING_LEFT + (meta->type == TYPE_NODE ? CELL_OFFSET_NODE: CELL_OFFSET_LEAF)), LV_PART_MAIN);
  lv_obj_set_user_data      (cell, reinterpret_cast<void*>(node));
}

//--------------------------------------------------------------------------------
// Append / Delete the specified node to the list
//--------------------------------------------------------------------------------
static inline void update_cell(lv_obj_t *cell, Node *node) {
  lv_label_set_text(cell, node->name.c_str());
  setup_cell_styles(cell, node);
}

static inline lv_obj_t *append_cell(lv_obj_t *list, Node *node) {
  DBG_EXEC({
    printf("count:%3d added   %3d \"%s\"\n", album_control.count, node->key, node->name.c_str());
  });

  lv_obj_t * cell = lv_list_add_text(list, node->name.c_str());
  setup_cell_styles(cell, node);
  return cell;
}

static inline void delete_cell(lv_obj_t *list, lv_obj_t *cell) {
  DBG_EXEC({
    Node *node = get_node(cell);
    printf("count:%3d deleted %3d \"%s\"\n", album_control.count, node->key, node->name.c_str());
  });

  lv_obj_delete(cell);
  update_album_control(list);
}

//--------------------------------------------------------------------------------
// Get the open node after/before the specified key
//--------------------------------------------------------------------------------
static Node *find_after(int key) {
  while (++key < album_control.n_nodes) {
    Node *node = album_control.root->find_preorder(key);
    DBG_ASSERT(node);
    if (node->meta.depth == 1 || node->meta.hidden == false) {
      return node;
    }
  }
  return NULL;
}

static Node *find_before(int key) {
  while (--key >= 0) {
    Node *node = album_control.root->find_preorder(key);
    if (node && (node->meta.depth == 1 || node->meta.hidden == false)) {
      return node;
    }
  }
  return NULL;
}

//--------------------------------------------------------------------------------
// Callbacks when a cell is opened or closed
//--------------------------------------------------------------------------------
static void update_close_cb(lv_anim_t* a) {
  lv_obj_t *list = (lv_obj_t*)lv_anim_get_user_data(a);
  lv_obj_t *cell = (lv_obj_t*)a->var;
  delete_cell(list, cell);
}

static void update_open_cb(lv_anim_t* a) {
  if (album_control.count > CELL_VISIBLE_MAX + CELL_VISIBLE_SPARE) {
    lv_obj_t *list = (lv_obj_t*)lv_anim_get_user_data(a);
    lv_obj_t *cell = lv_obj_get_child(list, -1);

    // Scroll to make it visible, then delete the appropriate cell
    if (a->var != cell) {
      lv_obj_scroll_to_view((lv_obj_t*)a->var, LV_ANIM_ON);
      delete_cell(list, cell); // delete the last cell
    }

    // Apply an open-to-close animation to the top cell
    else {
      lv_anim_t b;
      lv_anim_init            (&b);
      lv_anim_set_exec_cb     (&b, (lv_anim_exec_xcb_t)lv_obj_set_height);
      lv_anim_set_duration    (&b, FOLDING_DURATION);
      lv_anim_set_user_data   (&b, reinterpret_cast<void*>(list));
      lv_anim_set_values      (&b, CELL_HEIGHT_SMALL, 0);
      lv_anim_set_completed_cb(&b, update_close_cb);            // delete the top cell
      lv_anim_set_var         (&b, lv_obj_get_child(list, 0));  // after an animation
      lv_anim_start           (&b);
    }
  }
}

//--------------------------------------------------------------------------------
// Processing when opening and closing the cell
//--------------------------------------------------------------------------------
static void update_open(lv_obj_t *list, Node *node, uint32_t index, lv_anim_t *a) {
  Node *prev = node;
  std::vector <Node*> stack;
  stack.push_back(node);

  uint32_t depth = node->meta.depth;
  uint32_t d = depth + 1;

  for (int key = node->key + 1; key < album_control.n_nodes; key++) {
    node = album_control.root->find_preorder(key);
    if (node->meta.depth <= depth) {
      break;
    }

    // Update the stack depending on the depth
    if (node->meta.depth > d) {
      d = node->meta.depth;
      stack.push_back(prev);
    } else if (node->meta.depth < d) {
      d = node->meta.depth;
      stack.pop_back();
    }

    // Child's "hidden" follows parent's "checked"
    bool hidden = (stack.back()->meta.checked == NODE_UNFOLDED ? false : true);
    node->meta.hidden = hidden;

    if (hidden == false && ++index <= CELL_VISIBLE_MAX + CELL_VISIBLE_SPARE) {
      // 1. Add a new cell and re-index
      lv_obj_t *cell = append_cell(list, node);
      lv_obj_move_to_index(cell, index);
      update_album_control(list);

      // 2. Set the animation to delete the last cell
      lv_anim_set_var(a, cell);
      lv_anim_start(a);
    }

    // Update previous node
    prev = node;
  }
}

static void update_close(lv_obj_t *list, Node *node, uint32_t index, lv_anim_t *a) {
  uint32_t depth = node->meta.depth;

  // Hide all the children
  for (int key = node->key + 1; key < album_control.n_nodes; key++) {
    node = album_control.root->find_preorder(key);
    if (node->meta.depth <= depth) {
      break;
    }
    node->meta.hidden = true;
  }

  // Get the last node key in the list
  uint32_t last_key = get_node(lv_obj_get_child(list, -1))->key;

  lv_obj_t *cell;
  while (cell = lv_obj_get_child(list, ++index)) {
    node = get_node(cell);
    if (node->meta.depth <= depth) {
      break;
    }

    // 1. Apply an open-to-close animation to each cell
    lv_anim_set_var(a, cell);
    lv_anim_start(a);

    // 2. Add the same number of new cells to the end/top of the list
    if (node = find_after(last_key)) {
      // In case node is not in the last cell
      last_key = node->key;
      append_cell(list, node);
      update_album_control(list);
    } else {
      // In case node is in the last cell
      cell = lv_obj_get_child(list, 0);
      node = get_node(cell);
      node = find_before(node->key);

      if (node) {
        cell = append_cell(list, node);
        lv_obj_move_to_index(cell, 0);
        update_album_control(list);
        ++index;

        // Apply a close-to-open animation to the top cell
        lv_anim_t b;
        lv_anim_init          (&b);
        lv_anim_set_exec_cb   (&b, (lv_anim_exec_xcb_t)lv_obj_set_height);
        lv_anim_set_duration  (&b, FOLDING_DURATION);
        lv_anim_set_user_data (&b, reinterpret_cast<void*>(list));
        lv_anim_set_values    (&b, 0, CELL_HEIGHT_SMALL);
        lv_anim_set_var       (&b, cell);
        lv_anim_start         (&b);
      }
    }
  }
}

//--------------------------------------------------------------------------------
// Event handler for when a cell is clicked
//--------------------------------------------------------------------------------
static void event_handler(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_CLICKED);

  lv_obj_t *cell = lv_event_get_target_obj(e);
  lv_obj_t *list = lv_obj_get_parent(cell);

  Node *node = get_node(cell);
  NodeMeta_t *meta = &node->meta;

  if (meta->type == TYPE_NODE) {
    // Update the node status and appearance
    bool checked  = meta->checked = !meta->checked;
    lv_obj_send_event(cell, LV_EVENT_STYLE_CHANGED, NULL);

    // Setup an animation template
    lv_anim_t a;
    lv_anim_init          (&a);
    lv_anim_set_exec_cb   (&a, (lv_anim_exec_xcb_t)lv_obj_set_height);
    lv_anim_set_duration  (&a, FOLDING_DURATION);
    lv_anim_set_user_data (&a, reinterpret_cast<void*>(list));

    uint32_t index = lv_obj_get_index(cell);

    if (checked == NODE_UNFOLDED) {
      lv_anim_set_values(&a, 0, CELL_HEIGHT_SMALL);
      lv_anim_set_completed_cb(&a, update_open_cb);
      update_open(list, node, index, &a);
    }

    else /* checked == NODE_FOLDED */ {
      lv_anim_set_values(&a, CELL_HEIGHT_SMALL, 0);
      lv_anim_set_completed_cb(&a, update_close_cb);
      update_close(list, node, index, &a);
    }
  }

  else /* meta->type == TYPE_LEAF */ {
    // Update the checkbox state and appearance
    meta->checked = !meta->checked;
    update_album_info(album_info);
    lv_obj_send_event(cell, LV_EVENT_STYLE_CHANGED, NULL);
  }
}

//--------------------------------------------------------------------------------
// Draw a graphic icon for the cell
// Reference: https://docs.lvgl.io/master/details/widgets/table.html
//--------------------------------------------------------------------------------
static void draw_image_cb(lv_event_t *e) {
  lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
  lv_draw_task_type_t type  = lv_draw_task_get_type(draw_task);
  lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);

  if (base_dsc->part == LV_PART_MAIN && type == LV_DRAW_TASK_TYPE_FILL) {
    lv_obj_t *cell = lv_event_get_target_obj(e);
    NodeMeta_t *meta = &(get_node(cell)->meta);
    if (meta->depth > 1 && meta->hidden) {
      return;
    }

    // Draw icon image
    const lv_image_dsc_t *img;
    if (meta->type == TYPE_NODE) {
      img = (const lv_image_dsc_t*)(meta->checked ? &img_symbol_right : &img_symbol_down);
    } else {
      img = (const lv_image_dsc_t*)(meta->checked ? &img_checked : &img_checkbox);
    }

    lv_area_t area;
    area.x1 = 0; area.x2 = img->header.w - 1;
    area.y1 = 0; area.y2 = img->header.h - 1;

    lv_area_t draw_task_area;
    lv_draw_task_get_area(draw_task, &draw_task_area);
    lv_area_align(&draw_task_area, &area, LV_ALIGN_LEFT_MID, meta->depth * CELL_PADDING_LEFT, 0);

#if 0
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_image_src = (const void *)img;
    rect_dsc.bg_color = (meta->type == TYPE_NODE ? CELL_COLOR_NODE : CELL_COLOR_LEAF);
    lv_draw_rect(base_dsc->layer, &rect_dsc, &area);
#else
    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    img_dsc.src = (const void *)img;
    img_dsc.recolor = lv_color_black();
    lv_draw_image(base_dsc->layer, &img_dsc, &area);
#endif
  }
}

//--------------------------------------------------------------------------------
// Adjust the number of cells in the list according to the scroll
//--------------------------------------------------------------------------------
static void update_scroll(lv_obj_t *list) {
  // Do not re-enter this function when `lv_obj_scroll_by` triggers this callback again.
  if (update_scroll_running) return;
  update_scroll_running = true;

  Node *node;

  // Scroll DOWN (Swipe UP)
  while (album_control.end < album_control.n_nodes - 1 && lv_obj_get_scroll_bottom(list) <= CELL_UPDATE_SCROLL) {
    // Find the cell to be replaced
    if (node = find_after(album_control.end)) {
      lv_obj_t *top = lv_obj_get_child(list, 0); // 0: get top
      update_cell(top, node);
      lv_obj_move_to_index(top, -1);  // -1: move top to end
      lv_obj_scroll_by(list, 0, lv_obj_get_height(top), LV_ANIM_OFF);
      update_album_control(list);
      DBG_EXEC({
        album_control.root->print_node(node);
      });
    } else {
      break;
    }
  }

  // Scroll UP (Swipe DOWN)
  while (album_control.top > 0 && lv_obj_get_scroll_top(list) <= CELL_UPDATE_SCROLL) {
    // Find the cell to be replaced
    if (node = find_before(album_control.top)) {
      lv_obj_t *end = lv_obj_get_child(list, -1); // -1: get end
      update_cell(end, node);
      lv_obj_move_to_index(end, 0); // 0: move end to top
      lv_obj_scroll_by(list, 0, -lv_obj_get_height(end), LV_ANIM_OFF);
      update_album_control(list);
      DBG_EXEC({
        album_control.root->print_node(node);
      });
    } else {
      break;
    }
  }

  DBG_EXEC({
    printf("album_control.count: %d\n", album_control.count);
    dump_album_list();
  });

  update_scroll_running = false;
}

static void scroll_cb(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  DBG_ASSERT(event_code == LV_EVENT_SCROLL);
  DBG_ASSERT(album_list == lv_event_get_target_obj(e));
  update_scroll(album_list);
}

//--------------------------------------------------------------------------------
// Initialize the album list widgets with a specified number of cells
//--------------------------------------------------------------------------------
static void album_list_refresh(int key = 0) {
  update_scroll_running = true;   // Disable 'update_scroll()' once
  lv_obj_clean(album_list);
  update_scroll_running = false;  // Enable 'update_scroll()' again

  if (album_control.root) {
    int top = album_control.top = album_control.end = key - 1;
    int end = album_control.top + CELL_VISIBLE_MAX + CELL_VISIBLE_SPARE;
    if (end > album_control.n_nodes) {
      end = album_control.n_nodes;
    }

    while (top++ < end) {
      Node *node = find_after(album_control.end);
      if (node) {
        DBG_EXEC({
          NodeMeta_t *meta = &node->meta;
          printf("No.%3d, key: %3d, type: %d, depth: %d, hidden: %d, checked: %d, name: %s\n",
            top, node->key, meta->type, meta->depth, meta->hidden, meta->checked, node->name.c_str()
          );
        });
        append_cell(album_list, node);
        update_album_control(album_list);
      }
    }
  }

  update_album_info(album_info);
}

//--------------------------------------------------------------------------------
// Sets the state of all cells in the album list
//--------------------------------------------------------------------------------
static void toggle_cell_state(int type, int state) {
  const int n = album_control.n_nodes;
  for (int i = 0; i < n; i++) {
    Node *node = album_control.root->find_preorder(i);
    DBG_ASSERT(node);
    if (type == TYPE_NODE) {
      node->meta.hidden = (state == NODE_FOLDED ? NODE_HIDDEN : NODE_REVEALED);
      if (node->meta.type == TYPE_NODE) {
        node->meta.checked = state;
      }
    } else if (node->meta.type == TYPE_LEAF) {
      node->meta.checked = state;
    }
  }
}

//--------------------------------------------------------------------------------
// Functions for manipulating JSON data / files
//  @album/
//  ├── @list.txt
//  │   ├── Current dropdown selection
//  │   ├── "All <tab> 0"
//  │   ├── "Name <tab> Hash" for 1.json
//  │   ├── ...
//  │
//  ├── 1.json
//  ├── ...
//--------------------------------------------------------------------------------
static inline String make_json_list(void) {
  String list;
  for (auto &i : album_control.list) {
    list += i.name + "\n";
  }
  list.trim();
  return list;
}

static inline String make_json_path(int id) {
  return String(album_control.root->name.c_str()) + ALBUM_LIST_PATH + String(id) + ALBUM_LIST_JSON;
}

static bool album_json_save(void) {
  bool ret = false;

  if (album_control.list.size() && album_control.list_id > 0) {
    JsonDocument doc;
    JsonTree::tree2json(album_control.root, doc);
    size_t size = measureJson(doc);

    String path = make_json_path(album_control.list_id);
    File fd = SD.open(path.c_str(), FILE_WRITE);
    if (fd) {
      if (size == serializeJson(doc, fd)) {
        ret = true;
      } else {
        lv_label_set_text_fmt(album_info, "failed saving json to %s\n", path.c_str());
      }
      fd.close();
    }
  }

  return ret;
}

static bool album_json_load(void) {
  // Count "All"
  if (album_control.list_id == 0) {
    AlbumInfo_t info = get_album_info();
    return info.n_selected ? true : false;
  }

  else if (album_control.list.size()) {
    String path = make_json_path(album_control.list_id);
    File fd = SD.open(path.c_str(), FILE_READ);
    if (fd) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, fd);
      fd.close();
      if (!error) {
        toggle_cell_state(TYPE_LEAF, LEAF_UNSELECTED);  // < 1[msec]
        JsonTree::select_leaf(doc, album_control.root); // < 1[msec]
        return true;
      }
    }
  }

  return false;
}

static bool album_list_save(void) {
  std::string path = album_control.root->name + ALBUM_LIST_PATH;
  if (!SD.exists(path.c_str())) {
    SD.mkdir(path.c_str());
  }

  path += ALBUM_LIST_FILE;
  File fd = SD.open(path.c_str(), FILE_WRITE);
  if (fd) {
    fd.println(album_control.list_id);
    for (auto &i : album_control.list) {
      fd.printf("%s\t%lu\n", i.name.c_str(), i.hash);
    }
    fd.close();
    return true;
  } else {
    return false;
  }
}

static void album_list_load(void) {
  String buf = (album_control.root->name + ALBUM_LIST_CONF).c_str();
  File fd = SD.open(buf.c_str(), FILE_READ);

  // Read data from an existing file
  if (fd) {
    // Read 1st line: Set current list ID
    album_control.list_id = fd.readStringUntil('\n').toInt();

    // Read following lines: Create an array of name/hash pairs
    album_control.list.clear();
    while (fd.available()) {
      buf = fd.readStringUntil('\n');
      int index = buf.indexOf('\t');
      if (index > 0) { // Exclude "All"
        album_control.list.push_back({
          /* .name = */ buf.substring(0, index++),
          /* .hash = */ (size_t)strtoul(buf.substring(index).c_str(), NULL, 10)
        });
      }
    }
    fd.close();

    DBG_EXEC({
      printf("list_id: %d\n", album_control.list_id);
      for (auto &i : album_control.list) {
        printf("name: %s, hash: 0x%x\n", i.name.c_str(), i.hash);
      }
    })

    // Just in case
    if (album_json_load() == false) {
      album_control.list_id = 0;
      toggle_cell_state(TYPE_LEAF, LEAF_SELECTED);
    }
  }

  // Create a new file
  if (album_control.list.size() == 0) {
    album_control.list_id = 0;
    album_control.list.push_back({ "All", 0 });
    album_list_save();
  }
}

//--------------------------------------------------------------------------------
// Dialog box for album list deletion confirmation
//--------------------------------------------------------------------------------
static void dialog_box_cb(lv_event_t *e) {
  // https://github.com/lvgl/lvgl/blob/master/src/widgets/msgbox/lv_msgbox.c#L269-L290
  lv_obj_t *obj = lv_event_get_target_obj(e);
  obj = lv_obj_get_parent(lv_obj_get_parent(obj));
  lv_msgbox_close_async(obj);

  bool yes = (bool)lv_event_get_user_data(e);
  if (yes) {
    // Delete target file
    SD.remove(make_json_path(album_control.list_id));

    // Round up file name one by one
    int n = album_control.list.size();
    for (int i = album_control.list_id + 1; i < n; i++) {
      String src = make_json_path(i);
      String dst = make_json_path(i - 1);
      SD.rename(src, dst);
    }

    // Update album list
    album_control.list.erase(album_control.list.begin() + album_control.list_id);
    lv_dropdown_set_options (dropdown_list, make_json_list().c_str());
    lv_dropdown_set_selected(dropdown_list, album_control.list_id = 0);
    toggle_cell_state(TYPE_LEAF, LEAF_SELECTED);
    album_list_save();
    album_list_refresh();
  }
}

static void show_dialog_box(int id) {
  String name = "Are you sure you want to delete \"" + album_control.list[id].name + "\" ?\n";
  lv_obj_t *box = lv_msgbox_create(NULL);
  lv_msgbox_add_text      (box, name.c_str());
  lv_obj_set_style_pad_all(box, LV_PCT_Y( 2), (uint32_t)LV_PART_MAIN | (uint32_t)LV_STATE_DEFAULT);
  lv_obj_set_style_width  (box, LV_PCT_X(95), (uint32_t)LV_PART_MAIN | (uint32_t)LV_STATE_DEFAULT);

  lv_obj_t *btn = lv_msgbox_add_footer_button(box, "Yes");
  lv_obj_add_event_cb(btn, dialog_box_cb, LV_EVENT_CLICKED, (void*)true);

  btn = lv_msgbox_add_footer_button(box, "No");
  lv_obj_add_event_cb(btn, dialog_box_cb, LV_EVENT_CLICKED, (void*)false);
}

//--------------------------------------------------------------------------------
// Callback for dropdown list
//--------------------------------------------------------------------------------
static void dropdown_cb(lv_event_t *e) {
  DBG_ASSERT(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED);

  lv_obj_t * obj = lv_event_get_target_obj(e);
  album_control.list_id = lv_dropdown_get_selected(obj);

  // Just in case
  if (album_control.list_id >= album_control.list.size()) {
    album_control.list_id = 0;
  }

  // Select "All"
  if (album_control.list_id == 0) {
    toggle_cell_state(TYPE_LEAF, LEAF_SELECTED);
  }

  album_json_load();
  album_list_save();
  album_list_refresh();
}

//--------------------------------------------------------------------------------
// Drawing task callback for button matrix
// https://docs.lvgl.io/master/details/widgets/buttonmatrix.html#custom-buttons
//--------------------------------------------------------------------------------
static void draw_button_cb(lv_event_t *e) {
  lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
  lv_draw_task_type_t type  = lv_draw_task_get_type(draw_task);
  lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);

  // When the button matrix draws the buttons...
  if(base_dsc->part == LV_PART_ITEMS) {
    lv_obj_t *obj = lv_event_get_target_obj(e);
    bool pressed  = lv_obj_has_state(obj, LV_STATE_PRESSED);
    int selected  = lv_buttonmatrix_get_selected_button(obj);

    if (type == LV_DRAW_TASK_TYPE_LABEL) {
      lv_draw_label_dsc_t *label_dsc = lv_draw_task_get_label_dsc(draw_task);
      if (label_dsc) {
        if (obj == keypad_buttons) {
          bool disabled = lv_buttonmatrix_has_button_ctrl(obj, base_dsc->id1, LV_BUTTONMATRIX_CTRL_DISABLED);
          label_dsc->opa = (disabled ? 128 : 255);
        }
        else if (base_dsc->id1 == selected) {
          label_dsc->ofs_x = label_dsc->ofs_y = (pressed == true ? 1 : 0);
        }
      }
    }

    else if (type == LV_DRAW_TASK_TYPE_FILL) {
      lv_draw_fill_dsc_t *fill_dsc = lv_draw_task_get_fill_dsc(draw_task);
      if (fill_dsc) {
        fill_dsc->opa = (base_dsc->id1 == selected && pressed == true) ? 128 : 0;
      }
    }
  }
}

//--------------------------------------------------------------------------------
// Event callback for button matrix
//--------------------------------------------------------------------------------
static void toggle_event_cb(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  DBG_ASSERT(event_code == LV_EVENT_VALUE_CHANGED);

  AlbumInfo_t info = get_album_info();

  lv_obj_t *obj = lv_event_get_target_obj(e);
  uint32_t id = lv_buttonmatrix_get_selected_button(obj);
  switch (id) {
    case 0:
      toggle_cell_state(TYPE_NODE, info.n_folded == 0 ? NODE_FOLDED : NODE_UNFOLDED);
      break;
    case 1:
      toggle_cell_state(TYPE_LEAF, info.n_selected == 0 ? LEAF_SELECTED : LEAF_UNSELECTED);
      break;
    case LV_BUTTONMATRIX_BUTTON_NONE:
    default:
      DBG_ASSERT(false);
      break;
  }

  album_list_refresh(/*album_control.top*/);
}

//--------------------------------------------------------------------------------
// Keypad control event callback for handling textarea
//--------------------------------------------------------------------------------
static void keypad_button_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);

  if (code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
  }

  else if (code == LV_EVENT_CANCEL) {
    lv_obj_add_flag(keypad_panel, LV_OBJ_FLAG_HIDDEN);
    lv_indev_reset(NULL, ta); /* To forget the last clicked object to make it focusable again */
  }

  else if (code == LV_EVENT_READY) {
    lv_obj_add_flag(keypad_panel, LV_OBJ_FLAG_HIDDEN);
    lv_indev_reset(NULL, ta);

    String name = lv_textarea_get_text(ta);
    name.trim();

    if (name != "") {
      // Check if the "name" is already in the array
      int n = album_control.list.size();
      for (int i = 1; i < n; i++) {
        // Move upward in order
        if (album_control.list[i].name == name) {
          if (album_control.list_id == i) {
            album_json_save();
          } else {
            album_control.list_id = i;
            album_json_load();
          }
          album_list_save();
          album_list_refresh();
          lv_dropdown_set_selected(dropdown_list, album_control.list_id);
          return;
        }
      }

      if (album_control.list_id == 0) {
        // Creating JSON with all selected will result in high memory consumption
        // AlbumInfo_t info = get_album_info();
        // if (info.n_selected == album_control.root->get_n_leafs())
        toggle_cell_state(TYPE_LEAF, LEAF_UNSELECTED);

        // Create a new empty entry
        album_control.list_id = album_control.list.size();
        album_control.list.push_back({
          /* .name = */ name,
          /* .hash = */ 0
        });
      } else {
        // Replace the existing list
        album_control.list[album_control.list_id].name = name;
      }

      album_json_save();
      album_list_save();
      album_list_refresh();

      lv_dropdown_set_options (dropdown_list, make_json_list().c_str());
      lv_dropdown_set_selected(dropdown_list, album_control.list_id);
    }
  }
}

//--------------------------------------------------------------------------------
// Keypad button event callback for handling textarea
//--------------------------------------------------------------------------------
static void keypad_event_cb(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  DBG_ASSERT(event_code == LV_EVENT_VALUE_CHANGED);
  DBG_ASSERT(album_control.list_id < album_control.list.size());

  lv_obj_t *obj = lv_event_get_target_obj(e);
  lv_obj_t *ta  = lv_obj_get_child(keypad_panel, 1);
  uint32_t id   = lv_buttonmatrix_get_selected_button(obj);

  switch (id) {
    case 0: /* LV_SYMBOL_KEYBOARD */
      if (lv_obj_has_flag(keypad_panel, LV_OBJ_FLAG_HIDDEN)) {
        if (album_control.list_id == 0) {
          lv_textarea_set_text(ta, ""); // Create a new list
        } else {
          lv_textarea_set_text(ta, album_control.list[album_control.list_id].name.c_str()); // Edit an exsisting list
        }
        lv_obj_remove_flag(keypad_panel, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(keypad_panel, LV_OBJ_FLAG_HIDDEN);
      }
      break;
    case 1: /* LV_SYMBOL_SAVE */
      if (album_control.list_id > 0) {
        JsonDocument doc;
        JsonTree::tree2json(album_control.root, doc);

        std::hash<std::string> makeHash;
        size_t hash = makeHash(doc.as<std::string>()); // 1.2[msec]
        album_control.list[album_control.list_id].hash = hash;

        album_json_save();
        album_list_save();
        album_list_refresh();
      }
      break;
    case 2: /* LV_SYMBOL_TRASH */
      if (album_control.list_id > 0) {
        show_dialog_box(album_control.list_id);
      }
      break;
    case LV_BUTTONMATRIX_BUTTON_NONE:
    default:
      DBG_ASSERT(false);
      break;
  }
}

//--------------------------------------------------------------------------------
// Set the pointer to the widget to NULL when its object is deleted
//--------------------------------------------------------------------------------
static void delete_cb(lv_event_t *e) {
  lv_obj_t **obj = (lv_obj_t **)lv_event_get_user_data(e);
  static constexpr lv_obj_t ** const adrs[] = {
    &ui_ScreenAlbumList,
    &album_list,
    &album_info,
    &keypad_panel,
    &dropdown_list,
    &keypad_buttons,
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
// Initialize / Deinitialize widgets
//--------------------------------------------------------------------------------
void ui_ScreenAlbumList_screen_init(void) {
  if (ui_ScreenAlbumList == NULL) {
    ui_ScreenAlbumList = lv_obj_create(NULL);
    lv_obj_set_style_bg_color (ui_ScreenAlbumList, UI_COLOR_BACKGROUND, 0);
    lv_obj_add_event_cb       (ui_ScreenAlbumList, ui_event_ScreenAlbumList, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb       (ui_ScreenAlbumList, ui_event_ScreenAlbumList, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb       (ui_ScreenAlbumList, ui_event_ScreenAlbumList, LV_EVENT_SCREEN_UNLOADED, NULL);
  }

  ///////////////////// Title Label /////////////////////
  lv_obj_t *obj = lv_label_create(ui_ScreenAlbumList);
  lv_obj_set_pos(obj, TITLE_LABEL_X, TITLE_LABEL_Y);
  lv_label_set_text_static(obj, "Album List");

  //////////////////// Dropdown List ////////////////////
  if (dropdown_list == NULL) {
    static constexpr lv_style_const_prop_t style_prop_dropdown[] = {
      LV_STYLE_CONST_X(DROPDOWN_LIST_X),
      LV_STYLE_CONST_Y(DROPDOWN_LIST_Y),
      LV_STYLE_CONST_WIDTH(DROPDOWN_LIST_WIDTH),
      LV_STYLE_CONST_HEIGHT(LV_SIZE_CONTENT),
      LV_STYLE_CONST_TEXT_FONT(&CUSTOM_FONT_SMALL),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_dropdown, (void*)(style_prop_dropdown));

    dropdown_list = lv_dropdown_create(ui_ScreenAlbumList);
    lv_obj_add_style        (dropdown_list, &style_dropdown, 0);
    lv_dropdown_set_options (dropdown_list, make_json_list().c_str());
    lv_dropdown_set_selected(dropdown_list, album_control.list_id);
    lv_obj_add_event_cb     (dropdown_list, dropdown_cb, LV_EVENT_VALUE_CHANGED, NULL);
  }

  //////////////////// Button Matrix ////////////////////
  static constexpr lv_style_const_prop_t style_prop_button_main[] = {
    LV_STYLE_CONST_HEIGHT(30),
    LV_STYLE_CONST_BG_OPA(0),
    LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_PAD_TOP(0),
    LV_STYLE_CONST_PAD_LEFT(0),
    LV_STYLE_CONST_PAD_RIGHT(0),
    LV_STYLE_CONST_PAD_BOTTOM(0),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_button_item[] = {
    LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
    LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_SHADOW_WIDTH(0),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_toggle_main[] = {
    LV_STYLE_CONST_X(TOGGLE_BUTTON_X),
    LV_STYLE_CONST_Y(TOGGLE_BUTTON_Y),
    LV_STYLE_CONST_WIDTH(68), // 34 * 2
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_keypad_main[] = {
    LV_STYLE_CONST_X(KEYPAD_BUTTON_X),
    LV_STYLE_CONST_Y(KEYPAD_BUTTON_Y),
    LV_STYLE_CONST_WIDTH(102), // 34 * 3
    LV_STYLE_CONST_PROPS_END
  };
  static LV_STYLE_CONST_INIT(style_button_main, (void*)(style_prop_button_main));
  static LV_STYLE_CONST_INIT(style_button_item, (void*)(style_prop_button_item));
  static LV_STYLE_CONST_INIT(style_toggle_main, (void*)(style_prop_toggle_main));
  static LV_STYLE_CONST_INIT(style_keypad_main, (void*)(style_prop_keypad_main));
  static constexpr lv_buttonmatrix_ctrl_t button_ctrl = (lv_buttonmatrix_ctrl_t)(
    (uint32_t)LV_BUTTONMATRIX_CTRL_CLICK_TRIG |
    (uint32_t)LV_BUTTONMATRIX_CTRL_NO_REPEAT
  );
  static constexpr const char* toggle_map[] = { LV_SYMBOL_DIRECTORY, LV_SYMBOL_OK, NULL };
  static constexpr const char* keypad_map[] = { LV_SYMBOL_KEYBOARD, LV_SYMBOL_SAVE, LV_SYMBOL_TRASH, NULL };

  obj = lv_buttonmatrix_create(ui_ScreenAlbumList);
  lv_buttonmatrix_set_map             (obj, toggle_map );
  lv_buttonmatrix_set_button_ctrl_all (obj, button_ctrl);
  lv_obj_add_style                    (obj, &style_toggle_main, LV_PART_MAIN );
  lv_obj_add_style                    (obj, &style_button_main, LV_PART_MAIN );
  lv_obj_add_style                    (obj, &style_button_item, LV_PART_ITEMS);
  lv_obj_add_event_cb                 (obj, draw_button_cb,  LV_EVENT_DRAW_TASK_ADDED, NULL);
  lv_obj_add_event_cb                 (obj, toggle_event_cb, LV_EVENT_VALUE_CHANGED,   NULL);
  lv_obj_add_flag                     (obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

  if (keypad_buttons == NULL) {
    keypad_buttons = lv_buttonmatrix_create(ui_ScreenAlbumList);
    lv_buttonmatrix_set_map             (keypad_buttons, keypad_map);
    lv_buttonmatrix_set_button_ctrl_all (keypad_buttons, button_ctrl);
    lv_obj_add_style                    (keypad_buttons, &style_keypad_main, LV_PART_MAIN );
    lv_obj_add_style                    (keypad_buttons, &style_button_main, LV_PART_MAIN );
    lv_obj_add_style                    (keypad_buttons, &style_button_item, LV_PART_ITEMS);
    lv_obj_add_event_cb                 (keypad_buttons, draw_button_cb,  LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_event_cb                 (keypad_buttons, keypad_event_cb, LV_EVENT_VALUE_CHANGED,   NULL);
    lv_obj_add_flag                     (keypad_buttons, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
  }

  //////////////////// List Container ////////////////////
  if (album_list == NULL) {
    static constexpr lv_style_const_prop_t style_prop_album[] = {
      LV_STYLE_CONST_X(ALBUM_LIST_X),
      LV_STYLE_CONST_Y(ALBUM_LIST_Y),
      LV_STYLE_CONST_WIDTH(SCREEN_WIDTH - LV_PCT_X(10)),
      LV_STYLE_CONST_HEIGHT(ALBUM_LIST_HEIGHT),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_album, (void*)(style_prop_album));

    album_list = lv_list_create(ui_ScreenAlbumList);
    lv_obj_add_style          (album_list, &style_album, 0);
//  lv_obj_set_scrollbar_mode (album_list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb       (album_list, scroll_cb, LV_EVENT_SCROLL, NULL);
  }

  /////////////////// Infomation Label ///////////////////
  if (album_info == NULL) {
    static constexpr lv_style_const_prop_t style_prop_info[] = {
      LV_STYLE_CONST_Y(LV_PCT_Y(-2)+1),
      LV_STYLE_CONST_ALIGN(LV_ALIGN_BOTTOM_MID),
      LV_STYLE_CONST_TEXT_FONT(&CUSTOM_FONT_SMALL),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_info, (void*)(style_prop_info));

    album_info = lv_label_create(ui_ScreenAlbumList);
    lv_obj_add_style            (album_info, &style_info, LV_PART_MAIN);
    lv_label_set_text_static    (album_info, "No album");
    lv_obj_set_style_text_color (album_info, INFO_LABEL_COLOR, LV_PART_MAIN);
  }

  //////////////////// Keypad Panel ////////////////////
  if (keypad_panel == NULL) {
    // Base panel
    static constexpr lv_style_const_prop_t style_keypad_panel_prop[] = {
      LV_STYLE_CONST_ALIGN(LV_ALIGN_BOTTOM_MID),
      LV_STYLE_CONST_Y(-24),
      LV_STYLE_CONST_HEIGHT(LV_PCT_Y(70)-4),
      LV_STYLE_CONST_WIDTH(LV_PCT_X(100)),
      LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
      LV_STYLE_CONST_PAD_TOP(12),
      LV_STYLE_CONST_PAD_LEFT(0),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PAD_BOTTOM(0),
      LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_NONE),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_paenl, (void *)style_keypad_panel_prop);
    keypad_panel = lv_obj_create(ui_ScreenAlbumList);
    lv_obj_add_style          (keypad_panel, &style_paenl, (uint32_t)LV_PART_MAIN);
    lv_obj_set_scrollbar_mode (keypad_panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag           (keypad_panel, LV_OBJ_FLAG_HIDDEN);

    // Keypad
    static constexpr lv_style_const_prop_t style_keypad_prop[] = {
      LV_STYLE_CONST_HEIGHT(160),
      LV_STYLE_CONST_PAD_LEFT(0),
      LV_STYLE_CONST_PAD_RIGHT(0),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_keypad, (void *)style_keypad_prop);
    lv_obj_t *kb = lv_keyboard_create(keypad_panel);
    lv_obj_add_style(kb, &style_keypad, (uint32_t)LV_PART_MAIN);

    // Textarea
    static constexpr lv_style_const_prop_t style_textarea_prop[] = {
      LV_STYLE_CONST_ALIGN(LV_ALIGN_TOP_MID),
      LV_STYLE_CONST_WIDTH(LV_PCT_X(90)),
      LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(style_textarea, (void *)style_textarea_prop);
    lv_obj_t *ta = lv_textarea_create(keypad_panel);
    lv_obj_add_style                (ta, &style_textarea, (uint32_t)LV_PART_MAIN);
    lv_textarea_set_one_line        (ta, true);
    lv_textarea_set_max_length      (ta, 32);
    lv_textarea_set_placeholder_text(ta, "List name");
    lv_obj_add_event_cb             (ta, keypad_button_cb, LV_EVENT_ALL, (void *)kb);
    lv_obj_send_event               (ta, LV_EVENT_FOCUSED, NULL);
  }

  lv_obj_add_event_cb(ui_ScreenAlbumList, delete_cb, LV_EVENT_DELETE, reinterpret_cast<void*>(&ui_ScreenAlbumList));
  lv_obj_add_event_cb(dropdown_list,      delete_cb, LV_EVENT_DELETE, reinterpret_cast<void*>(&dropdown_list)     );
  lv_obj_add_event_cb(album_list,         delete_cb, LV_EVENT_DELETE, reinterpret_cast<void*>(&album_list)        );
  lv_obj_add_event_cb(album_info,         delete_cb, LV_EVENT_DELETE, reinterpret_cast<void*>(&album_info)        );
  lv_obj_add_event_cb(keypad_panel,       delete_cb, LV_EVENT_DELETE, reinterpret_cast<void*>(&keypad_panel)      );
  lv_obj_add_event_cb(keypad_buttons,     delete_cb, LV_EVENT_DELETE, reinterpret_cast<void*>(&keypad_buttons)    );

#if SHOW_ARROW_BUTTON || true
  //////////////// Move to Main/Setting ////////////////
  static constexpr lv_style_const_prop_t style_prop_U[] = {
    LV_STYLE_CONST_WIDTH(27),
    LV_STYLE_CONST_HEIGHT(27),
    LV_STYLE_CONST_X(MOVE_TO_UP_X),
    LV_STYLE_CONST_Y(MOVE_TO_UP_Y),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_R[] = {
    LV_STYLE_CONST_WIDTH(27),
    LV_STYLE_CONST_HEIGHT(27),
    LV_STYLE_CONST_X(MOVE_TO_RIGHT_X),
    LV_STYLE_CONST_Y(MOVE_TO_RIGHT_Y),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_default[] = {
    LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
    LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
    LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_PAD_TOP(8),
    LV_STYLE_CONST_PAD_RIGHT(0),
    LV_STYLE_CONST_PAD_BOTTOM(0),
    LV_STYLE_CONST_PAD_LEFT(8),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_default_U[] = {
    LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_up),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_default_R[] = {
    LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_right),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_checked_U[] = {
    LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_up),
    LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_checked_R[] = {
    LV_STYLE_CONST_BG_IMAGE_SRC(&img_menu_right),
    LV_STYLE_CONST_BG_COLOR(UI_COLOR_BACKGROUND),
    LV_STYLE_CONST_PROPS_END
  };
  static constexpr lv_style_const_prop_t style_prop_pressed[] = {
    LV_STYLE_CONST_PAD_TOP(10),
    LV_STYLE_CONST_PAD_LEFT(10),
    LV_STYLE_CONST_PROPS_END
  };
  static LV_STYLE_CONST_INIT(style_U,         (void*)style_prop_U        );
  static LV_STYLE_CONST_INIT(style_R,         (void*)style_prop_R        );
  static LV_STYLE_CONST_INIT(style_default,   (void*)style_prop_default  );
  static LV_STYLE_CONST_INIT(style_default_U, (void*)style_prop_default_U);
  static LV_STYLE_CONST_INIT(style_default_R, (void*)style_prop_default_R);
  static LV_STYLE_CONST_INIT(style_checked_U, (void*)style_prop_checked_U);
  static LV_STYLE_CONST_INIT(style_checked_R, (void*)style_prop_checked_R);
  static LV_STYLE_CONST_INIT(style_pressed,   (void*)style_prop_pressed  );

  obj = lv_checkbox_create(ui_ScreenAlbumList);
  lv_checkbox_set_text_static(obj, "");
  lv_obj_add_style    (obj, &style_U,         (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
  lv_obj_add_style    (obj, &style_default,   (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
  lv_obj_add_style    (obj, &style_default_U, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
  lv_obj_add_style    (obj, &style_checked_U, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  lv_obj_add_style    (obj, &style_pressed,   (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
  lv_obj_add_event_cb (obj, ui_event_ScreenAlbumList, LV_EVENT_CLICKED, (void*)true);

  obj = lv_checkbox_create(ui_ScreenAlbumList);
  lv_checkbox_set_text_static(obj, "");
  lv_obj_add_style    (obj, &style_R,         (uint32_t)LV_PART_MAIN      | (uint32_t)LV_STATE_DEFAULT);
  lv_obj_add_style    (obj, &style_default,   (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
  lv_obj_add_style    (obj, &style_default_R, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_DEFAULT);
  lv_obj_add_style    (obj, &style_checked_R, (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_CHECKED);
  lv_obj_add_style    (obj, &style_pressed,   (uint32_t)LV_PART_INDICATOR | (uint32_t)LV_STATE_PRESSED);
  lv_obj_add_event_cb (obj, ui_event_ScreenAlbumList, LV_EVENT_CLICKED, (void*)false);
#endif // SHOW_ARROW_BUTTON
}

void ui_ScreenAlbumList_screen_deinit(void) {
  if (ui_ScreenAlbumList) {
    // Re-traverse node tree before making a new playlist
    if (album_control.root) {
      album_control.root->traverse_node();
    }

    // Delete all the instances at delete_cb()
    lv_obj_delete_async(ui_ScreenAlbumList);
  }
}

//--------------------------------------------------------------------------------
// Create album list widgets
//--------------------------------------------------------------------------------
void ui_album_create(void *root, bool update) {
  if (root) {
    // Re-traverse node tree by preorder and initialize album list
    album_control.root = reinterpret_cast<Node*>(root);
    album_control.n_nodes = album_control.root->traverse_preorder();
    album_list_refresh(); // Initialize album list widgets

    // Load json data and update album list and dropdown
    if (update) {
      album_list_load(); // Load album list data
      lv_dropdown_set_options (dropdown_list, make_json_list().c_str());
      lv_dropdown_set_selected(dropdown_list, album_control.list_id);
    }
  } else {
    // In case the SD card is not inserted
    memset((void*)&album_control, 0, sizeof(album_control));
  }
}

//--------------------------------------------------------------------------------
// Load album list data in preparation for scanning audio files
//--------------------------------------------------------------------------------
void ui_album_load(void *root) {
  if (root) {
    // Once traverse node tree by preorder and load album list
    album_control.root = reinterpret_cast<Node*>(root);
    album_control.n_nodes = album_control.root->traverse_preorder();
    album_list_load(); // Load album list data

    // Then traverse again to scan audio files
    album_control.root->traverse_node();
  }
}

#if   DEBUG
//--------------------------------------------------------------------------------
// Debug functions (static)
//--------------------------------------------------------------------------------
static size_t count_exposed(Node *node) {
  size_t count = 0;
  for (auto &n : node->children) {
    ++count; // count self

    // when node is open and has children...
    size_t size = n->children.size();
    if (size && n->meta.checked == NODE_UNFOLDED) {
      if (n->children[0]->children.size()) {
        count += count_exposed(n);
      } else {
        count += size;
      }
    }
  }

  return count;
}

// Dump the contents of a list
typedef struct {
  const lv_obj_class_t *m_class;
  const char *m_name;
} ClassName_t;

static const char *check_class(lv_obj_t *obj) {
  // https://docs.lvgl.io/master/details/widgets/index.html
  static const ClassName_t list[] = {
    { &lv_bar_class,      "lv_bar"      },
    { &lv_label_class,    "lv_label"    },
    { &lv_button_class,   "lv_button"   },
    { &lv_checkbox_class, "lv_checkbox" },
    { &lv_image_class,    "lv_image"    },
    { &lv_list_class,     "lv_list"     },
    { &lv_obj_class,      "lv_obj"      },
  };

  for (int i = 0; i < sizeof(list) / sizeof(list[0]); i++) {
    if (lv_obj_has_class(obj, list[i].m_class)) {
      return list[i].m_name;
    }
  }

  return "unknown";
}

static void dump_album(lv_obj_t *obj, int depth) {
  for (int i = 0; i < depth; i++) { printf("  "); }

  Node *node = get_node(obj);
  if (node) {
    NodeMeta_t *meta = &node->meta;
    printf("key: %3d, type: %d, depth: %d, hidden: %d, checked: %d --> ",
      node->key, meta->type, meta->depth, meta->hidden, meta->checked
    );
  }

  const char *c = check_class(obj);
  if (strcmp(c, "lv_label"   ) == 0) { printf("%s \"%s\"\n", c, lv_label_get_text(obj));    } else
  if (strcmp(c, "lv_checkbox") == 0) { printf("%s \"%s\"\n", c, lv_checkbox_get_text(obj)); } else
  if (strcmp(c, "lv_dropdown") == 0) { printf("%s \"%s\"\n", c, lv_dropdown_get_text(obj)); } else
  if (strcmp(c, "lv_dropdown") == 0) { printf("%s \"%s\"\n", c, lv_dropdown_get_text(obj)); } else
  printf("%s\n", c);

  lv_obj_t *cell;
  for (int i = 0; cell = lv_obj_get_child(obj, i); i++) {
    dump_album(cell, depth + 1);
  }
}

//--------------------------------------------------------------------------------
// Debug functions (global)
//--------------------------------------------------------------------------------
size_t count_exposed_nodes(void) {
  return count_exposed(album_control.root); // number of exposed nodes in tree
}

size_t count_album_list(void) {
  return lv_obj_get_child_count(album_list); // number of cells in album list
}

void show_album_list(void) {
#if   true
  Node *top = album_control.root->find_preorder(album_control.top);
  Node *end = album_control.root->find_preorder(album_control.end);
  printf("top:%3d (pos: %d) \"%s\", end:%3d (pos: %d) \"%s\", count: %d/%d\n",
    album_control.top, lv_obj_get_scroll_top   (album_list), top->name.c_str(),
    album_control.end, lv_obj_get_scroll_bottom(album_list), end->name.c_str(),
    album_control.count, lv_obj_get_child_count(album_list)
  );
#else
  AlbumInfo_t info = get_album_info();
  printf("n_nodes: %d, n_leafs: %d, n_folded: %d, n_selected: %d, n_files: %d\n",
    album_control.n_nodes, album_control.root->get_n_leafs(), info.n_folded, info.n_selected, info.n_files
  );
#endif
}

void dump_album_list(void) {
  dump_album(album_list, 0); // dump all cells in album list
}

void dump_preorder(void) {
  album_control.root->dump_preorder(true); // dump all nodes in tree by preorder
}
#endif // Debug functions