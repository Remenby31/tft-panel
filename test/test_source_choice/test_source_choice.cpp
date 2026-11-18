#include <net/source_choice.h>
#include <unity.h>

using settings::SourceKind;
using sourcechoice::Active;
using sourcechoice::Chooser;
using sourcechoice::Seen;

void setUp() {}
void tearDown() {}

Seen seen(bool companion, bool server, bool approved) {
    Seen s;
    s.companion = companion;
    s.server = server;
    s.approved = approved;
    return s;
}
const Seen NOTHING = seen(false, false, false);
const Seen COMPANION = seen(true, false, false);
const Seen BOTH_PENDING = seen(true, true, false);
const Seen BOTH_APPROVED = seen(true, true, true);
const Seen SERVER_PENDING = seen(false, true, false);

void test_a_panel_updated_with_wifi_keeps_its_companion_and_a_new_one_looks() {
    TEST_ASSERT_EQUAL(SourceKind::Companion, sourcechoice::fromStored(false, 0, true));
    TEST_ASSERT_EQUAL(SourceKind::Auto, sourcechoice::fromStored(false, 0, false));
}

void test_a_stored_source_reads_back_whatever_the_wifi() {
    TEST_ASSERT_EQUAL(SourceKind::Companion, sourcechoice::fromStored(true, 0, false));
    TEST_ASSERT_EQUAL(SourceKind::Direct, sourcechoice::fromStored(true, 1, false));
    TEST_ASSERT_EQUAL(SourceKind::Auto, sourcechoice::fromStored(true, 2, true));
}

void test_a_choice_made_by_hand_never_moves() {
    Chooser companion(SourceKind::Companion, false);
    TEST_ASSERT_EQUAL(Active::Companion, companion.step(BOTH_APPROVED, 1000));
    Chooser direct(SourceKind::Direct, false);
    TEST_ASSERT_EQUAL(Active::Direct, direct.step(COMPANION, 1000));
    TEST_ASSERT_EQUAL(Active::Direct, direct.step(COMPANION, 600000));
}

void test_both_found_plays_through_the_companion_at_once_then_moves_on_approval() {
    Chooser c(SourceKind::Auto, false);
    TEST_ASSERT_EQUAL(Active::Looking, c.step(NOTHING, 1000));
    TEST_ASSERT_EQUAL(Active::Companion, c.step(BOTH_PENDING, 2000));
    TEST_ASSERT_TRUE(c.awaitingApproval());
    TEST_ASSERT_EQUAL(Active::Companion, c.step(BOTH_PENDING, 600000));   // never approved: stays
    TEST_ASSERT_EQUAL(Active::Direct, c.step(BOTH_APPROVED, 600100));
    TEST_ASSERT_FALSE(c.awaitingApproval());
}

void test_a_companion_alone_plays_at_once() {
    Chooser c(SourceKind::Auto, false);
    TEST_ASSERT_EQUAL(Active::Companion, c.step(COMPANION, 1000));
    TEST_ASSERT_FALSE(c.awaitingApproval());
}

void test_a_server_alone_asks_for_its_approval_until_a_companion_can_play() {
    Chooser c(SourceKind::Auto, false);
    TEST_ASSERT_EQUAL(Active::Direct, c.step(SERVER_PENDING, 1000));
    TEST_ASSERT_EQUAL(Active::Companion, c.step(BOTH_PENDING, 5000));
}

void test_a_panel_paired_before_waits_a_moment_for_its_server_instead_of_switching() {
    Chooser quick(SourceKind::Auto, true);
    TEST_ASSERT_EQUAL(Active::Looking, quick.step(COMPANION, 1000));
    TEST_ASSERT_EQUAL(Active::Direct, quick.step(BOTH_APPROVED, 4000));

    Chooser gone(SourceKind::Auto, true);
    TEST_ASSERT_EQUAL(Active::Looking, gone.step(COMPANION, 1000));
    TEST_ASSERT_EQUAL(Active::Looking, gone.step(COMPANION, 1000 + 11999));
    TEST_ASSERT_EQUAL(Active::Companion, gone.step(COMPANION, 1000 + 12000));
}

