#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "pinout_esp32.h"

class LGFX : public lgfx::LGFX_Device
{
#if defined(ILI9341_DRIVER)
  lgfx::Panel_ILI9341 _panel_instance;
#else
  lgfx::Panel_ST7789  _panel_instance;
#endif
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  // Create a constructor and set various settings here.
  LGFX(void)
  {
    { // Sets the bus control.
      auto cfg = _bus_instance.config();  // Get the bus configuration structure.

      // SPI bus settings
      cfg.spi_host = HSPI_HOST;   // Select the SPI (ESP32-S2,C3: SPI2_HOST or SPI3_HOST / ESP32: VSPI_HOST or HSPI_HOST)
      // Due to the ESP-IDF version upgrade, the VSPI_HOST and HSPI_HOST are deprecated, so if an error occurs, use SPI2_HOST or SPI3_HOST instead.
      cfg.spi_mode = 0;               // SPI communication mode (0 to 3)
      cfg.freq_write = SPI_FREQUENCY; // SPI clock for transmit (Maximum 80MHz, rounded to an integer value of 80MHz)
      cfg.freq_read  = 16000000;      // SPI clock for receive
      cfg.spi_3wire  = false;         // Set to true if receive on the MOSI pin
      cfg.use_lock   = true;          // Set to true if transaction lock is used
      cfg.dma_channel = SPI_DMA_CH_AUTO; // Set the DMA channel (0=DMA not used / 1=1ch / 2=2ch / SPI_DMA_CH_AUTO=automatic)
      // Due to the ESP-IDF version upgrade, SPI_DMA_CH_AUTO is recommended. Specifying 1ch or 2ch is no longer recommended.
      cfg.pin_sclk = TFT_SCLK;        // Set the SPI SCLK pin
      cfg.pin_mosi = TFT_MOSI;        // Set the SPI MOSI pin
      cfg.pin_miso = TFT_MISO;        // Set the SPI MISO pin (-1 = disable)
      cfg.pin_dc   = TFT_DC;          // Set the SPI D/C  pin (-1 = disable)
      // When sharing the SPI bus with an SD card, do not omit the MISO pin.

      _bus_instance.config(cfg);      // Configure setting values in the bus.
      _panel_instance.setBus(&_bus_instance); // Set the bus on the panel.
    }

    { // Configure the display panel control settings.
      auto cfg = _panel_instance.config();// Get the structure for display panel settings.

      cfg.pin_cs           = TFT_CS;  // CS   pin number (-1 = disable)
      cfg.pin_rst          = TFT_RST; // RST  pin number (-1 = disable)
      cfg.pin_busy         = -1;      // BUSY pin number (-1 = disable)

      // The following are set to general values, so try commenting out if you are unsure of.
      cfg.panel_width      =   240;  // Panel width
      cfg.panel_height     =   320;  // Panel height
      cfg.offset_x         =     0;  // Panel offset in X direction
      cfg.offset_y         =     0;  // Panel offset in Y direction
#if defined(ILI9341_DRIVER)
      cfg.offset_rotation  =     2;  // Rotation direction offset 0~7 (4~7 are upside down)
      cfg.dummy_read_pixel =     8;  // Number of dummy read bits before pixel read
#else
      cfg.offset_rotation  =     0;  // Rotation direction offset 0~7 (4~7 are upside down)
      cfg.dummy_read_pixel =    16;  // Number of dummy read bits before pixel read
#endif
      cfg.dummy_read_bits  =     1;  // Number of dummy read bits before reading non-pixel data
      cfg.readable         = true;   // Set to true if data can be read
      cfg.invert           = false;  // Set to true if the panel is inverted
      cfg.rgb_order        = false;  // Set to true if the red and blue of the panel are swapped
      cfg.dlen_16bit       = false;  // Set to true if the panel transmit data in 16-bit via 16-bit parallel or SPI
      cfg.bus_shared       = false;  // Set to true if the bus is shared with the SD card (The bus is controlled for drawJpg etc.)

      // Set the following only if your display is misaligned, such as ST7735 or ILI9163, which have variable pixel counts.
      // cfg.memory_width     =   240;  // Maximum width  supported by driver IC
      // cfg.memory_height    =   320;  // Maximum height supported by driver IC

      _panel_instance.config(cfg);
    }

    { // Set the backlight control. (Delete if not required)
      auto cfg = _light_instance.config();  // Get the backlight setting structure

      cfg.pin_bl      = TFT_BL; // Backlight pin number
      cfg.invert      = false;  // Set to true if the backlight brightness is inverted
      cfg.freq        = 12000;  // Backlight PWM frequency
      cfg.pwm_channel = 7;      // The PWM channel number

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // Set the backlight on the panel.
    }

    { // Configure touch screen control (delete if not needed)
      auto cfg = _touch_instance.config();

      cfg.x_min       =  240;   // Minimum X value (raw value) from touch screen
      cfg.x_max       = 3800;   // Maximum X value (raw value) from touch screen
      cfg.y_min       = 3700;   // Minimum Y value (raw value) from touch screen
      cfg.y_max       =  200;   // Maximum Y value (raw value) from touch screen
      cfg.pin_int     = TP_IRQ; // Interrupt pin number

#if defined(CYD_ELECROW_ILI9341) || defined(CYD_JC2432W328_ST7789)
      cfg.bus_shared  = true;   // Set to true if the bus shared with the screen
#else
      cfg.bus_shared  = false;  // Set to true if the bus shared with the screen
#endif
#if defined(CYD_2432S028R_2USB)
      cfg.offset_rotation = 2;  // Adjust when display and touch orientation do not match (0~7)
#else
      cfg.offset_rotation = 0;  // Adjust when display and touch orientation do not match (0~7)
#endif

      // For SPI connection
#if defined(CYD_ELECROW_ILI9341) || defined(CYD_JC2432W328_ST7789) 
      cfg.spi_host = HSPI_HOST; // Select the SPI (HSPI_HOST or VSPI_HOST)
#else
      cfg.spi_host  = -1;       // It can be set to -1 for Bit Banging
#endif
      cfg.freq      = 1000000;  // Set the SPI clock
      cfg.pin_sclk  = TP_SCLK;  // SCLK pin number
      cfg.pin_mosi  = TP_MOSI;  // MOSI pin number
      cfg.pin_miso  = TP_MISO;  // MISO pin number
      cfg.pin_cs    = TP_CS;    // CS   pin number

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // Set the touch screen on the panel.
    }

    setPanel(&_panel_instance); // Set the panel to be used.
  }
};