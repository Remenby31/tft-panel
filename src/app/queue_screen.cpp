#include "queue_screen.h"

#include <Arduino.h>

#include <vector>

#include "../layout.h"
#include "fonts.h"

namespace queueui {
namespace {

using layout::S;
using layout::SCREEN_H;
using layout::SCREEN_W;
using layout::X;
using layout::Y;

constexpr uint32_t COL_BG = 0x0B0B0C, COL_TEXT = 0xF2F2F2, COL_MID = 0xAEAEB2, COL_DIM = 0x78787C;
constexpr uint32_t COL_ACCENT = 0x6DB56D, COL_ERROR = 0xFFB0A0, COL_BUTTON = 0x232326, COL_ROW = 0x18181B;
constexpr uint32_t MESSAGE_MS = 5000;

lv_obj_t *g_root = nullptr, *g_zone = nullptr, *g_list = nullptr, *g_info = nullptr, *g_toast = nullptr;
// Function-local, as in net/queue.cpp: a board without HAS_QUEUE drops them at link time.
std::vector<lv_obj_t *> &thumbs() {   // one per item, at most 20 (the companion's limit)
    static std::vector<lv_obj_t *> v;
    return v;
}
std::shared_ptr<const net::QueuePage> &page() {
    static std::shared_ptr<const net::QueuePage> p;
    return p;
}
uint32_t g_seenSerial = UINT32_MAX, g_shownId = 0, g_shownMessageAt = 0, g_toastAt = 0;
bool g_demo = false;

lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, uint32_t colour, const char *text = "") {
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(colour), 0);
    lv_label_set_text(l, text);
    return l;
}

void onRow(lv_event_t *e) {
    if (g_demo) return;
    net::queue(net::QueueOp::Play, (uint32_t)(uintptr_t)lv_event_get_user_data(e));
}

