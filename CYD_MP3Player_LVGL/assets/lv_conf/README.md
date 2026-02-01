## Configuring LVGL

### 1. Installing LVGL
This application has been tested on LVGL versions 9.2.2, 9.3.0, and 9.4.0.

After installing LVGL, please refer to [Configure LVGL][1] and place lv_conf.h in the library folder.

### 2. Configuring lv_conf.h
Define the following "Symbol" as "Setting Value":

| Symbol                | Default value | Setting value |
| --------------------- | ------------- | ------------- |
| LV_MEM_SIZE           | (64 * 1024U)  | (40 * 1024U)  |
| LV_FONT_MONTSERRAT_12 | 0             | 1             |
| LV_USE_FS_ARDUINO_SD  | 0             | 0             |
| LV_USE_TJPGD          | 0             | 0             |
| LV_USE_TFT_ESPI       | 0             | 0             |
| LV_USE_LOVYAN_GFX     | 0             | 0             |
| LV_USE_ST7789         | 0             | 0             |
| LV_USE_ILI9341        | 0             | 0             |

In particular, `LV_USE_FS_ARDUINO_SD` and `LV_USE_TJPGD` must be defined to 0 to prevent conflicts with this application.

To improve compilation times, it is recommended that the following symbols are defined to 0:

| Symbol            | Setting value                |
| ----------------- | ---------------------------- |
| LV_BUILD_EXAMPLES | 0                            |
| LV_BUILD_DEMOS    | 0 (for version 9.3.0, 9.4.0) |
| LV_USE_DEMO_XXXX  | 0 (only for version 9.2.2)   |

[1]: https://docs.lvgl.io/master/integration/frameworks/arduino.html#configure-lvgl "Arduino - LVGL  documentation"
