#include "queue.h"

#include <esp_heap_caps.h>

#include <algorithm>

#include "../app/cover.h"
#include "../app/i18n.h"
#include "net_internal.h"
#include "source.h"

namespace net::q {
namespace {

using namespace net::internal;

constexpr uint32_t REFRESH_MS = 5000;   // the queue moves on as tracks end, or from another remote
constexpr uint32_t ASKED_RETRY_MS = 300;   // a pushed queue that is on its way

// The two with destructors are function-local: a board without HAS_QUEUE never calls in, and
// the linker then drops them with everything else here.
std::shared_ptr<QueuePage> &page() {   // working copy; published as a const copy
    static std::shared_ptr<QueuePage> p;
    return p;
}
std::vector<PStr, PsramAllocator<PStr>> &failedThumbs() {
    static std::vector<PStr, PsramAllocator<PStr>> keys;
    return keys;
}
bool g_open = false;
uint32_t g_loadedAt = 0;                // millis() of the last answer, 0 = load now
uint32_t g_nextId = 1;
uint32_t g_loadedSerial = 0;            // source().queueSerial() at the last load

void publish() { publishQueue(std::make_shared<const QueuePage>(*page())); }

void setMessage(const String &m, bool error) {
    page()->message = m;
    page()->error = error;
    page()->messageAt = millis();
    Serial.printf("[queue] %s%s\n", error ? "ERROR: " : "", m.c_str());
}

String errorText(const HttpResult &r) {
    if (r.code == 401) return tr(Str::LibErrToken);
    if (r.code == 503) return tr(Str::LibErrNoCore);
    return r.message.length() ? r.message : "HTTP " + String(r.code);
}

bool sameItems(const QueueItems &a, const QueueItems &b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](const QueueItem &x, const QueueItem &y) { return x.id == y.id; });
}

void load() {
    String zone = followedZoneId();
    g_loadedAt = millis() ? millis() : 1;
    page()->loading = false;
    g_loadedSerial = source().queueSerial();
    QueueItems items;
    bool failed = false;
    if (zone.length()) {
        HttpResult r = source().queue(zone, items);
        if (r.code == 202) {   // asked for and on its way (pushed)
            if (zone != page()->zoneId) {   // another zone's list must not take a tap meant for this one
                page()->items.clear();
                page()->zoneId = zone;
                page()->id = g_nextId++;
            }
            page()->loading = page()->items.empty();
            g_loadedAt = millis() - REFRESH_MS + ASKED_RETRY_MS;
            if (!g_loadedAt) g_loadedAt = 1;
            publish();
            return;
        }
        // 404: the zone has nothing loaded, which is an empty queue rather than a failure.
        failed = r.code != 200 && r.code != 404;
        if (failed && !page()->failed)   // once, not at every refresh
            setMessage(i18n::fmt(Str::QueueCannotLoad, errorText(r).c_str()), true);
    }
    page()->failed = failed;
    // A failure keeps the list, unless it is another zone's: a tap must play where the screen says.
    if (failed && zone == page()->zoneId) {
        publish();
        return;
    }
    if (zone != page()->zoneId || !sameItems(items, page()->items)) {
        for (QueueItem &i : items)
            for (const QueueItem &old : page()->items)
                if (old.thumb && old.imageKey == i.imageKey) i.thumb = old.thumb;
        page()->items = std::move(items);
        page()->zoneId = zone;
        page()->id = g_nextId++;
    }
    publish();
}

void play(uint32_t itemId) {
    auto it = std::find_if(page()->items.begin(), page()->items.end(),
                           [&](const QueueItem &i) { return i.id == itemId; });
    if (it == page()->items.end()) return;   // gone from the queue meanwhile: the screen redraws
    HttpResult r = source().playFromHere(page()->zoneId, itemId);
    if (r.code == 200) setMessage(i18n::fmt(Str::QueuePlaying, it->title.c_str()), false);
    else setMessage(i18n::fmt(Str::QueueCannotPlay, errorText(r).c_str()), true);
    g_loadedAt = millis() - REFRESH_MS + 1000;   // Roon moves the queue on: look again in a second
    if (!g_loadedAt) g_loadedAt = 1;
    publish();
}

// Loads the next missing thumbnail, one per call so the net task stays responsive.
void nextThumb() {
    PStr key;
    for (const QueueItem &i : page()->items)
        if (!i.thumb && i.imageKey.length() &&
            std::find(failedThumbs().begin(), failedThumbs().end(), i.imageKey) == failedThumbs().end()) {
            key = i.imageKey;
            break;
        }
    if (!key.length()) return;
    uint8_t *jpeg = nullptr;
    size_t len = 0;
    HttpResult r = getBytes(source().imageUrl(String(key.c_str()), COVER_THUMB_PX), jpeg, len, 64 * 1024, 4000);
    cover::Cover c;
    String why;
    bool ok = r.code == 200 && jpeg && cover::build(jpeg, len, false, COVER_THUMB_PX, false, 0, 0, c, why) && c.thumb;
    if (jpeg) heap_caps_free(jpeg);
    if (!ok) {
        failedThumbs().push_back(key);
        if (failedThumbs().size() > 40) failedThumbs().erase(failedThumbs().begin());
        return;
    }
    for (QueueItem &i : page()->items)
        if (i.imageKey == key) i.thumb = c.thumb;
    publish();
}

}  // namespace

void run(QueueOp op, uint32_t itemId) {
    if (!page()) page() = std::make_shared<QueuePage>();
    switch (op) {
    case QueueOp::Open:
        g_open = true;
        page()->loading = page()->items.empty();
        publish();
        load();
        break;
    case QueueOp::Close:
        g_open = false;
        source().queueClosed();
        break;
    case QueueOp::Play:
        play(itemId);
        break;
    }
}

void tick() {
    if (!g_open || !page() || !source().ready()) return;
    if (!g_loadedAt || millis() - g_loadedAt >= REFRESH_MS || followedZoneId() != page()->zoneId ||
        source().queueSerial() != g_loadedSerial)
        load();
    else nextThumb();
}

}  // namespace net::q
