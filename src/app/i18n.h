// Every user-visible string, in English and French, in one table (i18n_strings.inc): X() for
// the screens, P() for the web pages, whose templates name them as `%Id%`.
// Logs and the serial console stay English. The language is read once at boot: changing it
// restarts the panel, so no screen has to be rebuilt.
#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace i18n {

enum class Lang : uint8_t { En = 0, Fr = 1 };
// A board short of flash builds without French (I18N_NO_FR, the CrowPanel): one column, and
// nowhere to choose a language.
#ifdef I18N_NO_FR
constexpr int LANG_COUNT = 1;
#else
constexpr int LANG_COUNT = 2;
#endif

enum class S : uint16_t {
#define X(id, en, fr) id,
#define P(id, en, fr) id,
#include "i18n_strings.inc"
#undef X
#undef P
    Count
};

void setLang(Lang l);
Lang lang();
const char *code(Lang l);                 // "en", "fr": console, NVS-independent names
bool parse(const char *code, Lang &out);

const char *tr(S id);                     // in the current language
const char *tr(S id, Lang l);           // English for a language this build does not have
// printf through the table; every language of an id takes the same arguments (host test).
String fmt(S id, ...);
// Item `n` of a '\n'-separated list (day and month names), empty when out of range.
String nth(S id, int n);
// Replaces each `%Id%` token of a web page template with its text, for the ids marked as page
// strings in the table.
void fillPage(String &html);

// For the host test: the raw table.
const char *raw(int id, Lang l);
const char *pageName(int id);             // nullptr when `id` is not a page string

}  // namespace i18n

using i18n::tr;
using Str = i18n::S;
