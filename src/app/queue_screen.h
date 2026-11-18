// The followed zone's play queue on the panel (boards with HAS_QUEUE, either source): what is
// playing and what comes next; a tap plays from that item. Opened from the controls bar. The net
// task loads it (net/queue).
#pragma once

#include <lvgl.h>

#include "../net/net.h"

namespace queueui {

void build(lv_obj_t *scr);
void open();
void close();
bool isOpen();
void refresh(const net::Snapshot &s);   // new items, thumbnails, messages, followed zone
void tick(uint32_t now);                // hides the message after a while

// Showcase: an invented queue, every row with the demo cover.
void showcase(const std::shared_ptr<const Picture> &thumb);

}  // namespace queueui
