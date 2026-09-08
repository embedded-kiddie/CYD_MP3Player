//================================================================================
// MP3Player Configration
//  Auther: embedded-kiddie (https://github.com/embedded-kiddie)
//================================================================================
#pragma once

//--------------------------------------------------------------------------------
// 1. LCD Screen definition
//--------------------------------------------------------------------------------
#define TFT_WIDTH       240   // Portrait orientation default width
#define TFT_HEIGHT      320   // Portrait orientation default height
#define TFT_ROTATION    0     // 0: Portrait, 2: Portrait (Upside down)

//--------------------------------------------------------------------------------
// 2.1 Define the specific CYD variant type
// Select one of the following definitions and uncomment it.
//--------------------------------------------------------------------------------
  #define CYD_2432S028R_1USB    // Guition 2432S028R ILI9341 (micro-USB x 1 type)
//#define CYD_2432S028R_2USB    // Guition 2432S028R ST7789  (micro-USB x 1 + USB-C x 1 type)
//#define CYD_JC2432W328_ST7789 // Guition JC2432W328 ST7789 (USB-C x 1 type)
//#define CYD_FREENOVE_ST7789   // Freenove FNK0114B_2P8 ST7789
//#define CYD_FREENOVE_ILI9341  // Freenove FNK0114F_2P8 ILI9341
//#define CYD_ELECROW_ILI9341   // Elecrow CrowPanel ESP32 HMI ILI9341V
//#define CYD_ELEGOO_ILI9341    // ELEGOO E32R28T ILI9341V

//--------------------------------------------------------------------------------
// 2.2 Select Graphic library
// Select one of the graphic libraries below and uncomment it.
// Note: TFT_eSPI is available only for "CYD_ELECROW_ILI9341".
//--------------------------------------------------------------------------------
//#define USE_TFT_ESPI    // TFT_eSPI (Slower than LovyanGFX)
  #define USE_LOVYANGFX   // LovyanGFX (Highly recommended)

// Configure LovyanGFX auto-detection of display driver type only for original CYD.
// If auto-detection does not work on some variants, set USE_AUTODETECT to false.
// true  : Use the LGFX auto-detection feature (which increase available memory).
// false : Define the appropriate CYD variant type in section 2.1.
#define USE_AUTODETECT    false // "CYD_2432S028R_1USB" or "CYD_2432S028R_2USB" only

//--------------------------------------------------------------------------------
// 3. Touch panel calibration
// If there's some slippage with the default parameters, execute calibration and
// embed the results shown on the serial monitor at "tft_init()" in "gfx.hpp".
//--------------------------------------------------------------------------------
// false : Use embedded data at "tft_init()" in "gfx.hpp"
// true  : Execute touch calibration at power-on
#define EXEC_CALIBRATION  false

//--------------------------------------------------------------------------------
// 4. Custom fonts configuration
// Configure the font for displaying artists, albums, and songs.
// Refer to "fonts/README.md" for information on how to generate font data files.
//--------------------------------------------------------------------------------
#define USE_CUSTOM_FONTS  true

//--------------------------------------------------------------------------------
// 5. Path to the folder where MP3 audio files are saved
//--------------------------------------------------------------------------------
#define MP3_ROOT_PATH     "/MP3/"                       // Root folder for storing music files
#define MP3_FILE_EXT      {".m4a", ".mp3", ".wav"}      // Define the preferred extension first
#define MP3_IS_VALID(f)   (*(f) != '@' && *(f) != '.')  // Folder/File name prefix (1 char) to exclude

//--------------------------------------------------------------------------------
// 6. File that stores settings related to the operation of this player
//--------------------------------------------------------------------------------
#define MP3_SETTING_FILE  "@setting.dat"  // Save `UI_Setting_t` structure as binary

//--------------------------------------------------------------------------------
// 7. Partitions under "MP3_ROOT_PATH"
// Limit the # of audio files in your playlist to a maximum of around 600.
// You can make full use of your SD card capacity by creating subfolders
// (called partitions) under "MP3_ROOT_PATH". (e.g. "/MP3/1/", "/MP3/2/", ...)
//--------------------------------------------------------------------------------
#define PARTITION_MAX     5               // Maximum # of partitions (must be 5 or less)
#define PARTITION_PATH    "%d/"           // Partition folder name ("%d" : 1, 2, ... 5)

//--------------------------------------------------------------------------------
// 8. Album list configuration under "PARTITION_PATH"
// An album list is saved under "PARTITION_PATH" as a text file.
// Also the configuration for each classification is saved as a JSON file.
//--------------------------------------------------------------------------------
#define ALBUM_LIST_PATH   "@album/"       // Album list configuration folder
#define ALBUM_LIST_FILE   "@list.txt"     // Album list configuration file
#define ALBUM_LIST_JSON   ".json"         // Album list JSON file extension

// Album cover photo files (e.g. "@photo.jpg") may preferably be 96x96,
// compress the jpeg file by 50-75%, and keep the file size to 6KB or less.
#define ALBUM_META_FILE   "@meta.dat"     // Album meta information file (binary data)
#define ALBUM_PHOTO_FILE  "@photo"        // Album cover photo file name body
#define ALBUM_PHOTO_EXT   ".jpg"          // Album cover photo file name extension

//================================================================================
// Here're examples of folders/files structure on an SD card.
//
// [WITHOUT PARTITION]            [WIDTH PARTITION]
// /MP3/                          /MP3/
// ├── @setting.dat               ├── @setting.dat
// ├── @album/                    ├── 1/
// │   ├── @list.txt              │   ├── @album/
// │   ├── 1.json                 │   │   ├── @list.txt
// │   ├── 2.json                 │   │   ├── 1.json
// │   └── ...                    │   │   ├── 2.json
// ├── Artist1/                   │   │   └── ...
// │   ├── Arbum1.1/              │   ├── Artist1/
// │   │   ├── @meta.dat          │   │   ├── Arbum1.1/
// │   │   ├── @photo.jpg         │   │   │   ├── @meta.dat
// │   │   ├── 01 title01.m4a     │   │   │   ├── @photo.jpg
// │   │   ├── 02 title02.m4a     │   │   │   ├── 01 title01.m4a
// │   │   └── ...                │   │   │   ├── 02 title02.m4a
// │   ├── Arbum1.2/              │   │   │   └── ...
// │   │   └── ...                │   │   ├── Arbum1.2/
// │   └── ...                    │   │   │   └── ...
// ├── Artist2/                   │   │   └── ...
// │   ├── Arbum2.1               │   ├── Artist2/
// │   └──...                     │   │   ├── Arbum2.1/
// └── ...                        │   │   └── ...
//                                │   └── ...
//                                ├── 2/
//                                │   ├── @album/
//                                │   │   ├── ...
//================================================================================