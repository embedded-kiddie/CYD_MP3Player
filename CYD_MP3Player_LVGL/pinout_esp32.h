//===================================================================================
// GPIO pin and LCD driver / SPI constant definitions for ESP32-2432S028R variants
//===================================================================================
#pragma once

#include "config.h"

#if defined(CYD_JC2432W328_ST7789) || defined(CYD_ELECROW_ILI9341)
  #define TFT_MISO  12
  #define TFT_MOSI  13
  #define TFT_SCLK  14
  #define TFT_CS    15
  #define TFT_DC    2
  #define TFT_RST   -1
  #define TFT_BL    27

  #define TP_MISO   TFT_MISO
  #define TP_MOSI   TFT_MOSI
  #define TP_SCLK   TFT_SCLK
  #define TP_CS     33
  #define TP_IRQ    36
#endif

#if defined(CYD_2432S028R_1USB) || \
    defined(CYD_2432S028R_2USB) || \
    defined(CYD_ELEGOO_ILI9341) || \
    defined(CYD_FREENOVE_ILI9341) || \
    defined(CYD_FREENOVE_ST7789)
  #define TFT_MISO  12
  #define TFT_MOSI  13
  #define TFT_SCLK  14
  #define TFT_CS    15
  #define TFT_DC    2
  #define TFT_RST   -1
  #define TFT_BL    21

  #define TP_MISO   39
  #define TP_MOSI   32
  #define TP_SCLK   25
  #define TP_CS     33
  #define TP_IRQ    36
#endif

#if defined(CYD_2432S028R_1USB)   || \
    defined(CYD_ELEGOO_ILI9341)   || \
    defined(CYD_ELECROW_ILI9341)  || \
    defined(CYD_FREENOVE_ILI9341)
  #define ILI9341_DRIVER
#endif

#if defined(CYD_2432S028R_2USB)    || \
    defined(CYD_JC2432W328_ST7789) || \
    defined(CYD_FREENOVE_ST7789)
  #define ST7789_DRIVER
#endif

#if defined(ILI9341_DRIVER)
  #define SPI_FREQUENCY 40000000
#else
  #define SPI_FREQUENCY 80000000
#endif