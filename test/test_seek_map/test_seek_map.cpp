#include <app/seek_map.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

// A 640 px bar starting at x = 80, on a 4 minute track.
void test_a_touch_maps_linearly_along_the_bar() {
    TEST_ASSERT_EQUAL_INT32(0, seekTargetMs(80, 80, 640, 240000));
    TEST_ASSERT_EQUAL_INT32(120000, seekTargetMs(400, 80, 640, 240000));
    TEST_ASSERT_EQUAL_INT32(60000, seekTargetMs(240, 80, 640, 240000));
}

void test_a_touch_in_the_enlarged_area_clamps_to_the_ends() {
    TEST_ASSERT_EQUAL_INT32(0, seekTargetMs(60, 80, 640, 240000));
    TEST_ASSERT_EQUAL_INT32(240000, seekTargetMs(740, 80, 640, 240000));
}

void test_no_duration_or_no_width_seeks_to_the_start() {
    TEST_ASSERT_EQUAL_INT32(0, seekTargetMs(400, 80, 640, 0));
    TEST_ASSERT_EQUAL_INT32(0, seekTargetMs(400, 80, 0, 240000));
}

void test_a_long_track_does_not_overflow() {
    TEST_ASSERT_EQUAL_INT32(5400000, seekTargetMs(400, 80, 640, 10800000));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_a_touch_maps_linearly_along_the_bar);
    RUN_TEST(test_a_touch_in_the_enlarged_area_clamps_to_the_ends);
    RUN_TEST(test_no_duration_or_no_width_seeks_to_the_start);
    RUN_TEST(test_a_long_track_does_not_overflow);
    return UNITY_END();
}
