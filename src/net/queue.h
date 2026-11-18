// The followed zone's play queue (boards with HAS_QUEUE), from whichever source is in use.
// Net task only.
//
// Companion (src/api/roonRoutes.js): GET /api/roon/queue?zone_id=, polled every 5 s while the
// screen is open, and POST /api/roon/queue/play { queue_item_id, zone_id }. Direct: the server's
// subscribe_queue, pushed (tft-core-client followQueue), followed only while the screen is open,
// and play_from_here. Either way the panel only plays from an item: neither offers a way to
// remove, move or clear items.
#pragma once

#include "net.h"

namespace net::q {

void run(QueueOp op, uint32_t itemId);   // a queued UI command
void tick();                       // refreshes the queue and loads thumbnails while open

}  // namespace net::q
