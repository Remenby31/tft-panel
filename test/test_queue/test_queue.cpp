#include <net/queue_parse.h>
#include <unity.h>

#include <cstring>
#include <string>
#include <vector>

struct Item {
    uint32_t id = 0;
    std::string title, artist, imageKey;
    uint32_t durationS = 0;
};

std::vector<Item> parse(const char *json, bool *ok = nullptr) {
    JsonDocument doc;
    TEST_ASSERT_FALSE(deserializeJson(doc, json));
    std::vector<Item> items = {{7, "stale"}};
    bool r = net::q::parse(doc.as<JsonVariantConst>(), items);
    if (ok) *ok = r;
    return items;
}

void setUp() {}
void tearDown() {}

// The companion's GET /api/roon/queue, as roonClient.getQueue + queuePrefetch shape it.
void test_items_keep_the_companions_order_and_fields() {
    auto items = parse(R"({"success":true,"zone_id":"z1","items":[
        {"queue_item_id":12,"title":"Neon Autumn","artist":"The Demo Lights","album":"Night Drive",
         "duration_seconds":214,"image_key":"img-a","lyrics":"synced"},
        {"queue_item_id":13,"title":"Harbour Fog","artist":"Paper Satellites","album":null,
         "duration_seconds":199,"image_key":"img-b","lyrics":"unknown"}]})");
    TEST_ASSERT_EQUAL(2, items.size());
    TEST_ASSERT_EQUAL_UINT32(12, items[0].id);
    TEST_ASSERT_EQUAL_STRING("Neon Autumn", items[0].title.c_str());
    TEST_ASSERT_EQUAL_STRING("The Demo Lights", items[0].artist.c_str());
    TEST_ASSERT_EQUAL_STRING("img-a", items[0].imageKey.c_str());
    TEST_ASSERT_EQUAL_UINT32(214, items[0].durationS);
    TEST_ASSERT_EQUAL_UINT32(13, items[1].id);
    TEST_ASSERT_EQUAL_STRING("Harbour Fog", items[1].title.c_str());
}

// Roon leaves lines out (a radio stream has no artist, no length): null reads as empty, not "null".
void test_missing_fields_read_as_empty() {
    auto items = parse(R"({"items":[{"queue_item_id":5,"title":"Live Stream","artist":null,
                                     "duration_seconds":null,"image_key":null}]})");
    TEST_ASSERT_EQUAL(1, items.size());
    TEST_ASSERT_EQUAL_STRING("Live Stream", items[0].title.c_str());
    TEST_ASSERT_EQUAL_STRING("", items[0].artist.c_str());
    TEST_ASSERT_EQUAL_STRING("", items[0].imageKey.c_str());
    TEST_ASSERT_EQUAL_UINT32(0, items[0].durationS);
}

// "Play from here" sends the id back as an integer: an item without one could not be played.
void test_items_without_an_integer_id_are_left_out() {
    auto items = parse(R"({"items":[{"title":"No id"},{"queue_item_id":"x9","title":"Text id"},
                                    {"queue_item_id":-3,"title":"Negative"},
                                    {"queue_item_id":40,"title":"Kept"}]})");
    TEST_ASSERT_EQUAL(1, items.size());
    TEST_ASSERT_EQUAL_UINT32(40, items[0].id);
    TEST_ASSERT_EQUAL_STRING("Kept", items[0].title.c_str());
}

// The music server's own list (tft-core-client's QueueItem: fixed char arrays) gives the screen
// the same items as the companion's body for the same queue: the screen never knows the source.
struct CoreItem {
    uint32_t id = 0, lengthSec = 0;
    char title[128] = {0}, artist[128] = {0}, album[128] = {0}, imageKey[48] = {0};
};

void test_the_servers_own_queue_gives_the_same_items_as_the_companions() {
    std::vector<CoreItem> core(2);
    core[0].id = 12;
    core[0].lengthSec = 214;
    strcpy(core[0].title, "Neon Autumn");
    strcpy(core[0].artist, "The Demo Lights");
    strcpy(core[0].imageKey, "img-a");
    core[1].id = 13;
    strcpy(core[1].title, "Harbour Fog");
    std::vector<Item> direct = {{7, "stale"}};
    net::q::fromCore(core, direct);
    auto companion = parse(R"({"items":[
        {"queue_item_id":12,"title":"Neon Autumn","artist":"The Demo Lights","duration_seconds":214,"image_key":"img-a"},
        {"queue_item_id":13,"title":"Harbour Fog","artist":null,"duration_seconds":null,"image_key":null}]})");
    TEST_ASSERT_EQUAL(companion.size(), direct.size());
    for (size_t i = 0; i < direct.size(); i++) {
        TEST_ASSERT_EQUAL_UINT32(companion[i].id, direct[i].id);
        TEST_ASSERT_EQUAL_STRING(companion[i].title.c_str(), direct[i].title.c_str());
        TEST_ASSERT_EQUAL_STRING(companion[i].artist.c_str(), direct[i].artist.c_str());
        TEST_ASSERT_EQUAL_STRING(companion[i].imageKey.c_str(), direct[i].imageKey.c_str());
        TEST_ASSERT_EQUAL_UINT32(companion[i].durationS, direct[i].durationS);
    }
}

void test_an_error_body_is_no_queue() {
    bool ok = true;
    auto items = parse(R"({"success":false,"error":{"code":"NO_CURRENT_TRACK"}})", &ok);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_EQUAL(0, items.size());
    items = parse(R"({"items":[]})", &ok);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL(0, items.size());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_items_keep_the_companions_order_and_fields);
    RUN_TEST(test_missing_fields_read_as_empty);
    RUN_TEST(test_items_without_an_integer_id_are_left_out);
    RUN_TEST(test_an_error_body_is_no_queue);
    RUN_TEST(test_the_servers_own_queue_gives_the_same_items_as_the_companions);
    return UNITY_END();
}
