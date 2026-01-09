//================================================================================
// MP3Player Configration
//================================================================================
#ifndef _CONFIG_H_
#define _CONFIG_H_

//--------------------------------------------------------------------------------
// 1. TFT Screen configuration
//--------------------------------------------------------------------------------
#define TFT_WIDTH       240   // Portrait orientation default width
#define TFT_HEIGHT      320   // Portrait orientation default height
#define TFT_ROTATION    0     // 0: Portrait, 3: Portrait (Upside down)

//--------------------------------------------------------------------------------
// 2. Graphic library configuration
// Below are for the LovyanGFX library for the "ESP32-2432S028R" (aka CYD).
// If the LCD panel auto-detection feature does not work, define the panel type.
//--------------------------------------------------------------------------------
// true  : Use the LGFX auto-detect feature
// false : Define the appropriate LCD panel driver type to "DISPLAY_CYD_2USB"
#define USE_AUTODETECT  true

#if (USE_AUTODETECT == false)
// false: Panel driver: ILI9341 (micro-USB x 1 type)
// true : Panel driver: ST7789  (micro-USB x 1 + USB-C x 1 type)
#define DISPLAY_CYD_2USB  true
#endif

//--------------------------------------------------------------------------------
// 3. Touch panel calibration
// If there's some slippage with the default parameters, execute calibration and
// embed the results displayed on the serial monitor into "MP3Player.ino".
//--------------------------------------------------------------------------------
// true  : Use embedded data in "MP3Player.ino"
// false : Execute touch calibration at power-on
#define USE_CALIBRATED    true

//--------------------------------------------------------------------------------
// 4. Custom fonts configuration
// Configure the font for displaying artists, albums, and songs.
// For information on how to generate font data files, see "fonts/README.md".
//--------------------------------------------------------------------------------
#define USE_CUSTOM_FONTS  true

//--------------------------------------------------------------------------------
// 5. Path to the folder where MP3 audio files are saved
//--------------------------------------------------------------------------------
#define MP3_ROOT_PATH     "/MP3/"                       // Root folder for storing music files
#define MP3_FILE_EXT      {".m4a", ".mp3", ".wav"}      // Define the preferred extension first
#define IS_VALID_FILE(f)  (*(f) != '@' && *(f) != '.')  // Folder/file name prefix (1 char) to exclude

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
// The list to classify albums is saved under "PARTITION_PATH" as a text file.
// Also the configuration for each classification is saved as a JSON file.
//--------------------------------------------------------------------------------
#define ALBUM_CONF_PATH   "@album/"       // Album list configuration folder
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
// │   │   ├── 02 title02.m4a     │   │   ├── 01 title01.m4a
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

#endif // _CONFIG_H_