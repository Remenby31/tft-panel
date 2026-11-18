// The followed zone's play queue through the companion (boards with HAS_QUEUE). Net task only.
//
// Contract (companion src/api/roonRoutes.js): GET /api/roon/queue?zone_id= and
// POST /api/roon/queue/play { queue_item_id, zone_id }, always with the followed zone. The
// companion has no route to remove, move or clear items, so the panel only plays from one.
#pragma once

#include "net.h"

namespace net::q {

void run(QueueOp op, uint32_t itemId);   // a queued UI command
void tick();                       // refreshes the queue and loads thumbnails while open

}  // namespace net::q
