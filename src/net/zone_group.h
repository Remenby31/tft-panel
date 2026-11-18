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

// Whether `id` is one of the space-separated ids in `list`.
inline bool listed(const String &list, const String &id) {
    if (!id.length()) return false;
    for (int at = 0; at < (int)list.length();) {
        int end = list.indexOf(' ', at);
        if (end < 0) end = list.length();
        if (end - at == (int)id.length() && list.substring(at, end) == id) return true;
        at = end + 1;
    }
    return false;
}

// What a whole zone can group with: the ids every one of its outputs lists (each output's
// can_group_with_output_ids, space-separated), since an output joins a group only with outputs
// that list it. The first output's list, minus what any other output leaves out.
template <class Lists>
String groupableByAll(const Lists &perOutput) {
    String out;
    bool first = true;
    for (const String &mine : perOutput) {
        if (first) {
            out = mine;
            first = false;
            continue;
        }
        String kept;
        for (int at = 0; at < (int)out.length();) {
            int end = out.indexOf(' ', at);
            if (end < 0) end = out.length();
            String id = out.substring(at, end);
            if (listed(mine, id)) kept += (kept.length() ? " " : "") + id;
            at = end + 1;
        }
        out = kept;
    }
    return out;
}

// Whether to offer "+ Add to <host>" on `guest`: every one of its outputs must be one the host can
// group with (`hostGroupable`, from groupableByAll). A source that does not say (`known` false: a
// companion that does not pass the field on) offers every zone, and a refusal shows if it comes.
inline bool canJoin(const String &hostGroupable, bool known, const std::vector<String> &guest) {
    if (guest.empty()) return false;
    if (!known) return true;
    for (const String &g : guest)
        if (!listed(hostGroupable, g)) return false;
    return true;
}

// Splits a grouped zone back into one zone per output. Empty when the zone is not a group.
inline std::vector<String> splitOutputs(const std::vector<String> &zone) {
    return zone.size() > 1 ? zone : std::vector<String>{};
}

}  // namespace net::group
