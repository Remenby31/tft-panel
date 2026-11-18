// GET /api/roon/queue's body into items, in its order: the playing track first. Generic over the
// item (net::QueueItem on the panel) so the host tests build it without LVGL or the network.
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

}  // namespace net::q
