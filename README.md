# CYD MP3 Music Player
MP3 music player for Cheap Yellow Display

## Library & Sketches

### [CYD_Audio](CYD_Audio)
CYD_Audio is an [ESP32 I2S audio library][2] based on [schreibfaul1/ESP32-audioI2S][1] and customized by [Piotr Zapart][3] for the CYD with an internal DAC and onboard amplifier IC.

Originally written for PlatformIO, the version included here has been modified for the Arduino IDE and can be installed in the `libraries` folder within your Arduino sketchbook folder.

#### Note
This library strongly recommends using with **ESP32 core version 2.0.17**, since [the I2S driver in 3.x][9] has been [completely redesigned and refactored to use the new ESP-IDF driver][10], which deprecates some functions and causes issues such as clicking noises.

### [CYD_MP3Player_Basic](CYD_MP3Player_Basic)
This is a simple sketch that demonstrates how to use the CYD_Audio library. It plays a specified audio file stored on the SD card.

CYD28_audio.[ch] defines helper functions for creating an instance of the CYD_Audio class on Core 1, making it easy to create a GUI loop that runs on Core 0.

This example creates a task named `audioplay` on core 0 with a predefined set of commands, and allows core 1 to send messages to it to control playback, volume adjustment, etc.

### [CYD_MP3Player_Simple](CYD_MP3Player_Simple)
This sketch is an example of applying the `CYD_Audio` wrapper class named `MP3Player` to scan the audio files on the SD card, create a playlist, and control continuous playback in a set order.

