#include <net/zone_group.h>
#include <unity.h>

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

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_joining_puts_the_followed_zones_outputs_first);
    RUN_TEST(test_joining_a_zone_already_in_the_group_does_nothing);
    RUN_TEST(test_only_a_group_can_be_split);
    return UNITY_END();
}
