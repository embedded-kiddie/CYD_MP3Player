//================================================================================
// LVGL file system interfaces for handling an image file on SD card with cache
// https://github.com/lvgl/lvgl/blob/master/src/libs/fsdrv/lv_fs_arduino_sd.cpp
// NOTE: uncomment the followings to use SdFat
//  "#define SDFATFS_USED" in CYD_Audio.h
//  "#define USE_UTF8_LONG_NAMES 1" in SdFatConfig.h
//================================================================================
#include <lvgl.h>
#include <string>
#include <functional>
#include "sdfs.h"
#include "debug.h"

#if MY_USE_FS_ARDUINO_SD

#if LV_MEM_ADR
  #define MY_MALLOC(size) lv_malloc(size)
  #define MY_FREE(addr)   lv_free(addr)
#elif (ESP_ARDUINO_VERSION_MAJOR >= 3)
  // https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32/api-reference/system/mem_alloc.html
  #define MY_MALLOC(size) heap_caps_malloc(size, MALLOC_CAP_DEFAULT)
  #define MY_FREE(addr)   heap_caps_free(addr)  // free() is equivalent to heap_caps_free() in IDF
#else
  #define MY_MALLOC(size) heap_caps_malloc(size, MALLOC_CAP_DEFAULT)
  #define MY_FREE(addr)   free(addr)
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t fs_read (lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t fs_seek (lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_tell (lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);

typedef struct {
  size_t    id;
  size_t    size;
  char *    buffer;
  uint32_t  position;
} FsCache_t;

static FsCache_t fs_cache = {};

void lv_fs_clear_cache(void) {
  fs_cache.id = 0;
  fs_cache.size = 0;
  fs_cache.position = 0;

  if (fs_cache.buffer) {
    MY_FREE(fs_cache.buffer);
    fs_cache.buffer = 0;
  }
}

/**
 * Register a driver for the SD File System interface
 */
void lv_fs_arduino_sd_init(void) {
  static lv_fs_drv_t fs_drv;
  lv_fs_drv_init(&fs_drv);

  fs_drv.letter   = MY_FS_ARDUINO_SD_LETTER;
  fs_drv.open_cb  = fs_open;
  fs_drv.close_cb = fs_close;
  fs_drv.read_cb  = fs_read;
  fs_drv.write_cb = NULL;
  fs_drv.seek_cb  = fs_seek;
  fs_drv.tell_cb  = fs_tell;

  fs_drv.dir_close_cb = NULL;
  fs_drv.dir_open_cb  = NULL;
  fs_drv.dir_read_cb  = NULL;

  lv_fs_drv_register(&fs_drv);

#if MY_USE_TJPGD
  lv_tjpgd_init();
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/**
 * Open a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode      read: SDFS_MODE_RD, write: SDFS_MODE_WR, both: SDFS_MODE_RD | SDFS_MODE_WR
 * @return          a file descriptor or NULL on error
 */
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
  LV_UNUSED(drv);
  LV_UNUSED(mode);

  std::hash<std::string> makeHash;
  size_t id = makeHash(path);
  if (id != fs_cache.id) {
    lv_fs_clear_cache();
    fs_cache.id = id;

    File file = SD.open(path, FILE_READ);
    const size_t size = file.SDFS_SIZE();
    fs_cache.buffer = (char *)MY_MALLOC(size);
    DBG_ASSERT(fs_cache.buffer);

    fs_cache.size = file.read((uint8_t *)fs_cache.buffer, size);
    DBG_ASSERT(fs_cache.size == size);

    file.close();
  }

  fs_cache.position = 0;
  return (void *)drv;
}

/**
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p) {
  LV_UNUSED(drv);
  LV_UNUSED(file_p);

  return LV_FS_RES_OK; // Keep the cache
}

/**
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br) {
  LV_UNUSED(drv);
  LV_UNUSED(file_p);

  if (0 <= fs_cache.position && fs_cache.position <= fs_cache.size) {
    /* Do not allow reading beyond the actual memory block (it can be happend with 'LV_USE_TJPGD' */
    uint32_t remaining = fs_cache.size - fs_cache.position;
    if (btr > remaining) {
      btr = remaining;
    }

    memcpy(buf, fs_cache.buffer + fs_cache.position, btr);
    fs_cache.position += (*br = btr);
    return LV_FS_RES_OK;
  }

  else {
    DBG_ASSERT(false);
    *br = 0;
    return LV_FS_RES_INV_PARAM;
  }
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence) {
  LV_UNUSED(drv);
  LV_UNUSED(file_p);

  switch (whence) {
    case LV_FS_SEEK_SET:
      fs_cache.position = pos;
      break;
    case LV_FS_SEEK_CUR:
      fs_cache.position += pos;
      break;
    case LV_FS_SEEK_END:
      fs_cache.position = (fs_cache.size - 1) - pos;
      break;
  }

  DBG_ASSERT(fs_cache.position < fs_cache.size);
  return LV_FS_RES_OK;
}

/**
 * Give the position of the read write pointer
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_p variable
 * @param pos_p     pointer to store the result
 * @return          LV_FS_RES_OK: no error or any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
  LV_UNUSED(drv);
  LV_UNUSED(file_p);

  *pos_p = fs_cache.position;
  return LV_FS_RES_OK;
}

#endif // MY_USE_FS_ARDUINO_SD