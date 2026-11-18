// Which zones to offer for grouping, and which outputs to send to group or ungroup them (companion
// POST /api/roon/group and /ungroup, or the server's group_outputs / ungroup_outputs, with
// `output_ids` from the zone list). Pure, so the host tests build it.
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

// Whether to offer "+ Add to <host>" on `guest`: every one of its outputs must be one the host's
// outputs can share a group with, as the music server says (can_group_with_output_ids). A source
// that does not say (`known` false: a companion that does not pass the field on) offers every
// zone, and the server's refusal shows if it comes.
inline bool canJoin(const std::vector<String> &hostGroupable, bool known, const std::vector<String> &guest) {
    if (guest.empty()) return false;
    if (!known) return true;
    for (const String &g : guest)
        if (std::find(hostGroupable.begin(), hostGroupable.end(), g) == hostGroupable.end()) return false;
    return true;
}

// Splits a grouped zone back into one zone per output. Empty when the zone is not a group.
inline std::vector<String> splitOutputs(const std::vector<String> &zone) {
    return zone.size() > 1 ? zone : std::vector<String>{};
}

}  // namespace net::group
