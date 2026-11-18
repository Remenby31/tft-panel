// LVGL 9.6 configuration for TFT Panel. Only non-default values are set here;
// lv_conf_internal.h supplies the rest.
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16

// All LVGL allocations in PSRAM (src/board/lv_mem_psram.c): with the C allocator,
// blocks under 4 KB stay in internal RAM and the glyph caches starved the Wi-Fi stack.
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CUSTOM
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

#define LV_USE_OS LV_OS_NONE
#define LV_DEF_REFR_PERIOD 16

#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1

#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

// FPS / CPU overlay: off (it covers the track time). Set both to 1 for performance work;
// last reading with lyrics scrolling: 49 FPS, 22 % CPU.
#define LV_USE_SYSMON 0
#define LV_USE_PERF_MONITOR 0

// Fonts. All text is the embedded Noto Sans through tiny_ttf (src/app/fonts.cpp). LVGL's
// Montserrat stays for its symbols only (play, volume, settings...): src/app/symbol_fonts/ keeps
// the ones tools/fonts/symbols.txt lists, byte for byte, under the same names, and LVGL's own
// files (letters included, ~60 KB more on the CrowPanel) are off.
#define LV_FONT_MONTSERRAT_14 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_48 0
#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(lv_font_montserrat_14) LV_FONT_DECLARE(lv_font_montserrat_18) \
    LV_FONT_DECLARE(lv_font_montserrat_20) LV_FONT_DECLARE(lv_font_montserrat_24)                        \
    LV_FONT_DECLARE(lv_font_montserrat_28) LV_FONT_DECLARE(lv_font_montserrat_36)
// Symbols only: what is drawn before the theme gets Noto Sans (src/app/ui.cpp) shows no letters.
#define LV_FONT_DEFAULT &lv_font_montserrat_20

// Run-time TTF rendering for the embedded Noto Sans (src/app/fonts.cpp). Glyph and
// kerning caches are per font; the bitmaps go to PSRAM through the C allocator.
#define LV_USE_TINY_TTF 1
#define LV_TINY_TTF_FILE_SUPPORT 0
#define LV_TINY_TTF_CACHE_GLYPH_CNT 256
#define LV_TINY_TTF_CACHE_KERNING_CNT 256

// TJpgDec for the cover backdrop (src/app/backdrop.cpp decodes with it directly).
#define LV_USE_TJPGD 1

// Setup screen: QR codes to join the panel's access point and open its page.
#define LV_USE_QRCODE 1

#define LV_BUILD_EXAMPLES 0
#define LV_BUILD_DEMOS 0

#endif  // LV_CONF_H
