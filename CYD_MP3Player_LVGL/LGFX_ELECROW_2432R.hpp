#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

//===================================================================================
// CrowPanel ESP32 HMI 2.8-inch
// https://github.com/Elecrow-RD/CrowPanel-ESP32-Display-Course-File
// https://www.elecrow.com/wiki/esp32-display-282727-intelligent-touch-screen-wi-fi26ble-240320-hmi-display.html
//===================================================================================
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
#define TP_IRQ    38

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:

  LGFX(void)
  {
    { // バス制御の設定を行います。
      auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。

      // SPIバスの設定
      cfg.spi_host = HSPI_HOST;     // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  = 16000000;    // 受信時のSPIクロック
      cfg.spi_3wire  = false;       // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock   = true;        // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = TFT_SCLK;      // SPIのSCLKピン番号を設定
      cfg.pin_mosi = TFT_MOSI;      // SPIのMOSIピン番号を設定
      cfg.pin_miso = TFT_MISO;      // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc   = TFT_DC;        // SPIのD/Cピン番号を設定  (-1 = disable)
      // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs           = TFT_CS; // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst          = TFT_RST;// RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy         =    -1;  // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。
      cfg.panel_width      =   240;  // 実際に表示可能な幅
      cfg.panel_height     =   320;  // 実際に表示可能な高さ
      cfg.offset_x         =     0;  // パネルのX方向オフセット量
      cfg.offset_y         =     0;  // パネルのY方向オフセット量
      cfg.offset_rotation  =     2;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable         =  true;  // データ読出しが可能な場合 trueに設定
      cfg.invert           = false;  // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit       = false;  // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared       =  true;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      _panel_instance.config(cfg);
    }

    { // バックライト制御の設定を行います。（必要なければ削除）
      auto cfg = _light_instance.config();    // バックライト設定用の構造体を取得します。

      cfg.pin_bl = TFT_BL;          // バックライトが接続されているピン番号
      cfg.invert = false;           // バックライトの輝度を反転させる場合 true
      cfg.freq   = 44100;           // バックライトのPWM周波数
      cfg.pwm_channel = 7;          // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // バックライトをパネルにセットします。
    }

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      cfg.x_min      = 240;     // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max      = 3800;    // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min      = 3700;    // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max      = 200;     // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int    = TP_IRQ;  // INTが接続されているピン番号
      cfg.bus_shared = true;    // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0;  // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

      // SPI接続の場合
      cfg.spi_host = HSPI_HOST; // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
      cfg.freq = 1000000;       // SPIクロックを設定
      cfg.pin_sclk = TP_SCLK;   // SCLKが接続されているピン番号
      cfg.pin_mosi = TP_MOSI;   // MOSIが接続されているピン番号
      cfg.pin_miso = TP_MISO;   // MISOが接続されているピン番号
      cfg.pin_cs   = TP_CS;     //   CSが接続されているピン番号

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};