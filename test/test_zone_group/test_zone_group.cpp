#include <net/zone_group.h>
#include <unity.h>

using net::group::canJoin;
using net::group::joinOutputs;
using net::group::splitOutputs;
using Ids = std::vector<String>;

void setUp() {}
void tearDown() {}

void assertIds(const Ids &expected, const Ids &got) {
    TEST_ASSERT_EQUAL(expected.size(), got.size());
    for (size_t i = 0; i < expected.size(); i++) TEST_ASSERT_EQUAL_STRING(expected[i].c_str(), got[i].c_str());
}

// Roon keeps the first output's queue: the followed zone's music must go on in the group.
void test_joining_puts_the_followed_zones_outputs_first() {
    assertIds({"out-living", "out-kitchen"}, joinOutputs({"out-living"}, {"out-kitchen"}));
    assertIds({"out-living", "out-office", "out-kitchen", "out-dining"},
              joinOutputs({"out-living", "out-office"}, {"out-kitchen", "out-dining"}));
}

void test_joining_a_zone_already_in_the_group_does_nothing() {
    assertIds({}, joinOutputs({"out-living", "out-kitchen"}, {"out-kitchen"}));
    assertIds({}, joinOutputs({}, {"out-kitchen"}));
    assertIds({}, joinOutputs({"out-living"}, {}));
}

void test_only_a_group_can_be_split() {
    assertIds({"out-kitchen", "out-dining"}, splitOutputs({"out-kitchen", "out-dining"}));
    assertIds({}, splitOutputs({"out-office"}));
}

// "+ Add to <followed zone>" only where the server says the outputs can share a group.
void test_a_zone_is_offered_only_if_the_server_says_its_outputs_can_join() {
    Ids living = {"out-living", "out-kitchen", "out-dining", "out-office"};   // can_group_with_output_ids
    TEST_ASSERT_TRUE(canJoin(living, true, {"out-office"}));
    TEST_ASSERT_TRUE(canJoin(living, true, {"out-kitchen", "out-dining"}));
    TEST_ASSERT_FALSE(canJoin(living, true, {"out-garden"}));                 // another kind of device
    TEST_ASSERT_FALSE(canJoin(living, true, {"out-kitchen", "out-garden"}));  // one output is enough to refuse
    TEST_ASSERT_FALSE(canJoin(living, true, {}));                             // no output to send
}

// A companion that does not pass can_group_with_output_ids on: every zone is offered.
void test_a_source_that_does_not_say_offers_every_zone() {
    TEST_ASSERT_TRUE(canJoin({}, false, {"out-garden"}));
    TEST_ASSERT_FALSE(canJoin({}, false, {}));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_joining_puts_the_followed_zones_outputs_first);
    RUN_TEST(test_joining_a_zone_already_in_the_group_does_nothing);
    RUN_TEST(test_only_a_group_can_be_split);
    RUN_TEST(test_a_zone_is_offered_only_if_the_server_says_its_outputs_can_join);
    RUN_TEST(test_a_source_that_does_not_say_offers_every_zone);
    return UNITY_END();
}