void test_a_server_that_drops_hands_over_to_the_companion_after_a_grace_and_cannot_flap() {
    Chooser c(SourceKind::Auto, false);
    c.step(BOTH_APPROVED, 1000);
    TEST_ASSERT_EQUAL(Active::Direct, c.step(COMPANION, 10000));           // a short drop
    TEST_ASSERT_EQUAL(Active::Direct, c.step(BOTH_APPROVED, 20000));       // back before the grace
    TEST_ASSERT_EQUAL(Active::Direct, c.step(COMPANION, 30000));
    TEST_ASSERT_EQUAL(Active::Direct, c.step(COMPANION, 30000 + 14999));
    TEST_ASSERT_EQUAL(Active::Companion, c.step(COMPANION, 30000 + 15000));
    TEST_ASSERT_EQUAL(Active::Companion, c.step(BOTH_APPROVED, 45000 + 59999));   // held
    TEST_ASSERT_EQUAL(Active::Direct, c.step(BOTH_APPROVED, 45000 + 60000));
}

void test_a_companion_that_drops_hands_over_to_the_server_after_the_same_grace() {
    Chooser c(SourceKind::Auto, false);
    c.step(BOTH_PENDING, 1000);
    TEST_ASSERT_EQUAL(Active::Companion, c.step(SERVER_PENDING, 2000));
    TEST_ASSERT_EQUAL(Active::Companion, c.step(SERVER_PENDING, 2000 + 14999));
    TEST_ASSERT_EQUAL(Active::Direct, c.step(SERVER_PENDING, 2000 + 15000));
}

void test_nothing_found_keeps_looking() {
    Chooser c(SourceKind::Auto, false);
    TEST_ASSERT_EQUAL(Active::Looking, c.step(NOTHING, 600000));
}

void test_the_status_line_says_what_was_found() {
    char line[200];
    Chooser looking(SourceKind::Auto, true);
    looking.step(NOTHING, 1000);
    sourcechoice::describe(looking, NOTHING, false, "TFT Panel A1B2", line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Looking for your music server, or a TextFromTrack companion, on this Wi-Fi...", line);
    looking.step(COMPANION, 2000);
    sourcechoice::describe(looking, COMPANION, false, "TFT Panel A1B2", line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Found a TextFromTrack companion. Checking for a music server too...", line);

    Chooser c(SourceKind::Auto, false);
    c.step(BOTH_PENDING, 1000);
    sourcechoice::describe(c, BOTH_PENDING, true, "TFT Panel A1B2", line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Playing through the TextFromTrack companion. To play straight from your music "
                             "server, enable \"TFT Panel A1B2\" in its Settings > Extensions.",
                             line);
    c.step(BOTH_APPROVED, 2000);
    sourcechoice::describe(c, BOTH_APPROVED, true, "TFT Panel A1B2", line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Found your music server and a TextFromTrack companion: music from the server, "
                             "lyrics from the companion.",
                             line);
    // A companion that the lyrics will not reach (typed address, lyrics set elsewhere) is not promised.
    sourcechoice::describe(c, BOTH_APPROVED, false, "TFT Panel A1B2", line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Found your music server: music comes from it directly.", line);

    Chooser alone(SourceKind::Auto, false);
    alone.step(COMPANION, 1000);
    sourcechoice::describe(alone, COMPANION, false, "TFT Panel A1B2", line, sizeof line);
    TEST_ASSERT_EQUAL_STRING("Found a TextFromTrack companion: music and lyrics come from it.", line);
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
    RUN_TEST(test_a_choice_made_by_hand_never_moves);
    RUN_TEST(test_both_found_plays_through_the_companion_at_once_then_moves_on_approval);
    RUN_TEST(test_a_companion_alone_plays_at_once);
    RUN_TEST(test_a_server_alone_asks_for_its_approval_until_a_companion_can_play);
    RUN_TEST(test_a_panel_paired_before_waits_a_moment_for_its_server_instead_of_switching);
    RUN_TEST(test_a_server_that_drops_hands_over_to_the_companion_after_a_grace_and_cannot_flap);
    RUN_TEST(test_a_companion_that_drops_hands_over_to_the_server_after_the_same_grace);
    RUN_TEST(test_nothing_found_keeps_looking);
    RUN_TEST(test_the_status_line_says_what_was_found);
    RUN_TEST(test_every_word_reads_back_as_its_setting);
    return UNITY_END();
}
