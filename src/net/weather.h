// Weather on the idle clock, from Open-Meteo: no account, no key, no tracking.
//
// A place is typed as a name ("Montreal", "Lyon, France"); it is turned into coordinates once
// and they are kept, so the lookup runs again only when the name changes. The forecast itself
// is asked for every twenty minutes, which is far more often than the clock needs.
#pragma once

#include <Arduino.h>

#include "../app/i18n.h"

namespace net::weather {

struct Now {
    bool valid = false;
    int16_t tempC = 0;         // now
    int16_t minC = 0, maxC = 0;   // today
    uint8_t code = 0;          // WMO weather code
    String place;              // as the geocoder named it
    String problem;            // why there is nothing to show, in words for the person who typed it
    String located;            // the full match: "Montréal, Quebec, Canada (45.51, -73.59)"
};

void tick(uint32_t now);       // net task: resolves the place, then refreshes on its own schedule
void reset();                  // the place changed: look it up again
const char *describe(uint8_t wmoCode, i18n::Lang lang = i18n::lang());   // "Light snow", "Overcast"...

}  // namespace net::weather
