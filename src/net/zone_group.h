// Which outputs to send for grouping and ungrouping zones (companion POST /api/roon/group and
// /ungroup, `output_ids` from its zone list). Pure, so the host tests build it.
#pragma once

#include <Arduino.h>

#include <algorithm>
#include <vector>

namespace net::group {

// Joins `guest` to `host`: the host's outputs first, because Roon keeps the first output's queue,
// so what the followed zone plays goes on and the guest starts playing it too. Empty when there is
// nothing to do: a zone without outputs, or a guest already in the host's group.
inline std::vector<String> joinOutputs(const std::vector<String> &host, const std::vector<String> &guest) {
    if (host.empty() || guest.empty()) return {};
    std::vector<String> ids = host;
    for (const String &g : guest)
        if (std::find(host.begin(), host.end(), g) == host.end()) ids.push_back(g);
    if (ids.size() == host.size()) return {};
    return ids;
}

// Splits a grouped zone back into one zone per output. Empty when the zone is not a group.
inline std::vector<String> splitOutputs(const std::vector<String> &zone) {
    return zone.size() > 1 ? zone : std::vector<String>{};
}

}  // namespace net::group
