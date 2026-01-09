//================================================================================
// LVGL file system interfaces for handling an image file on SD card with cache
// https://github.com/lvgl/lvgl/blob/master/src/libs/fsdrv/lv_fs_arduino_sd.cpp
// NOTE: uncomment the followings to use SdFat
//  "#define SDFATFS_USED" in CYD_Audio.h
//  "#define USE_UTF8_LONG_NAMES 1" in SdFatConfig.h
//================================================================================
#ifndef _SDFS_H_
#define _SDFS_H_

#include "CYD28_audio.h" // Defines the instance of SD or SdFat
#include "sdspi.h"

//--------------------------------------------------------------------------------
// Possible combinations
//
// | Image source / Symbol  | MY_USE_TJPGD | LV_USE_TJPGD | LV_USE_BMP |
// | ---------------------- | ------------ | ------------ | ---------- |
// | Jpg from SD with cache |       1      |       0      |      0     |
// | Bmp from SD with cache |       0      |       0      |      1     |
// | Binary from Flash      |       0      |       0      |      0     |
//
// Note: In either case, "LV_USE_FS_ARDUINO_SD" in lv_conf.h  must be set to 0.
//--------------------------------------------------------------------------------
#include <lvgl.h>

// JPEG decoder / SD file system with cache
#define MY_USE_TJPGD  1
#define MY_USE_FS_ARDUINO_SD  1
#define MY_FS_ARDUINO_SD_LETTER 'S'

void lv_tjpgd_init(void);
void lv_fs_clear_cache(void);
void lv_fs_arduino_sd_init(void);

#if (LV_USE_TJPGD != 0 || LV_USE_FS_ARDUINO_SD != 0)
  #error LV_USE_TJPGD and LV_USE_FS_ARDUINO_SD in lv_conf.h must be 0.
#endif

#endif // _SDFS_H_