### [CYD_MP3Player_LVGL](CYD_MP3Player_LVGL)
[![CYD MP3 Music Player](images/CYD-MP3-Music-Player.jpg)](https://youtu.be/sG8XH6JwQDA "")

This version features a rich LVGL GUI that allows you to play and manage audio files, add favorites, and shuffle playback.

The `MP3Player` class has been extended to meet the requirements of UI components.

## Hardware Specifications

### Audio Amplifier IC (SC8002B) Shutdown Signal
Depending on the CYD variant, the connection point for the audio amplifier IC's shutdown signal differs.

The GND-connected type activates the SC8002B upon startup, whereas the pull-up type requires software-based activation.

<details>
<summary>Guition type (GND-connected)</summary>

- Variants:  
    [ESP32-2432S028R][20], [JC2432W328][21], [CrowPanel ESP32 HMI Display][22]
- Schematics:  
    ![ILI9341 vs ST7789](images/SC8002B_2432S028R.jpg)
</details>

<details>
<summary>Elegoo / Freenove type (pull-up)</summary>

- Variants:  
    [ELEGOO E32R28T][23], [Freenove ESP32 Display][24]
- Schematics:  
![ILI9341 vs ST7789](images/SC8002B_ELEGOO_FREENOVE.jpg)
</details>

### Improvement of Sound Quality
Some CYD variants suffer from poor sound quality, so you need to replace some registers arround audio amplifier IC if you want to properly enjoy the music.

The following links are good resources to help you solve this problem.

- [Audio amp gain mod - ESP32-2432S028 aka Cheap Yellow Display example project][4].
- [ESP32-2432S028 aka Cheap Yellow Display - fixing the audio issues - YouTube][5]

#### Alternative modification for ESP32-2432S028R ST7789
By simply replacing two resistors, you can get almost the same output as the ILI9341 type with the following settings:

![ILI9341 vs ST7789](images/CYD-ILI9341-ST7789.jpg)

![The resistor settings around the audio amplifier](images/CYD-ST7789-modified.webp)

| Resister | Before modification | After modification |
| :------: | ------------------: | -----------------: |
| R7       | 0 Ω                 | 0 Ω                |
| R8       | 0 Ω                 | 22 KΩ              |
| R9       | 68 KΩ               | 15 KΩ              |

### Use External DAC and Amplifier
The links below explain how to connect external DAC modules.

- [Audio I2S mod - ESP32-2432S028 aka Cheap Yellow Display example project][6]

In this case, please define the symbol `USE_I2S_DAC` and each pin appropriately in [audioTask() in CYD28_audio.cpp](/CYD_MP3Player_LVGL/CYD28_audio.cpp#L35-L43).

```c++
void audioTask(void *parameter)
{
    // if using the I2S mod, RGB led is removed, I2S pinout defined in platformio.ini file
#ifdef USE_I2S_DAC
    audio.begin();
    audio.setPinout(I2S_BCK_PIN, I2S_LRCLK_PIN, I2S_DIN_PIN);
#else
    audio.begin(true, I2S_DAC_CHANNEL_LEFT_EN);
#endif
...
}
```

where:

| Symbol          | Value |
| --------------- | -----:|
| `I2S_BCK_PIN`   | 4     |
| `I2S_LRCLK_PIN` | 22    |
| `I2S_DIN_PIN`   | 27    |

Similar modifications can also be found at the following site:

- [CYD’s Note 2025 - macsbug][7]

## Download Link
- [GUITION-LINK](https://pan.jczn1688.com/ "UITION-DISK · Cloud Storage")
    - ESP32 module
    - HMI display

## Special Thanks
- [ESP32 I2S audio library][2] by [hexeguitar/ESP32_TFT_PIO][8] (MIT license).

[1]: https://github.com/schreibfaul1/ESP32-audioI2S "schreibfaul1/ESP32-audioI2S: Play mp3 files from SD via I2S"
[2]: https://github.com/hexeguitar/ESP32_TFT_PIO/tree/main/Examples/CYD28_BaseProject/lib/CYD_Audio "ESP32_TFT_PIO/Examples/CYD28_BaseProject/lib/CYD_Audio at main · hexeguitar/ESP32_TFT_PIO"
[3]: https://github.com/hexeguitar "hexeguitar (Piotr Zapart)"
[4]: https://github.com/hexeguitar/ESP32_TFT_PIO?tab=readme-ov-file#audio-amp-gain-mod "hexeguitar/ESP32_TFT_PIO: Example project for the ESP32-2432S028 &quot;Cheap Yellow Display&quot; board."
[5]: https://www.youtube.com/watch?v=6JCLHIXXVus "ESP32-2432S028 aka Cheap Yellow Display - fixing the audio issues - YouTube"
[6]: https://github.com/hexeguitar/ESP32_TFT_PIO?tab=readme-ov-file#audio-i2s-mod "hexeguitar/ESP32_TFT_PIO: Example project for the ESP32-2432S028 &quot;Cheap Yellow Display&quot; board."
[7]: https://macsbug.wordpress.com/2025/04/18/cyds-note-2025/ "CYD&#8217;s Note 2025 | macsbug"
[8]: https://github.com/hexeguitar/ESP32_TFT_PIO "hexeguitar/ESP32_TFT_PIO: Example project for the ESP32-2432S028 &quot;Cheap Yellow Display&quot; board."
[9]: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/i2s.html "I2S -  -  &mdash; Arduino ESP32 latest documentation"
[10]: https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html#i2s "Migration from 2.x to 3.0 -  -  &mdash; Arduino ESP32 latest documentation"

[20]: https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display "witnessmenow/ESP32-Cheap-Yellow-Display: Building a community around a cheap ESP32 Display with a touch screen"
[21]: https://github.com/maxpill/JC2432W328 "maxpill/JC2432W328: This repository contains documentation related to the JC2432W328 board equipped with the ST7789 display controller."
[22]: https://www.elecrow.com/wiki/esp32-display-282727-intelligent-touch-screen-wi-fi26ble-240320-hmi-display.html "CrowPanel ESP32 HMI 2.8-inch Display - Elecrow Wiki"
[23]: https://wiki.elegoo.com/oshw-parts-&-accessories/esp32-dispaly-introduction "ESP32 2.8 Inch dispaly Instruction | ELEGOO Wiki"
[24]: https://docs.freenove.com/projects/fnk0103/en/latest/ "FNK0103/FNK0114 &mdash; fnk0103-docs v1.0.0 documentation"