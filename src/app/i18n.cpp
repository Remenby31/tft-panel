#include "i18n.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace i18n {
namespace detail {

String format(const char *f, ...) {
    va_list ap, again;
    va_start(ap, f);
    va_copy(again, ap);
    int n = vsnprintf(nullptr, 0, f, ap);
    va_end(ap);
    String out;
    if (n > 0) {
        char *buf = (char *)malloc(n + 1);
        if (buf) {
            vsnprintf(buf, n + 1, f, again);
            out = String(buf);
            free(buf);
        }
    }
    va_end(again);
    return out;
}

String nthOf(const char *p, int n) {
    for (; n > 0 && p; n--) {
        p = strchr(p, '\n');
        if (p) p++;
    }
    if (!p || n < 0) return String();
    const char *end = strchr(p, '\n');
    return end ? String(p).substring(0, end - p) : String(p);
}

}  // namespace detail

#ifndef I18N_NO_FR
namespace {

const char *const FR[] = {
#define X(id, en, fr) fr,
#define P(id, en, fr) fr,
#include "i18n_strings.inc"
#undef X
#undef P
};

volatile Lang g_lang = Lang::En;

}  // namespace

void setLang(Lang l) { g_lang = l == Lang::Fr ? Lang::Fr : Lang::En; }
Lang lang() { return g_lang; }

const char *tr(S id, Lang l) {
    if ((int)id >= (int)S::Count) return "";
    return l == Lang::Fr ? FR[(int)id] : detail::EN[(int)id];
}

const char *tr(S id) { return tr(id, g_lang); }

void fillPage(String &html) {
    for (const detail::PageRow &r : detail::PAGE_ROWS) html.replace(r.token, tr(r.id));
}
#endif

const char *code(Lang l) { return l == Lang::Fr ? "fr" : "en"; }

bool parse(const char *c, Lang &out) {
    if (!strcmp(c, "en")) out = Lang::En;
    else if (!strcmp(c, "fr")) out = Lang::Fr;
    else return false;
    return true;
}

const char *raw(int id, Lang l) { return tr((S)id, l); }

const char *pageName(int id) {
    for (const detail::PageRow &r : detail::PAGE_ROWS)
        if ((int)r.id == id) return r.token;
    return nullptr;
}

}  // namespace i18n
