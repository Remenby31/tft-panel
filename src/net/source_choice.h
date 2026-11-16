// Which source a panel uses when nobody chose one: it looks for both and takes what it finds.
//
// Header-only, its words from the string table, so the host tests (test/test_source_choice) call it
// as net.cpp does.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../app/i18n.h"

namespace settings {

// Where the panel gets what it shows. The companion reads the lyrics in the listener's own
// files; Direct talks to the music server itself. Auto looks for both at every start.
// Stored in NVS as this number.
enum class SourceKind : uint8_t { Companion = 0, Direct = 1, Auto = 2 };

}  // namespace settings

namespace sourcechoice {

using settings::SourceKind;

// The words used for it on the console, the setup portal and the web page.
inline const char *name(SourceKind k) {
    return k == SourceKind::Direct ? "direct" : k == SourceKind::Auto ? "auto" : "companion";
}
inline SourceKind parse(const char *word) {
    if (!strcmp(word, "direct") || !strcmp(word, "roon")) return SourceKind::Direct;
    if (!strcmp(word, "auto")) return SourceKind::Auto;
    return SourceKind::Companion;
}

// The setting as read from NVS. With none stored, this firmware is starting for the first time:
// a panel that already has Wi-Fi was running on its companion, so it keeps it (and the caller
// writes it down), because an update must never move a working panel to a source that waits for
// an approval. Only a new panel looks by itself.
inline SourceKind fromStored(bool present, uint8_t value, bool wifiStored) {
    if (!present) return wifiStored ? SourceKind::Companion : SourceKind::Auto;
    if (value == (uint8_t)SourceKind::Direct) return SourceKind::Direct;
    if (value == (uint8_t)SourceKind::Auto) return SourceKind::Auto;
    return SourceKind::Companion;
}

// After a companion answers, Auto waits this long for a music server too: the server's
// discovery repeats every 10 s, so this covers a second try.
constexpr uint32_t GRACE_MS = 12000;

struct Probe {
    bool companion = false;       // a companion answered (or one is configured by address)
    bool server = false;          // a music server answered its discovery (or one is configured)
    uint32_t companionAtMs = 0;   // when the companion first answered
};

enum class Pick : uint8_t { Companion, Direct, Looking };

// A choice made on purpose always wins. Otherwise a music server means Direct, for playback
// (lyrics then come from a companion if there is one, through the lyrics setting's default);
// a companion alone is taken once the grace is over; nothing found means look again.
inline Pick choose(SourceKind setting, const Probe &p, uint32_t nowMs) {
    if (setting == SourceKind::Companion) return Pick::Companion;
    if (setting == SourceKind::Direct) return Pick::Direct;
    if (p.server) return Pick::Direct;
    if (p.companion && nowMs - p.companionAtMs >= GRACE_MS) return Pick::Companion;
    return Pick::Looking;
}

// The status line for an automatic choice, in words for someone who has never heard of either.
// `companionLyrics`: the lyrics setting is left to find a companion, and one was found that way.
inline void describe(Pick pick, const Probe &p, bool companionLyrics, char *out, size_t size) {
    switch (pick) {
        case Pick::Looking:
            if (p.companion)
                snprintf(out, size, "%s", tr(Str::AutoFoundCompanionLooking));
            else
                snprintf(out, size, "%s", tr(Str::AutoLooking));
            break;
        case Pick::Companion:
            snprintf(out, size, "%s", tr(Str::AutoCompanion));
            break;
        case Pick::Direct:
            if (companionLyrics)
                snprintf(out, size, "%s", tr(Str::AutoDirectWithLyrics));
            else
                snprintf(out, size, "%s", tr(Str::AutoDirect));
            break;
    }
}

}  // namespace sourcechoice
