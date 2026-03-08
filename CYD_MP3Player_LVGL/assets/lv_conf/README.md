## Configuring LVGL

### 1. Installing LVGL
This application has been tested on LVGL versions 9.2.2, 9.3.0, 9.4.0 and 9.5.0.

After installing LVGL, please refer to [Configure LVGL][1] and place lv_conf.h in the library folder.

### 2. Configuring lv_conf.h

#### Required Setting
Define the following "Symbol" as "Setting Value":

| Symbol                    | Default value | Setting value    |
| ------------------------- | ------------- | ---------------- |
| **LV_MEM_SIZE**           | (64 * 1024U)  | **(40 * 1024U)** |
| **LV_FONT_MONTSERRAT_12** | 0             | **1**            |
| LV_USE_FS_ARDUINO_SD      | 0             | 0                |
| LV_USE_TJPGD              | 0             | 0                |
| LV_USE_TFT_ESPI           | 0             | 0                |
| LV_USE_LOVYAN_GFX         | 0             | 0                |
| LV_USE_ST7789             | 0             | 0                |
| LV_USE_ILI9341            | 0             | 0                |

In particular, `LV_USE_FS_ARDUINO_SD` and `LV_USE_TJPGD` must be defined to 0 to prevent conflicts with this application.

#### Recommended Setting
To improve compilation times, it is recommended that the following symbols are defined to 0:

| BUILD OPTIONS         | Setting value                       |
| --------------------- | ----------------------------------- |
| **LV_BUILD_EXAMPLES** | **0**                               |
| **LV_BUILD_DEMOS**    | **0** (for version 9.3.0 and later) |
| **LV_USE_DEMO_XXXX**  | **0** (only for version 9.2.2)      |

#### Used/Unused Widgets
The following symbols indicate the widgets used in this application.

Setting unused widget symbols to `0` will reduce LVGL heap memory consumption as defined by `LV_MEM_SIZE`, but is not required.

| WIDGETS                          | Default value | Setting value |
| -------------------------------- | ------------- | ------------- |
| **LV_WIDGETS_HAS_DEFAULT_VALUE** | 1             | **0**         |
| **LV_USE_ANIMIMG**               | 1             | **0**         |
| **LV_USE_ARC**                   | 1             | **0**         |
| **LV_USE_ARCLABEL**              | 1             | **0**         |
| LV_USE_BAR                       | 1             | 1             |
| LV_USE_BUTTON                    | 1             | 1             |
| LV_USE_BUTTONMATRIX              | 1             | 1             |
| **LV_USE_CALENDAR**              | 1             | **0**         |
| **LV_USE_CANVAS**                | 1             | **0**         |
| **LV_USE_CHART**                 | 1             | **0**         |
| LV_USE_CHECKBOX                  | 1             | 1             |
| LV_USE_DROPDOWN                  | 1             | 1             |
| LV_USE_IMAGE                     | 1             | 1             |
| **LV_USE_IMAGEBUTTON**           | 1             | **0**         |
| LV_USE_KEYBOARD                  | 1             | 1             |
| LV_USE_LABEL                     | 1             | 1             |
| **LV_USE_LED**                   | 1             | **0**         |
| LV_USE_LINE                      | 1             | 1             |
| LV_USE_LIST                      | 1             | 1             |
| LV_USE_LOTTIE                    | 0             | 0             |
| **LV_USE_MENU**                  | 1             | **0**         |
| LV_USE_MSGBOX                    | 1             | 1             |
| LV_USE_ROLLER                    | 1             | 1             |
| **LV_USE_SCALE**                 | 1             | **0**         |
| LV_USE_SLIDER                    | 1             | 1             |
| **LV_USE_SPAN**                  | 1             | **0**         |
| **LV_USE_SPINBOX**               | 1             | **0**         |
| **LV_USE_SPINNER**               | 1             | **0**         |
| **LV_USE_SWITCH**                | 1             | **0**         |
| **LV_USE_TABLE**                 | 1             | **0**         |
| **LV_USE_TABVIEW**               | 1             | **0**         |
| LV_USE_TEXTAREA                  | 1             | 1             |
| **LV_USE_TILEVIEW**              | 1             | **0**         |
| **LV_USE_WIN**                   | 1             | **0**         |
| LV_USE_3DTEXTURE                 | 0             | 0             |

[1]: https://docs.lvgl.io/master/integration/frameworks/arduino.html#configure-lvgl "Arduino - LVGL  documentation"
