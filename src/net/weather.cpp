#include "weather.h"

#include "http_get.h"
#include "net_internal.h"
#include "settings.h"

namespace net::weather {

using namespace internal;

namespace {

constexpr uint32_t REFRESH_MS = 20u * 60u * 1000u;
constexpr uint32_t RETRY_MS = 2u * 60u * 1000u;

String g_place;            // the short name shown on the clock
String g_full;             // the full match, shown where the town was typed
float g_lat = 0, g_lon = 0;
bool g_located = false;
uint32_t g_nextAt = 0;

String urlEncode(const String &in) {
    static const char *DIGITS = "0123456789ABCDEF";
    String out;
    out.reserve(in.length() + 16);
    for (unsigned i = 0; i < in.length(); i++) {
        uint8_t c = (uint8_t)in[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') out += (char)c;
        else {
            out += '%';
            out += DIGITS[c >> 4];
            out += DIGITS[c & 0x0f];
        }
    }
    return out;
}

// One search. The geocoder matches a plain town name, so "Quebec City" or "Lyon, France" find
// nothing until they are cut down; the caller tries the shorter forms in turn.
bool search(const String &name, JsonDocument &doc) {
    // Ten results, not one: the first is not always the right Paris. The caller chooses.
    String url = "https://geocoding-api.open-meteo.com/v1/search?count=10&language=en&format=json&name=" +
                 urlEncode(name);
    httpget::Result r = httpget::json(url, doc);
    if (!r.ok()) {
        Serial.printf("[weather] could not look up \"%s\": %s\n", name.c_str(), r.message.c_str());
        return false;
    }
    return !doc["results"][0].isNull();
}

bool matches(JsonObjectConst hit, const String &want) {
    if (!want.length()) return true;
    String low = want;
    low.toLowerCase();
    for (const char *field : {"country", "country_code", "admin1", "admin2"}) {
        String v = hit[field] | "";
        v.toLowerCase();
        if (v.length() && (v.indexOf(low) >= 0 || low.indexOf(v) >= 0)) return true;
    }
    return false;
}

bool locate(const String &place) {
    JsonDocument doc(psram());
    // "Montreal, Canada" or "Springfield, Illinois": the town is searched for, the rest picks
    // between the towns that share its name.
    int comma = place.indexOf(',');
    String town = comma > 0 ? place.substring(0, comma) : place;
    String where = comma > 0 ? place.substring(comma + 1) : String();
    town.trim();
    where.trim();

    // As typed, then the town alone, then its first word: "Quebec City" is indexed as "Québec".
    bool found = search(place, doc);
    if (!found && town != place) found = search(town, doc);
    if (!found && town.indexOf(' ') > 0) found = search(town.substring(0, town.indexOf(' ')), doc);

    // Among the matches, the one the region or country points at, else the largest: that is the
    // Paris, the Montreal and the Springfield a person means nine times out of ten.
    JsonObjectConst hit;
    long best = -1;
    for (JsonObjectConst c : doc["results"].as<JsonArrayConst>()) {
        if (!matches(c, where)) continue;
        long pop = c["population"] | 0;
        if (pop > best) {
            best = pop;
            hit = c;
        }
    }
    if (!found || hit.isNull()) {
        Serial.printf("[weather] no place called \"%s\"\n", place.c_str());
        Now w;
        w.problem = i18n::fmt(Str::WeatherNoPlace, place.c_str());
        publishWeather(w);   // say so rather than showing nothing and leaving them guessing
        return false;
    }
    g_lat = hit["latitude"] | 0.0f;
    g_lon = hit["longitude"] | 0.0f;
    String name = hit["name"] | place.c_str();
    String admin = hit["admin1"] | "";
    String country = hit["country_code"] | "";
    g_place = name;
    if (admin.length() && admin != name) g_place += ", " + admin;
    // The full match is shown where the town was typed, so the wrong Paris is obvious at once.
    String fullCountry = hit["country"] | "";
    g_located = true;
    g_full = name;
    if (admin.length() && admin != name) g_full += ", " + admin;
    if (fullCountry.length()) g_full += ", " + fullCountry;
    g_full += " (" + String(g_lat, 2) + ", " + String(g_lon, 2) + ")";
    Serial.printf("[weather] %s at %.3f, %.3f\n", g_full.c_str(), g_lat, g_lon);
    return true;
}

bool refresh() {
    JsonDocument doc(psram());
    char url[220];
    snprintf(url, sizeof url,
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,weather_code&daily=temperature_2m_max,temperature_2m_min"
             "&forecast_days=1&timezone=auto",
             g_lat, g_lon);
    httpget::Result r = httpget::json(url, doc);
    if (!r.ok()) {
        Serial.printf("[weather] no forecast: %s\n", r.message.c_str());
        return false;
    }
    Now w;
    w.valid = true;
    w.place = g_place;
    w.located = g_full;
    w.tempC = (int16_t)lroundf(doc["current"]["temperature_2m"] | 0.0f);
    w.code = (uint8_t)(doc["current"]["weather_code"] | 0);
    w.maxC = (int16_t)lroundf(doc["daily"]["temperature_2m_max"][0] | 0.0f);
    w.minC = (int16_t)lroundf(doc["daily"]["temperature_2m_min"][0] | 0.0f);
    Serial.printf("[weather] %s %d C, %s, today %d to %d\n", w.place.c_str(), (int)w.tempC, describe(w.code, i18n::Lang::En),
                  (int)w.minC, (int)w.maxC);
    publishWeather(w);
    return true;
}

}  // namespace

void reset() {
    g_located = false;
    g_place = "";
    g_full = "";
    g_nextAt = 0;
    publishWeather({});
}

const char *describe(uint8_t c, i18n::Lang lang) {
    // WMO 4677, as Open-Meteo uses it, in the words a clock face needs.
    Str id;
    switch (c) {
        case 0: id = Str::WxClear; break;
        case 1: id = Str::WxMainlyClear; break;
        case 2: id = Str::WxPartlyCloudy; break;
        case 3: id = Str::WxOvercast; break;
        case 45: case 48: id = Str::WxFog; break;
        case 51: case 53: case 55: id = Str::WxDrizzle; break;
        case 56: case 57: id = Str::WxFreezingDrizzle; break;
        case 61: id = Str::WxLightRain; break;
        case 63: id = Str::WxRain; break;
        case 65: id = Str::WxHeavyRain; break;
        case 66: case 67: id = Str::WxFreezingRain; break;
        case 71: id = Str::WxLightSnow; break;
        case 73: id = Str::WxSnow; break;
        case 75: id = Str::WxHeavySnow; break;
        case 77: id = Str::WxSnowGrains; break;
        case 80: case 81: id = Str::WxShowers; break;
        case 82: id = Str::WxHeavyShowers; break;
        case 85: case 86: id = Str::WxSnowShowers; break;
        case 95: id = Str::WxThunderstorm; break;
        case 96: case 99: id = Str::WxThunderstormWithHail; break;
        default: return "";
    }
    return i18n::tr(id, lang);
}

void tick(uint32_t now) {
    String place = settings::get().weatherPlace;
    if (!place.length()) {
        if (g_located || g_place.length()) reset();
        return;
    }
    if (g_nextAt && (int32_t)(now - g_nextAt) < 0) return;

    if (!g_located || !g_place.startsWith(place)) {
        if (!locate(place)) {
            g_nextAt = now + RETRY_MS;
            return;
        }
    }
    g_nextAt = now + (refresh() ? REFRESH_MS : RETRY_MS);
}

}  // namespace net::weather
