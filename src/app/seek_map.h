// Where on a track a touch on the progress bar points. Free of LVGL and Arduino so it can be
// unit-tested on the host.
#pragma once

#include <stdint.h>

// touchX and barX are screen pixels, barW the drawn width. A touch past either end of the bar
// (the hit area is wider than the drawing) clamps to the start or the end of the track.
inline int32_t seekTargetMs(int32_t touchX, int32_t barX, int32_t barW, uint32_t durationMs) {
    if (barW <= 0 || !durationMs) return 0;
    int32_t dx = touchX - barX;
    if (dx <= 0) return 0;
    if (dx >= barW) return (int32_t)durationMs;
    return (int32_t)((int64_t)dx * durationMs / barW);
}
