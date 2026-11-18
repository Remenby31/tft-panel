// Which source a panel uses when nobody chose one: it looks for both and takes what it finds.
//
// Pure and header-only, so the host tests (test/test_source_choice) call it as net.cpp does.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

// A panel paired with its server before waits this long, once a companion answers, for the server
// to take its stored pairing back: otherwise every start would play a few seconds through the
// companion and then switch. The server's discovery repeats every 10 s, so this covers a second try.
constexpr uint32_t PAIRED_WAIT_MS = 12000;
// The source in use stopped answering and the other one is there: this long before switching.
constexpr uint32_t FALLBACK_MS = 15000;
// After falling back to the companion, the server is not taken again for this long, so a server
// that comes and goes cannot make the panel switch back and forth.
constexpr uint32_t HOLD_MS = 60000;

// What answers right now. `approved`: the server accepted this panel and sends its zones.
struct Seen {
    bool companion = false;
    bool server = false;
    bool approved = false;
};

enum class Active : uint8_t { Looking, Companion, Direct };

// Which source plays, step by step as things answer or stop answering. A choice made by hand is
// fixed. Auto plays through a companion as soon as one answers, keeps the server's pairing going
// behind it, and moves to the server once it accepts the panel: waiting for that approval never
// keeps the music off.
class Chooser {
  public:
    Chooser(SourceKind setting, bool pairedBefore) : setting_(setting), pairedBefore_(pairedBefore) {
        if (setting == SourceKind::Companion) active_ = Active::Companion;
        if (setting == SourceKind::Direct) active_ = Active::Direct;
    }

    Active step(const Seen &seen, uint32_t nowMs) {
        seen_ = seen;
        if (setting_ != SourceKind::Auto) return active_;
        if (seen.companion && !companionSeen_) {
            companionSeen_ = true;
            companionSince_ = nowMs;
        }
        switch (active_) {
            case Active::Looking:
                if (seen.approved) take(Active::Direct);
                else if (seen.companion && (!pairedBefore_ || nowMs - companionSince_ >= PAIRED_WAIT_MS))
                    take(Active::Companion);
                else if (seen.server && !seen.companion) take(Active::Direct);
                break;
            case Active::Direct:
                if (seen.approved || !seen.companion) {
                    losing_ = false;
                    approvedHere_ = approvedHere_ || seen.approved;
                } else if (!approvedHere_ && !pairedBefore_) {
                    take(Active::Companion);   // still waiting for an approval: music first
                } else if (lost(nowMs)) {
                    take(Active::Companion);
                    held_ = true;
                    heldSince_ = nowMs;
                }
                break;
            case Active::Companion:
                if (seen.approved && (!held_ || nowMs - heldSince_ >= HOLD_MS)) {
                    take(Active::Direct);
                } else if (!seen.companion && seen.server) {
                    if (lost(nowMs)) take(Active::Direct);
                } else {
                    losing_ = false;
                }
                break;
        }
        return active_;
    }

    Active active() const { return active_; }
    // Auto, playing through the companion, a server answered and has not accepted the panel yet.
    bool awaitingApproval() const {
        return setting_ == SourceKind::Auto && active_ == Active::Companion && seen_.server && !seen_.approved;
    }

  private:
    void take(Active a) {
        active_ = a;
        losing_ = false;
        approvedHere_ = a == Active::Direct && seen_.approved;
    }
    bool lost(uint32_t nowMs) {
        if (!losing_) {
            losing_ = true;
            lostSince_ = nowMs;
        }
        return nowMs - lostSince_ >= FALLBACK_MS;
    }

    SourceKind setting_;
    bool pairedBefore_;
    Active active_ = Active::Looking;
    Seen seen_;
    bool companionSeen_ = false, losing_ = false, held_ = false, approvedHere_ = false;
    uint32_t companionSince_ = 0, lostSince_ = 0, heldSince_ = 0;
};

// The line that says what the automatic choice found, for someone who has never heard of either.
// `companionLyrics`: the lyrics setting is left to find a companion, and one was found that way.
inline void describe(const Chooser &c, const Seen &seen, bool companionLyrics, const char *panelName, char *out,
                     size_t size) {
    switch (c.active()) {
        case Active::Looking:
            if (seen.companion)
                snprintf(out, size, "Found a TextFromTrack companion. Checking for a music server too...");
            else
                snprintf(out, size, "Looking for your music server, or a TextFromTrack companion, on this Wi-Fi...");
            break;
        case Active::Companion:
            if (c.awaitingApproval())
                snprintf(out, size, "Playing through the TextFromTrack companion. To play straight from your "
                                    "music server, enable \"%s\" in its Settings > Extensions.", panelName);
            else
                snprintf(out, size, "Found a TextFromTrack companion: music and lyrics come from it.");
            break;
        case Active::Direct:
            if (companionLyrics)
                snprintf(out, size, "Found your music server and a TextFromTrack companion: music from the "
                                    "server, lyrics from the companion.");
            else
                snprintf(out, size, "Found your music server: music comes from it directly.");
            break;
    }
}

}  // namespace sourcechoice
