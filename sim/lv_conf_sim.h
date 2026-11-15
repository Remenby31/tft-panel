// LVGL configuration of the simulator: the firmware's own (include/lv_conf.h), with what only a
// desktop needs changed. Selected by -DLV_CONF_PATH in [env:sim].
#pragma once

#include "../include/lv_conf.h"

// The firmware routes LVGL's heap to PSRAM (src/board/lv_mem_psram.c); the host has one heap.
#undef LV_USE_STDLIB_MALLOC
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB

// --window: an SDL2 window, the mouse as the touch screen.
#define LV_USE_SDL 1
#define LV_SDL_INCLUDE_PATH <SDL2/SDL.h>
#define LV_SDL_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT
#define LV_SDL_BUF_COUNT 1
