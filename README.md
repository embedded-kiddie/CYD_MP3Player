# CYD_MP3Player
MP3 player with internal DAC for Cheap Yellow Display

## CYD_Audio
This library is derived from [hexeguitar/ESP32_TFT_PIO][1], which was created for PlatformIO.

The original can play internet radio (PSRAM required) as well as audio files stored on the SD card.

This derivative has been modified for Arduino so that it can be installed in the `libraries` folder in your Arduino sketchbook folder.

## CYD_MP3Player_Basic
This is a simple sketch that demonstrates how to use the CYD_Audio library. It plays a specified audio file stored on the SD card.

It creates a task named `audioplay` on ESP32 core 0 and sends commands to control play, stop, volume, etc.

## CYD_MP3Player_Simple
This sketch is an example of applying a wrapper class that scans for audio files on an SD card, creates a playlist, and controls playback.

## CYD_MP3Player_LVGL
This version features a rich LVGL GUI that allows you to play and manage audio files, add favorites, and shuffle playback.

# Special Thanks
- [hexeguitar/ESP32_TFT_PIO][1] (published under the MIT license)

[1]: https://github.com/hexeguitar/ESP32_TFT_PIO "hexeguitar/ESP32_TFT_PIO: Example project for the ESP32-2432S028 &quot;Cheap Yellow Display&quot; board."
