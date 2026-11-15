#include <app/i18n.h>
#include <unity.h>

#include <string>
#include <vector>

using i18n::Lang;

void setUp() { i18n::setLang(Lang::En); }
void tearDown() {}

// The printf conversions of a string, in order: "%s at %d %%" -> {"%s", "%d"}.
static std::vector<std::string> conversions(const char *s) {
    std::vector<std::string> out;
    for (const char *p = s; *p; p++) {
        if (*p != '%') continue;
        const char *start = p++;
        if (*p == '%') continue;
        while (*p && !strchr("sdiufxXc", *p)) p++;
        out.push_back(std::string(start, p - start + 1));
        if (!*p) break;
    }
    return out;
}

static int lines(const char *s) {
    int n = 1;
    for (; *s; s++) n += *s == '\n';
    return n;
}

// Identical in both languages on purpose.
static const char *const SAME[] = {"English\nFrançais", "Zone", "QR code", "Volume", "Instrumental"};

void test_every_string_has_french_with_the_same_arguments_and_items() {
    for (int id = 0; id < (int)i18n::S::Count; id++) {
        const char *en = i18n::raw(id, Lang::En), *fr = i18n::raw(id, Lang::Fr);
        char what[96];
        snprintf(what, sizeof what, "id %d \"%.60s\"", id, en);
        TEST_ASSERT_TRUE_MESSAGE(*en && *fr, what);
        // A wrong argument list reads the stack: a crash in one language only. Page rows are not
        // printf formats (their %VERSION% is a template token).
        if (!i18n::pageName(id)) TEST_ASSERT_TRUE_MESSAGE(conversions(en) == conversions(fr), what);
        // Lists (days, time zones, dropdown choices) are picked by position.
        TEST_ASSERT_EQUAL_INT_MESSAGE(lines(en), lines(fr), what);
        bool same = false;
        for (const char *s : SAME) same |= !strcmp(en, s);
        TEST_ASSERT_TRUE_MESSAGE(same == !strcmp(en, fr), what);
        TEST_ASSERT_NULL_MESSAGE(strchr(fr, '\''), what);
        for (const char *bad : {" ?", " !", " :", " ;", " »", "« "}) TEST_ASSERT_NULL_MESSAGE(strstr(fr, bad), what);
    }
}

void test_the_chosen_language_fills_screens_and_pages() {
    TEST_ASSERT_EQUAL_STRING("Settings", tr(i18n::S::Settings));
    TEST_ASSERT_EQUAL_STRING("Install v1.2.0", i18n::fmt(i18n::S::FwInstall, "v1.2.0").c_str());
    i18n::setLang(Lang::Fr);
    TEST_ASSERT_EQUAL_STRING("Réglages", tr(i18n::S::Settings));
    TEST_ASSERT_EQUAL_STRING("Installer v1.2.0", i18n::fmt(i18n::S::FwInstall, "v1.2.0").c_str());
    TEST_ASSERT_EQUAL_STRING("samedi", i18n::nth(i18n::S::Days, 6).c_str());
    TEST_ASSERT_EQUAL_STRING("", i18n::nth(i18n::S::Days, 7).c_str());
    String page = "<button>%PgSave%</button>";
    i18n::fillPage(page);
    TEST_ASSERT_EQUAL_STRING("<button>Enregistrer</button>", page.c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_every_string_has_french_with_the_same_arguments_and_items);
    RUN_TEST(test_the_chosen_language_fills_screens_and_pages);
    return UNITY_END();
}