// The playing track (index 0) is shown, not offered: playing from it would restart it.
void addRow(size_t i) {
    const net::QueueItem &it = page()->items[i];
    bool current = i == 0;
    lv_obj_t *b = current ? lv_obj_create(g_list) : lv_button_create(g_list);
    lv_obj_set_scrollable(b, false);
    lv_obj_set_size(b, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_radius(b, S(10), 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_set_style_border_width(b, current ? S(2) : 0, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(COL_ACCENT), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(COL_ROW), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(0x2C2C30), LV_STATE_PRESSED);
    lv_obj_set_style_pad_all(b, S(8), 0);
    lv_obj_set_style_pad_column(b, X(14), 0);
    lv_obj_set_flex_flow(b, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(b, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *img = lv_image_create(b);
    lv_obj_set_size(img, net::COVER_THUMB_PX, net::COVER_THUMB_PX);
    lv_image_set_inner_align(img, LV_IMAGE_ALIGN_CENTER);
    lv_obj_set_style_bg_color(img, lv_color_hex(COL_BUTTON), 0);
    lv_obj_set_style_bg_opa(img, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(img, S(6), 0);
    lv_obj_set_style_clip_corner(img, true, 0);
    if (it.thumb) lv_image_set_src(img, &it.thumb->dsc);
    thumbs().push_back(img);

    lv_obj_t *texts = lv_obj_create(b);
    lv_obj_remove_style_all(texts);
    lv_obj_set_flex_grow(texts, 1);
    lv_obj_set_height(texts, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(texts, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_clickable(texts, false);
    lv_obj_t *t = label(texts, fonts::ui20, COL_TEXT, it.title.c_str());
    lv_obj_set_width(t, lv_pct(100));
    lv_label_set_long_mode(t, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_t *st = label(texts, fonts::ui16, COL_DIM, it.artist.c_str());
    lv_obj_set_width(st, lv_pct(100));
    lv_label_set_long_mode(st, LV_LABEL_LONG_MODE_DOTS);

    if (it.durationS) {
        char d[12];
        snprintf(d, sizeof d, "%u:%02u", (unsigned)(it.durationS / 60), (unsigned)(it.durationS % 60));
        label(b, fonts::ui16, COL_DIM, d);
    }
    if (current) return;
    label(b, fonts::icons, COL_ACCENT, LV_SYMBOL_PLAY);
    lv_obj_add_event_cb(b, onRow, LV_EVENT_CLICKED, (void *)(uintptr_t)it.id);
}

void fillRows() {
    lv_obj_clean(g_list);
    thumbs().clear();
    for (size_t i = 0; i < page()->items.size(); i++) {
        if (i <= 1) label(g_list, fonts::ui16, COL_ACCENT, i ? "Up next" : "Now playing");
        addRow(i);
    }
    lv_obj_scroll_to_y(g_list, 0, LV_ANIM_OFF);
}

void updateThumbs() {
    for (size_t i = 0; i < thumbs().size() && i < page()->items.size(); i++)
        if (page()->items[i].thumb && lv_image_get_src(thumbs()[i]) != &page()->items[i].thumb->dsc)
            lv_image_set_src(thumbs()[i], &page()->items[i].thumb->dsc);
}

void showToast(const String &msg, bool error) {
    lv_label_set_text(g_toast, msg.c_str());
    lv_obj_set_style_text_color(g_toast, lv_color_hex(error ? COL_ERROR : COL_TEXT), 0);
    lv_obj_set_hidden(g_toast, false);
    lv_obj_move_foreground(g_toast);
    g_toastAt = millis();
}

void applyPage() {
    if (!page()) return;
    if (page()->id != g_shownId) {
        g_shownId = page()->id;
        fillRows();
    } else {
        updateThumbs();
    }
    if (page()->messageAt != g_shownMessageAt && page()->message.length()) {
        g_shownMessageAt = page()->messageAt;
        showToast(page()->message, page()->error);
    }
    const char *info = page()->loading ? "Loading..."
                       : page()->items.empty() && !page()->failed ? "Nothing in the queue"
                                                                 : "";
    lv_label_set_text(g_info, info);
    lv_obj_set_hidden(g_info, !*info);
}

void setZone(const String &name) {
    String zone = name.length() ? name : String("No zone: pick one in the controls bar");
    if (strcmp(lv_label_get_text(g_zone), zone.c_str()) != 0) lv_label_set_text(g_zone, zone.c_str());
}

}  // namespace

void build(lv_obj_t *scr) {
    g_root = lv_obj_create(scr);
    lv_obj_remove_style_all(g_root);
    lv_obj_set_size(g_root, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(g_root, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(g_root, LV_OPA_COVER, 0);
    lv_obj_set_scrollable(g_root, false);
    lv_obj_set_clickable(g_root, true);   // modal

    lv_obj_t *h = label(g_root, fonts::lyricsOther, COL_TEXT, "Queue");
    lv_obj_set_pos(h, X(32), Y(20));
    lv_obj_t *x = lv_button_create(g_root);
    lv_obj_set_size(x, S(56), S(56));
    lv_obj_set_style_radius(x, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(x, 0, 0);
    lv_obj_set_style_bg_color(x, lv_color_hex(COL_BUTTON), 0);
    lv_obj_set_style_bg_color(x, lv_color_hex(COL_ACCENT), LV_STATE_PRESSED);
    lv_obj_center(label(x, fonts::icons, COL_TEXT, LV_SYMBOL_CLOSE));
    lv_obj_add_event_cb(x, [](lv_event_t *) { close(); }, LV_EVENT_CLICKED, nullptr);
    lv_obj_align(x, LV_ALIGN_TOP_RIGHT, -X(20), Y(10));
    g_zone = label(g_root, fonts::ui16, COL_DIM);
    lv_obj_set_width(g_zone, X(300));
    lv_label_set_long_mode(g_zone, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_style_text_align(g_zone, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(g_zone, LV_ALIGN_TOP_RIGHT, -X(20) - S(56) - X(14), Y(28));

    g_list = lv_obj_create(g_root);
    lv_obj_remove_style_all(g_list);
    lv_obj_set_size(g_list, SCREEN_W - 2 * X(20), SCREEN_H - Y(96));
    lv_obj_set_pos(g_list, X(20), Y(88));
    lv_obj_set_flex_flow(g_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(g_list, Y(6), 0);
    lv_obj_set_style_pad_bottom(g_list, Y(16), 0);
    lv_obj_set_scroll_dir(g_list, LV_DIR_VER);

    g_info = label(g_root, fonts::ui20, COL_MID);
    lv_obj_align(g_info, LV_ALIGN_CENTER, 0, Y(20));
    lv_obj_set_hidden(g_info, true);

    g_toast = label(g_root, fonts::ui20, COL_TEXT);
    lv_obj_set_style_bg_color(g_toast, lv_color_hex(0x202127), 0);
    lv_obj_set_style_bg_opa(g_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g_toast, S(12), 0);
    lv_obj_set_style_pad_hor(g_toast, X(20), 0);
    lv_obj_set_style_pad_ver(g_toast, Y(12), 0);
    lv_obj_set_style_max_width(g_toast, X(700), 0);
    lv_label_set_long_mode(g_toast, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_align(g_toast, LV_ALIGN_BOTTOM_MID, 0, -Y(20));
    lv_obj_set_hidden(g_toast, true);

    lv_obj_set_hidden(g_root, true);
}

void open() {
    if (g_demo) {   // leaving the showcase: the next refresh brings the real queue
        g_demo = false;
        page() = nullptr;
        g_shownId = 0;
        g_seenSerial = UINT32_MAX;
        lv_obj_clean(g_list);
        thumbs().clear();
        refresh(net::snapshot());
    }
    net::queue(net::QueueOp::Open);
    lv_obj_set_hidden(g_root, false);
    lv_obj_move_foreground(g_root);
    applyPage();
}

void close() {
    if (!g_root) return;
    lv_obj_set_hidden(g_root, true);
    if (!g_demo) net::queue(net::QueueOp::Close);
}

bool isOpen() { return g_root && !lv_obj_is_hidden(g_root); }

void refresh(const net::Snapshot &s) {
    if (!g_root || g_demo) return;
    setZone(s.zoneName);
    if (s.queueSerial == g_seenSerial) return;
    g_seenSerial = s.queueSerial;
    page() = s.queue;
    if (isOpen()) applyPage();
}

void tick(uint32_t now) {
    if (g_toast && !lv_obj_is_hidden(g_toast) && now - g_toastAt > MESSAGE_MS) lv_obj_set_hidden(g_toast, true);
}

void showcase(const std::shared_ptr<const Picture> &thumb) {
    auto demo = std::make_shared<net::QueuePage>();
    demo->id = 0x5EED;
    struct {
        const char *title, *artist;
        uint32_t s;
    } tracks[] = {{"Neon Autumn", "The Demo Lights", 214},       {"City Lights at Two", "The Demo Lights", 187},
                  {"Radio Static Heart", "The Demo Lights", 241}, {"Harbour Fog", "Paper Satellites", 199},
                  {"Slow Train Home", "The Demo Lights", 263},    {"Paper Moon Avenue", "Marigold Static", 176},
                  {"The Last Arcade", "The Demo Lights", 305}};
    uint32_t id = 101;
    for (const auto &t : tracks) {
        net::QueueItem it;
        it.id = id++;
        it.title = t.title;
        it.artist = t.artist;
        it.durationS = t.s;
        it.thumb = thumb;
        demo->items.push_back(it);
    }
    g_demo = true;
    page() = demo;
    g_shownId = 0;
    lv_label_set_text(g_zone, "Living Room");
    lv_obj_set_hidden(g_root, false);
    lv_obj_move_foreground(g_root);
    applyPage();
}

}  // namespace queueui
