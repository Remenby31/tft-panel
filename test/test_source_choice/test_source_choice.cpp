#include <net/source_choice.h>
#include <unity.h>

using settings::SourceKind;
using sourcechoice::choose;
using sourcechoice::Pick;
using sourcechoice::Probe;

void setUp() {}
void tearDown() {}

Probe found(bool companion, bool server, uint32_t companionAtMs = 1000) {
    Probe p;
    p.companion = companion;
    p.server = server;
    p.companionAtMs = companionAtMs;
    return p;
}

void test_a_panel_updated_with_wifi_keeps_its_companion_and_a_new_one_looks() {
    TEST_ASSERT_EQUAL(SourceKind::Companion, sourcechoice::fromStored(false, 0, true));
    TEST_ASSERT_EQUAL(SourceKind::Auto, sourcechoice::fromStored(false, 0, false));
}

void test_a_stored_source_reads_back_whatever_the_wifi() {
    TEST_ASSERT_EQUAL(SourceKind::Companion, sourcechoice::fromStored(true, 0, false));
    TEST_ASSERT_EQUAL(SourceKind::Direct, sourcechoice::fromStored(true, 1, false));
    TEST_ASSERT_EQUAL(SourceKind::Auto, sourcechoice::fromStored(true, 2, true));
}

void test_a_choice_made_by_hand_wins_over_what_is_found() {
    TEST_ASSERT_EQUAL(Pick::Companion, choose(SourceKind::Companion, found(false, true), 0));
    TEST_ASSERT_EQUAL(Pick::Direct, choose(SourceKind::Direct, found(true, false, 0), 60000));
    TEST_ASSERT_EQUAL(Pick::Direct, choose(SourceKind::Direct, found(false, false), 0));
}

void test_auto_takes_the_music_server_whenever_one_answers() {
    TEST_ASSERT_EQUAL(Pick::Direct, choose(SourceKind::Auto, found(false, true), 0));
    TEST_ASSERT_EQUAL(Pick::Direct, choose(SourceKind::Auto, found(true, true), 1000));
}

void test_auto_waits_for_a_server_before_settling_on_a_companion_alone() {
    TEST_ASSERT_EQUAL(Pick::Looking, choose(SourceKind::Auto, found(true, false, 1000), 1000 + 11999));
    TEST_ASSERT_EQUAL(Pick::Companion, choose(SourceKind::Auto, found(true, false, 1000), 1000 + 12000));
}

void test_auto_keeps_looking_when_nothing_answers() {
    TEST_ASSERT_EQUAL(Pick::Looking, choose(SourceKind::Auto, found(false, false), 600000));
}

void test_the_status_line_says_what_was_found() {
    char line[160];
    sourcechoice::describe(Pick::Looking, found(false, false), false, line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Looking for your music server, or a TextFromTrack companion, on this Wi-Fi...", line);
    sourcechoice::describe(Pick::Looking, found(true, false), false, line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Found a TextFromTrack companion. Checking for a music server too...", line);
    sourcechoice::describe(Pick::Companion, found(true, false), false, line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Found a TextFromTrack companion: music and lyrics come from it.", line);
    sourcechoice::describe(Pick::Direct, found(true, true), true, line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Found your music server and a TextFromTrack companion: music from the server, "
                             "lyrics from the companion.",
                             line);
    // A companion that the lyrics will not reach (typed address, lyrics set elsewhere) is not promised.
    sourcechoice::describe(Pick::Direct, found(true, true), false, line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Found your music server: music comes from it directly.", line);
}

void test_every_word_reads_back_as_its_setting() {
    const SourceKind kinds[] = {SourceKind::Companion, SourceKind::Direct, SourceKind::Auto};
    for (SourceKind k : kinds) TEST_ASSERT_EQUAL(k, sourcechoice::parse(sourcechoice::name(k)));
    TEST_ASSERT_EQUAL(SourceKind::Direct, sourcechoice::parse("roon"));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_a_panel_updated_with_wifi_keeps_its_companion_and_a_new_one_looks);
    RUN_TEST(test_a_stored_source_reads_back_whatever_the_wifi);
    RUN_TEST(test_a_choice_made_by_hand_wins_over_what_is_found);
    RUN_TEST(test_auto_takes_the_music_server_whenever_one_answers);
    RUN_TEST(test_auto_waits_for_a_server_before_settling_on_a_companion_alone);
    RUN_TEST(test_auto_keeps_looking_when_nothing_answers);
    RUN_TEST(test_the_status_line_says_what_was_found);
    RUN_TEST(test_every_word_reads_back_as_its_setting);
    return UNITY_END();
}
