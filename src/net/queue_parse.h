// A zone's queue into items, in its order, the playing track first: from the companion's
// GET /api/roon/queue body, or from the music server's own list. Generic over the item
// (net::QueueItem on the panel) so the host tests build it without LVGL or the network.
#pragma once

#include <ArduinoJson.h>

namespace net::q {

// Items without an integer queue_item_id cannot be played from and are left out.
// false = the body has no item list at all.
template <class Items>
bool parse(JsonVariantConst body, Items &out) {
    out.clear();
    JsonArrayConst items = body["items"].as<JsonArrayConst>();
    if (items.isNull()) return false;
    for (JsonObjectConst it : items) {
        JsonVariantConst id = it["queue_item_id"];
        if (!id.is<uint32_t>()) continue;
        typename Items::value_type q;
        q.id = id.as<uint32_t>();
        q.title = it["title"] | "";
        q.artist = it["artist"] | "";
        q.imageKey = it["image_key"] | "";
        q.durationS = it["duration_seconds"] | 0u;
        out.push_back(std::move(q));
    }
    return true;
}

// The music server's own queue (tft-core-client's QueueList, direct source) into the same items,
// so the screen and the play command never know which source filled them.
template <class CoreList, class Items>
void fromCore(const CoreList &in, Items &out) {
    out.clear();
    for (const auto &c : in) {
        typename Items::value_type q;
        q.id = c.id;
        q.title = c.title;
        q.artist = c.artist;
        q.imageKey = c.imageKey;
        q.durationS = c.lengthSec;
        out.push_back(std::move(q));
    }
}

}  // namespace net::q
