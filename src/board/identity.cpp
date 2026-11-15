// Is this image running on the board it was built for?
//
// Nothing else answers that question. The browser installer picks firmware by **chip family**,
// and the two ESP32-P4 boards share one, so it hands any P4 the Waveshare image; a person
// flashing by hand picks a file from a list of three. Given the wrong one a board boots, runs,
// joins Wi-Fi — and shows nothing, because the panel behind the wrong driver never lights. That
// looks like dead hardware and has cost a day before.
//
// So the firmware says it itself, out of what the chip reports: the model, and the flash size,
// which is what tells the two P4 boards apart (16 MB on the VIEWE, 32 MB on the Waveshare).

#include <Arduino.h>
#include <esp_flash.h>

#include "board.h"
#include "board_pins.h"

namespace board {

String hardwareVerdict() {
    const String chip = ESP.getChipModel();          // "ESP32-S3", "ESP32-P4"…
    uint32_t bytes = info().flashBytes;
    if (!bytes) {                                     // before begin(), or if the probe failed
        uint32_t probed = 0;
        if (esp_flash_get_size(nullptr, &probed) == ESP_OK) bytes = probed;
    }
    const uint32_t mb = bytes / (1024u * 1024u);

    // The chip is the hard mismatch: an image for another family cannot drive anything here.
    if (!chip.startsWith(EXPECTED_CHIP)) {
        return String("This firmware is built for the ") + BOARD_NAME + " (" + EXPECTED_CHIP +
               "), but this board is a " + chip + ". Flash the image for your board: " +
               "https://r45635.github.io/tft-panel/install.html";
    }
    // Same chip, different board: the flash size is the only thing that separates them.
    if (mb && EXPECTED_FLASH_MB && mb != EXPECTED_FLASH_MB) {
        return String("This firmware is built for the ") + BOARD_NAME + ", which has " +
               EXPECTED_FLASH_MB + " MB of flash; this board reports " + mb +
               " MB. The screen will stay dark until the matching image is flashed (asset \"" +
               BOARD_ASSET + "\" is for this firmware; see docs/HARDWARE.md for the other boards).";
    }
    return String();
}

}  // namespace board
