## Creating custom fonts

### 1. Prepare the TTF/OFF base font
- [Google Fonts][1]

### 2. Open LVGL Font Converter
 - [Font Converter][2]

### 3. Set up Font Converter
Create font data for 12px and 14px fonts. Below is an example of font converter settings when the base font is [Noto Sans Japanese][3].

#### Name
  - noto_sans_jp_4bit_jis1_12
  - noto_sans_jp_4bit_jis1_14

#### Size
  - 12
  - 14

#### Bpp
  - 4 bit-per-pixel

#### Fallback
Specifies a built-in LVGL font.

  - lv_font_montserrat_12
  - lv_font_montserrat_14

#### Output format
  - C file

#### TTF/WOFF font
  - [NotoSansJP-Regular.ttf][3]

#### Range
Open the base font in a font editor such as [FontForge][4] and specify the required glyphs in Unicode.

- `0x00A1-0x27A1,0x3001-0x30FF,0xFF01-0xFF9F`

#### Symbols (additional)
Specifies characters other than those specified in **Range**.

  - Level 1 Kanji (e.g. `亜唖娃阿哀愛...`)
  - Level 2 Kanji (e.g. `栞騙彗翳蜃...`)

### 4. Download the font's C source code
Click the **`Submit`** button to download the font data.

### 5. Optimize font height
Optimize the font height and baseline to fit the layout of this application.

Open the downloaded C source code. Change both `.line_height` and `.base_line` as shown below, and comment out the `.static_bitmap` line.

#### • 12px font file
```
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
...
//  .static_bitmap = 0,
```

#### • 14px font file
```
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
...
//  .static_bitmap = 0,
```

### 6. Prepare to compile
Place the modified files under the `src` folder.

```
CYD_MP3Player/src/
├── CYD_Audio/
├── _pictures/
├── _pictures.h
├── noto_sans_jp_4bit_jis1_12.c <== Here!
├── noto_sans_jp_4bit_jis1_14.c <== Here!
├── tjpgd/
└── ui_img.c
```

[1]: https://fonts.google.com/ "Browse Fonts - Google Fonts"
[2]: https://lvgl.io/tools/fontconverter "Font Converter - LVGL"
[3]: https://fonts.google.com/noto/specimen/Noto+Sans+JP "Noto Sans Japanese - Google Fonts"
[4]: https://fontforge.org/ "FontForge Open Source Font Editor"
