// Talking to the music server itself, with no companion in between (docs/ARCHITECTURE.md).
//
// Where the companion polls, this subscribes: the server pushes the zones and the playing
// position once a second, so the position is known more precisely and the network carries a
// twentieth of the traffic. What it cannot do is lyrics, which are the companion's whole job.
#pragma once

#if defined(SOURCE_ROON)

#include <tftcore/client.h>

#include "source.h"

namespace net {

class DirectSource : public Source {
  public:
    const char *name() const override { return "direct"; }
    // No lyrics at all: nothing here reads the music files or resolves anything.
    Capabilities caps() const override { return {false, false, board::HAS_QUEUE || BOARD_HAS_GROUPING}; }

    bool ready() const override;
    bool connect(uint32_t now) override;
    void reset() override;
    void pump() override;

    // For the automatic choice (source_choice.h): a server answered, and giving up on it.
    bool serverFound() const;
    bool awaitingApproval() const;
    // Made the source in use while already connected behind another one: publishes what it has.
    void bringForward();

    // Nothing is polled: the server pushes, and pump() publishes what arrives.
    bool zonesDue(uint32_t) const override { return false; }
    bool refreshZones() override { return true; }
    bool nowPlayingDue(uint32_t) const override { return false; }
    void pollNowPlaying() override {}
    void zoneSelectionChanged() override;

    void command(const Command &c) override;
    // No lyrics of its own: a music server does not hand them to an extension. With a lyrics
    // server configured (an LRCLIB-compatible one, a mirror on the network or the public site)
    // it looks them up itself; without one it says there are none, and is not asked again.
    bool fetchLyrics(const String &zoneId) override;

    internal::HttpResult browse(JsonDocument &body, JsonDocument &resp) override;
    internal::HttpResult browseLoad(JsonDocument &body, JsonDocument &resp) override;
    String imageUrl(const String &imageKey, int px) const override;

    // The queue is subscribed to: the server pushes it, at most 20 items, kept in PSRAM by the
    // client. Grouping goes by the server's own can_group_with_output_ids.
    internal::HttpResult queue(const String &zoneId, QueueItems &out) override;
    uint32_t queueSerial() const override { return queueSerial_; }
    void queueClosed() override;
    internal::HttpResult playFromHere(const String &zoneId, uint32_t queueItemId) override;
#if BOARD_HAS_GROUPING
    internal::HttpResult groupOutputs(const std::vector<String> &outputIds, bool group) override;
#endif

  private:
    void begin();
    void publishEverything(const tftcore::ZoneList &zones, const tftcore::ZoneDelta &delta);
    internal::HttpResult call(bool load, JsonDocument &body, JsonDocument &resp);

    static void onZones(void *ctx, const tftcore::ZoneList &zones, const tftcore::ZoneDelta &delta);
    static void onToken(void *ctx, const char *serverId, const char *token);
    static void onState(void *ctx, tftcore::State state, const char *detail);
    static void onQueue(void *ctx, const char *zoneOrOutputId, const tftcore::QueueList &queue);

    bool started_ = false;
    bool healthy_ = false;      // the first full zone list confirms a freshly installed image
    String trackKey_;           // to spot a track change before publishing
    uint32_t queueSerial_ = 0;  // bumped by every queue push
    String queueFor_;           // the zone of the last queue push
    uint32_t queueAskedAt_ = 0;
    static constexpr uint32_t QUEUE_ANSWER_MS = 8000;   // a queue not sent by then is asked again
};

}  // namespace net

#endif  // SOURCE_ROON
