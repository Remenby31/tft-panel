// Every user-visible string, in English and French, in one table (i18n_strings.inc): X() for
// the screens, P() for the web pages, whose templates name them as `%Id%`.
// Logs and the serial console stay English. The language is read once at boot: changing it
// restarts the panel, so no screen has to be rebuilt.
// Without French (I18N_NO_FR) the build pays nothing for the table: tr() is constexpr, each call
// folds to its English literal, and page templates are filled at compile time (I18N_PAGE).
#pragma once

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include <array>

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

namespace detail {
inline constexpr const char *EN[] = {
#define X(id, en, fr) en,
#define P(id, en, fr) en,
#include "i18n_strings.inc"
#undef X
#undef P
};
String format(const char *f, ...);
}  // namespace detail
using detail::format;   // printf with a format chosen at run time: format(c ? tr(A) : tr(B), ...)

#ifdef I18N_NO_FR
constexpr void setLang(Lang) {}
constexpr Lang lang() { return Lang::En; }
constexpr const char *tr(S id) { return detail::EN[(int)id]; }
constexpr const char *tr(S id, Lang) { return detail::EN[(int)id]; }
inline void fillPage(String &) {}
#else
void setLang(Lang l);
Lang lang();
const char *tr(S id);                     // in the current language
const char *tr(S id, Lang l);
// Replaces each `%Id%` token of a web page template with its text, for the P() rows.
void fillPage(String &html);
#endif
const char *code(Lang l);                 // "en", "fr": console, NVS-independent names
bool parse(const char *code, Lang &out);

// printf through the table; every language of an id takes the same arguments (host test).
// Inline, so that a constant id stays a constant: `tr(cond ? A : B)` would index the table at
// run time; write `cond ? tr(A) : tr(B)` instead.
template <class... A>
inline __attribute__((always_inline)) String fmt(S id, A... args) {
    return detail::format(tr(id), args...);
}

// For the host test: the raw table.
const char *raw(int id, Lang l);
const char *pageName(int id);             // nullptr when `id` is not a page string

// A page template whose `%Id%` tokens are filled: at compile time with English when the build has
// no French, at run time (fillPage) otherwise.
//   I18N_PAGE(SETTINGS_PAGE, R"HTML(...)HTML");
namespace detail {
struct PageRow {
    const char *token;
    S id;
};
inline constexpr PageRow PAGE_ROWS[] = {
#define X(id, en, fr)
#define P(id, en, fr) {"%" #id "%", S::id},
#include "i18n_strings.inc"
#undef X
#undef P
};
constexpr size_t slen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}
constexpr int rowAt(const char *t, size_t i) {
    for (size_t r = 0; r < sizeof PAGE_ROWS / sizeof PAGE_ROWS[0]; r++) {
        const char *k = PAGE_ROWS[r].token;
        size_t j = 0;
        while (k[j] && t[i + j] == k[j]) j++;
        if (!k[j]) return (int)r;
    }
    return -1;
}
template <size_t N>
constexpr size_t filledSize(const char (&t)[N]) {
    size_t n = 0;
    for (size_t i = 0; i < N - 1;) {
        int r = t[i] == '%' ? rowAt(t, i) : -1;
        if (r < 0) {
            n++, i++;
            continue;
        }
        n += slen(EN[(int)PAGE_ROWS[r].id]);
        i += slen(PAGE_ROWS[r].token);
    }
    return n + 1;
}
template <size_t M, size_t N>
constexpr std::array<char, M> fillEnglish(const char (&t)[N]) {
    std::array<char, M> out{};
    size_t o = 0;
    for (size_t i = 0; i < N - 1;) {
        int r = t[i] == '%' ? rowAt(t, i) : -1;
        if (r < 0) {
            out[o++] = t[i++];
            continue;
        }
        for (const char *s = EN[(int)PAGE_ROWS[r].id]; *s;) out[o++] = *s++;
        i += slen(PAGE_ROWS[r].token);
    }
    out[o] = 0;
    return out;
}
}  // namespace detail

}  // namespace i18n

#ifdef I18N_NO_FR
#define I18N_PAGE(name, tpl)                                                                           \
    constexpr char name##_TPL[] = tpl;                                                                 \
    constexpr auto name##_EN = i18n::detail::fillEnglish<i18n::detail::filledSize(name##_TPL)>(name##_TPL); \
    const char *const name = name##_EN.data();
#else
#define I18N_PAGE(name, tpl) const char name[] = tpl;
#endif

using i18n::tr;
using Str = i18n::S;
