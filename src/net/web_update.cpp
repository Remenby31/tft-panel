#include "web_update.h"

#include "lyrics_source.h"
#include <mbedtls/sha256.h>

#include "news.h"
#include "weather.h"

#include <Update.h>
#include <WebServer.h>

#include <WiFi.h>

#include <esp_heap_caps.h>
#include <esp_ota_ops.h>

#include "../app/i18n.h"
#include "../board/board.h"
#include "../board/board_pins.h"
#include "../version.h"
#include "net.h"
#include "ota.h"
#include "portal.h"
#include "settings.h"
#include "signature.h"

namespace webupdate {
namespace {

constexpr uint32_t CODE_VALID_MS = 5 * 60 * 1000;

WebServer *g_server = nullptr;
volatile uint32_t g_code = 0;
volatile bool g_shots = false;
uint32_t g_codeAt = 0;
bool g_uploadAllowed = false, g_uploadOk = false, g_uploadWrongCode = false;
String g_uploadError;

// Text in `%Pg...%` tokens comes from the string table (i18n::fillPage), filled before any value.
const char SETTINGS_PAGE[] PROGMEM = R"HTML(<!doctype html><html lang="%LANG%"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1"><title>%NAME%</title>
<style>body{font:17px/1.5 system-ui,sans-serif;max-width:560px;margin:32px auto;padding:0 16px;background:#15161a;color:#f6f4ee}
h1{margin-bottom:2px}.who{color:#8a8b87;font-size:14px;margin-top:0}
label{display:block;margin-top:18px}small{color:#8a8b87}
input,select,button{font:inherit;width:100%;box-sizing:border-box;padding:10px;border-radius:8px;
border:1px solid #2f3037;background:#202127;color:#f6f4ee;margin-top:4px}
button{background:#6db56d;color:#15161a;border:0;margin-top:22px}
.msg{background:#202127;border-left:3px solid #6db56d;padding:8px 12px;border-radius:4px}
.warn{background:#202127;border-left:3px solid #d08a3e;padding:8px 12px;border-radius:4px;font-size:15px}
.found{color:#8a8b87;font-size:15px;margin:6px 0 0}
.bad{background:#2a1d1b;border-left:3px solid #d2605a;padding:8px 12px;border-radius:4px}
.forget{margin-top:28px}.forget button{background:#2f3037;color:#c9cac6;width:auto;padding:6px 14px;margin:0}
a{color:#6db56d}</style></head><body>
<h1>%NAME%</h1><p class=who>%WHO%</p>
%MSG%
<form method=post action=/settings>
%CODEROW%
<label>%PgSource%
<select name=source><option value=companion%CSEL%>%PgSourceCompanion%</option>
<option value=direct%DSEL%>%PgSourceDirect%</option></select></label>
<label>%PgCompanion% <small>%PgCompanionHint%</small>
<input name=companion placeholder="http://192.168.1.20:3888" value="%COMPANION%"></label>
<label>%PgToken% <small>%PgTokenHint%</small>
<input name=token type=password autocomplete=off></label>
<label>%PgLyrics% <small>%PgLyricsHint%</small>
<input name=lyrics placeholder="http://192.168.1.30:3300" value="%LYRICS%"></label>
<label>%PgLyricsToken% <small>%PgLyricsTokenHint%</small>
<input name=lyricstoken type=password autocomplete=off></label>
<label>%PgWeather% <small>%PgWeatherHint%</small>
<input name=weather placeholder="Montreal" value="%WEATHER%"></label>%WNOTE%
<label>%PgNews% <small>%PgNewsHint%</small>
<input name=news placeholder="https://example.com/rss" value="%NEWS%"></label>%NNOTE%
%LANGROW%%BMSG%<button>%PgSave%</button></form>
%FORGET%
<p><small>%PgFooterUpdate% <a href=/update>/update</a> &middot; %PgFooterSource%
<a href="https://github.com/r45635/tft-panel">github.com/r45635/tft-panel</a></small></p>
</body></html>)HTML";

const char PAGE[] PROGMEM = R"HTML(<!doctype html><html lang="%LANG%"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1"><title>%PgUpTabTitle%</title>
<style>body{font:17px/1.5 system-ui,sans-serif;max-width:560px;margin:32px auto;padding:0 16px;background:#15161a;color:#f6f4ee}
input,button{font:inherit;padding:10px;border-radius:8px;border:1px solid #2f3037;background:#202127;color:#f6f4ee}
button{background:#6db56d;color:#15161a;border:0;padding:10px 24px}#msg{margin-top:16px;color:#a9aaa6}
a{color:#6db56d}</style></head><body>
<h1>%PgUpTitle%</h1><p>%PgUpIntro%</p>
<p><input id="code" inputmode="numeric" maxlength="6" placeholder="%PgUpCode%" autocomplete="off"></p>
<p><input id="file" type="file" accept=".bin"></p>
<p><input id="sha" maxlength="64" autocomplete="off" placeholder="%PgUpSha%" style="width:100%;max-width:34em">
<br><small>%PgUpShaHint%</small></p>
%SIGROW%<p><button id="go">%PgUpGo%</button></p><div id="msg"></div>
<p><small>%PgUpReleases%
<a href="https://github.com/r45635/tft-panel/releases">github.com/r45635/tft-panel/releases</a>.
%PgUpAsset%</small></p>
<script>
go.onclick=()=>{const f=file.files[0];if(!f||code.value.length!=6){msg.textContent='%PgUpPick%';return}
const fd=new FormData();fd.append('firmware',f);const x=new XMLHttpRequest();
x.upload.onprogress=e=>{msg.textContent='%PgUpUploading% '+Math.round(100*e.loaded/e.total)+' %'};
x.onload=()=>{msg.textContent=x.responseText};x.onerror=()=>{msg.textContent='%PgUpLost%'};
const s=(sha.value.trim().split(/\s+/)[0]||'').toLowerCase();
if(s&&!/^[0-9a-f]{64}$/.test(s)){msg.textContent='%PgUpNotSha%';return}
const send=g=>{x.open('POST','/update?code='+encodeURIComponent(code.value)+(s?'&sha='+s:'')+(g?'&sig='+g:''));x.send(fd);go.disabled=true};
const g=window.sig&&sig.files[0];if(!g){send('');return}
if(g.size>72){msg.textContent='%PgUpNotSig%';return}
go.disabled=true;g.arrayBuffer().then(b=>send([...new Uint8Array(b)].map(v=>v.toString(16).padStart(2,'0')).join('')))}
</script></body></html>)HTML";

// The code lives on the panel's screen; it is also printed here, because a USB cable already
// gives this console `token` and `erase-all` — and it is how you read the code screen-off.
uint32_t newCode() {
    g_code = 100000 + esp_random() % 900000;
    g_codeAt = millis();
    Serial.printf("[web] code %lu, good for %lu minutes\n", (unsigned long)g_code,
                  (unsigned long)(CODE_VALID_MS / 60000));
    return g_code;
}

bool codeValid(const String &typed) {
    return g_code && millis() - g_codeAt < CODE_VALID_MS && typed.toInt() == (long)g_code;
}

// A browser that typed the code once gets a cookie, so changing a setting later asks nothing.
// The cookie is per panel (the token lives in that panel's NVS) and dies with `forget-wifi`,
// `erase-all` or the Forget button on the page. Firmware upload always asks for a fresh code.
// The SHA-256 of what the browser sent, so the person can compare it with the `.sha256` beside
// the firmware on the release page — and have the panel refuse it if they paste that digest in.
// The image's own integrity (truncation, corruption) is caught by the OTA layer regardless;
// this answers the other question, which is whether it is the file they meant to install.
mbedtls_sha256_context g_upSha;
bool g_upShaOn = false;
size_t g_upBytes = 0;          // what actually arrived, on either write path
size_t g_upTotal = 0;          // what the browser announced, for the progress screen
String g_upWant, g_upGot;
uint8_t g_upDigest[32];        // g_upGot as bytes, what a release signature covers
uint8_t g_upSig[signature::MAX_SIG];
size_t g_upSigLen = 0;

void shaStart() {
    if (g_upShaOn) mbedtls_sha256_free(&g_upSha);
    mbedtls_sha256_init(&g_upSha);
    mbedtls_sha256_starts(&g_upSha, 0);
    g_upShaOn = true;
    g_upBytes = 0;
    g_upGot = "";
}

String shaFinish() {
    if (!g_upShaOn) return String();
    mbedtls_sha256_finish(&g_upSha, g_upDigest);
    mbedtls_sha256_free(&g_upSha);
    g_upShaOn = false;
    char hex[65];
    for (int i = 0; i < 32; i++) snprintf(hex + i * 2, 3, "%02x", g_upDigest[i]);
    return String(hex);
}

bool g_trustedNow = false;      // set during the request that typed a correct code

bool trustedBrowser() {
    if (g_trustedNow) return true;
    String cookies = g_server->header("Cookie");
    if (!cookies.length()) return false;
    String want = "tftpanel=" + settings::webToken();
    for (int at = 0; at >= 0; ) {
        int end = cookies.indexOf(';', at);
        String one = end < 0 ? cookies.substring(at) : cookies.substring(at, end);
        one.trim();
        if (one == want) return true;
        at = end < 0 ? -1 : end + 1;
    }
    return false;
}

void trustThisBrowser() {
    g_trustedNow = true;        // so the page we are about to send already hides the code field
    // Path=/ so /update sees it too (it still asks for a code), a month before typing again.
    g_server->sendHeader("Set-Cookie", "tftpanel=" + settings::webToken() +
                                           "; Max-Age=2592000; Path=/; SameSite=Lax; HttpOnly");
}

// Download-first boards (board_pins.h) collect the upload in PSRAM and write it once complete:
// flash writes during the transfer would stall their Wi-Fi link.
uint8_t *g_image = nullptr;
size_t g_imageLen = 0, g_imageCap = 0;

void dropImage() {
    if (g_image) heap_caps_free(g_image);
    g_image = nullptr;
    g_imageLen = g_imageCap = 0;
}

// Every refusal goes through here: the screen is showing "Updating..." by then, and a path that
// returns without saying so leaves the panel on that picture for ever. Which is what the checksum
// branch did, the first time it ran.
// `why` answers the browser, which can hold a paragraph; `onScreen` is what the panel says, and
// it is short on purpose - two full sha256 across a screen in a living room is not a message,
// it is a wall of hex. The browser already has the digests.
void refuse(const String &why, const String &onScreen = String()) {
    g_uploadAllowed = false;
    g_uploadError = why;
    ota::uploadFailed(onScreen.length() ? onScreen : why);
}

void handleUpload() {
    HTTPUpload &up = g_server->upload();
    if (up.status == UPLOAD_FILE_START) {
        g_uploadOk = false;
        g_uploadWrongCode = false;
        g_uploadError = "";
        dropImage();
        g_uploadAllowed = codeValid(g_server->arg("code"));
        if (!g_uploadAllowed) {
            // Burn it. The settings page draws a new code on every refusal, so guessing there is
            // one chance in 900,000 each time; without this, an upload could be retried against
            // the same six digits for the whole five minutes. The message already says to reopen
            // the page, which is what now gets a fresh code.
            g_code = 0;
            g_uploadWrongCode = true;
            g_uploadError = tr(Str::WebWrongCode);
            return;
        }
        Serial.printf("[web] firmware upload started: %s\n", up.filename.c_str());
        // The code has done its work: clearing it takes the six digits off the panel's screen
        // (ui::tickCode) and makes them unusable a second time, and the update screen takes over.
        g_code = 0;
        // The whole multipart body, a few hundred bytes more than the firmware; close enough
        // for a bar, and 0 when the client announced nothing (then the words speak alone).
        g_upTotal = g_server->clientContentLength() > 0 ? (size_t)g_server->clientContentLength() : 0;
        ota::uploadStarted();
        shaStart();
        g_upWant = g_server->arg("sha");
        g_upWant.trim();
        g_upWant.toLowerCase();
        g_upSigLen = signature::fromHex(g_server->arg("sig").c_str(), g_upSig, sizeof g_upSig);
        if (board::OTA_DOWNLOAD_FIRST) {
            const esp_partition_t *slot = esp_ota_get_next_update_partition(nullptr);
            g_imageCap = slot ? slot->size : 0;
            g_image = g_imageCap ? (uint8_t *)heap_caps_malloc(g_imageCap, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) : nullptr;
            if (!g_image) refuse(tr(Str::WebNoMemory));
        } else if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
            refuse(i18n::fmt(Str::WebCannotStart, Update.errorString()));
        }
    } else if (up.status == UPLOAD_FILE_WRITE && g_uploadAllowed) {
        if (g_upShaOn) {
            mbedtls_sha256_update(&g_upSha, up.buf, up.currentSize);
            g_upBytes += up.currentSize;
            ota::uploadProgress(g_upBytes, g_upTotal);
        }
        if (g_image) {
            if (g_imageLen + up.currentSize > g_imageCap) {
                refuse(tr(Str::WebTooLarge));
                dropImage();
            } else {
                memcpy(g_image + g_imageLen, up.buf, up.currentSize);
                g_imageLen += up.currentSize;
            }
        } else if (Update.write(up.buf, up.currentSize) != up.currentSize) {
            refuse(i18n::fmt(Str::WebWriteFailed, Update.errorString()));
            Update.abort();
        }
    } else if (up.status == UPLOAD_FILE_END && g_uploadAllowed) {
        g_upGot = shaFinish();
        Serial.printf("[web] uploaded %u bytes, sha256 %s\n", (unsigned)g_upBytes, g_upGot.c_str());
        if (g_upWant.length() && g_upWant != g_upGot) {
            // Refused before a single byte is committed: a digest that does not match is either
            // the wrong file or a damaged transfer, and neither should reach the flash.
            refuse(i18n::fmt(Str::WebChecksum, g_upGot.c_str(), g_upWant.c_str()), tr(Str::WebChecksumScreen));
            if (g_image) dropImage();
            else Update.abort();
            return;
        }
        if (signature::enforced() && !signature::verifyRelease(g_upDigest, g_upSig, g_upSigLen)) {
            refuse(tr(g_upSigLen ? Str::WebSigMismatch : Str::WebSigMissing),
                   tr(g_upSigLen ? Str::WebSigMismatchScreen : Str::WebSigMissingScreen));
            if (g_image) dropImage();
            else Update.abort();
            return;
        }
        ota::uploadInstalling();
        if (g_image) {
            String err;
            if (ota::writeImage(g_image, g_imageLen, err, ota::uploadWriting)) g_uploadOk = true;
            else refuse(i18n::fmt(Str::WebNotImage, err.c_str()));
            dropImage();
        } else if (Update.end(true)) {
            g_uploadOk = true;
        } else {
            refuse(i18n::fmt(Str::WebNotImage, Update.errorString()));
        }
    } else if (up.status == UPLOAD_FILE_ABORTED && g_uploadAllowed) {
        shaFinish();
        if (g_image) dropImage();
        else Update.abort();
        refuse(tr(Str::WebInterrupted));
    }
}

void handleUploadDone() {
    if (g_uploadOk) {
        g_code = 0;
        g_server->send(200, "text/plain; charset=utf-8", i18n::fmt(Str::WebInstalled, g_upGot.c_str()));
        Serial.println("[web] firmware uploaded and verified, restarting");
        delay(1000);
        board::restart();
    }
    g_server->send(g_uploadWrongCode ? 403 : 400, "text/plain; charset=utf-8",
                   g_uploadError.length() ? g_uploadError : String(tr(Str::WebNoFirmware)));
    Serial.printf("[web] upload refused: %s\n", g_uploadError.c_str());
}

String esc(const String &in) {
    String o;
    o.reserve(in.length() + 8);
    for (unsigned i = 0; i < in.length(); i++) {
        char c = in[i];
        if (c == '&') o += "&amp;";
        else if (c == '<') o += "&lt;";
        else if (c == '>') o += "&gt;";
        else if (c == '"') o += "&quot;";
        else o += c;
    }
    return o;
}

// Everything that has to be typed lives here; the choices that do not are on the panel itself.
// Saving needs the code on its screen, so being on the network is not enough, and so the code
// also tells you which panel you have open.
void sendSettings(const String &message, bool error = false, bool keepTyped = false) {
    bool trusted = trustedBrowser();
    if (trusted) g_code = 0;        // nothing to type, so nothing on the panel's screen
    else newCode();
    settings::Settings st = settings::get();
    // On a refusal the page comes back with what was typed, not with what is stored.
    auto val = [&](const char *name, const String &stored) {
        return keepTyped ? g_server->arg(name) : stored;
    };
    String h = FPSTR(SETTINGS_PAGE);
    i18n::fillPage(h);
    h.replace("%LANG%", i18n::code(i18n::lang()));
    h.replace("%NAME%", esc(net::displayName()));
    // Escaped part by part: the separator is markup, not text.
    h.replace("%WHO%", esc(board::BOARD_NAME) + " &middot; " + esc(net::macAddress()) + " &middot; " +
                           esc(WiFi.localIP().toString()) + " &middot; " + esc(FW_VERSION));
    String box = message.length()
                     ? "<p class=" + String(error ? "bad" : "msg") + ">" + esc(message) + "</p>"
                     : String();
    h.replace("%MSG%", box);
    // A refusal is repeated beside the button: the top of the form is scrolled away by then.
    h.replace("%BMSG%", error ? box : String());
    h.replace("%CODEROW%",
              trusted ? "<p class=found>" + String(tr(Str::WebRemembered)) + "</p>"
                      : "<label>" + String(tr(Str::WebCodeLabel)) + "<input name=code "
                        "inputmode=numeric maxlength=6 required autocomplete=off "
                        "placeholder=\"" + tr(Str::WebCodeDigits) + "\"></label>");
    h.replace("%FORGET%", trusted ? "<form class=forget method=post action=/forget><button>" +
                                        String(tr(Str::WebForgetBrowser)) + "</button></form>"
                                  : String());
    String langRow;
    if (i18n::LANG_COUNT > 1) {
        langRow = "<label>" + String(tr(Str::Language)) + "<select name=lang>";
        for (int i = 0; i < i18n::LANG_COUNT; i++) {
            auto l = (i18n::Lang)i;
            langRow += "<option value=" + String(i18n::code(l)) + (l == i18n::lang() ? " selected>" : ">") +
                       i18n::nth(Str::LanguageChoices, i) + "</option>";
        }
        langRow += "</select></label>";
    }
    h.replace("%LANGROW%", langRow);
    bool direct = (keepTyped ? g_server->arg("source") == "direct"
                             : st.source == settings::SourceKind::Direct);
    h.replace("%COMPANION%", esc(val("companion", st.companionUrl)));
    h.replace("%CSEL%", direct ? "" : " selected");
    h.replace("%DSEL%", direct ? " selected" : "");
    h.replace("%LYRICS%", esc(val("lyrics", st.lyricsUrl)));
    h.replace("%WEATHER%", esc(val("weather", st.weatherPlace)));
    net::weather::Now w = net::snapshot().weather;
    String note;
    if (w.problem.length()) note = "<p class=warn>" + esc(w.problem) + "</p>";
    else if (w.located.length()) note = "<p class=found>" + esc(i18n::fmt(Str::WebShowing, w.located.c_str())) + "</p>";
    h.replace("%WNOTE%", note);
    h.replace("%NEWS%", esc(val("news", st.newsUrl)));
    String nnote;
    net::Snapshot snap = net::snapshot();   // a copy, like the weather note above
    if (snap.newsProblem.length()) nnote = "<p class=warn>" + esc(snap.newsProblem) + "</p>";
    else if (snap.news.length())
        nnote = "<p class=found>" + esc(i18n::fmt(Str::WebShowing, (snap.news.substring(0, 60) + "...").c_str())) + "</p>";
    h.replace("%NNOTE%", nnote);
    g_server->send(200, "text/html; charset=utf-8", h);
}

void handleSettingsSave() {
    g_trustedNow = false;
    bool trusted = trustedBrowser();
    if (!trusted) {
        if (!codeValid(g_server->arg("code"))) {
            sendSettings(tr(Str::WebCodeRefused), true, true);
            return;
        }
        trustThisBrowser();         // typed once, not asked again on this browser
    }
    // A field the request does not carry is left alone: a partial form must not blank a setting.
    settings::Settings was = settings::get();
    auto arg = [](const char *name, const String &unsent = String()) {
        if (!g_server->hasArg(name)) return unsent;
        String v = g_server->arg(name);
        v.trim();
        return v;
    };
    String companion = arg("companion", was.companionUrl), lyrics = arg("lyrics", was.lyricsUrl),
           weather = arg("weather", was.weatherPlace), news = arg("news", was.newsUrl);
    if (companion.length() && !companion.startsWith("http://")) {
        sendSettings(tr(Str::WebBadCompanion), true, true);
        return;
    }
    if (lyrics.length() && lyrics != "none" && !lyrics.startsWith("http")) {
        sendSettings(tr(Str::WebBadLyrics), true, true);
        return;
    }
    if (news.length() && !news.startsWith("http")) {
        sendSettings(tr(Str::WebBadNews), true, true);
        return;
    }
    while (companion.endsWith("/")) companion.remove(companion.length() - 1);
    while (lyrics.endsWith("/")) lyrics.remove(lyrics.length() - 1);

    settings::SourceKind wanted = !g_server->hasArg("source") ? was.source
                                  : arg("source") == "direct" ? settings::SourceKind::Direct
                                                              : settings::SourceKind::Companion;
    i18n::Lang lang = i18n::lang();
    if (g_server->hasArg("lang")) i18n::parse(arg("lang").c_str(), lang);
    bool langChanged = lang != i18n::lang() && (int)lang < i18n::LANG_COUNT;
    bool newSource = wanted != settings::source() || companion != was.companionUrl;
    bool restart = newSource || langChanged;
    // The answer names what moved, so a typo in a field you did not mean to touch shows up.
    String changed;
    auto note = [&changed](const String &what) {
        if (changed.length()) changed += ", ";
        changed += what;
    };
    // The answer is in the language just chosen, which is the one the panel restarts in.
    if (langChanged) i18n::setLang(lang);
    auto changedName = [](int n) { return i18n::nth(Str::WebChangedNames, n); };
    if (wanted != was.source) note(changedName(0));
    if (companion != was.companionUrl) note(changedName(1));
    if (arg("token").length()) note(changedName(2));
    if (lyrics != was.lyricsUrl) note(changedName(3));
    if (arg("lyricstoken").length()) note(changedName(4));
    if (weather != was.weatherPlace) note(changedName(5));
    if (news != was.newsUrl) note(changedName(6));
    if (langChanged) note(changedName(7));
    settings::setCompanion(companion);
    if (arg("token").length()) settings::setToken(arg("token"));
    settings::setSource(wanted);
    settings::setLyricsUrl(lyrics);
    if (arg("lyricstoken").length()) settings::setLyricsToken(arg("lyricstoken"));
    settings::setWeatherPlace(weather);
    settings::setNewsUrl(news);
    if (langChanged) settings::setLang((uint8_t)lang);
    net::lyricsrc::forgetDiscovered();
    net::weather::reset();
    net::news::reset();
    g_code = 0;

    Serial.println("[web] settings saved from the web page");
    if (restart) {
        g_server->send(200, "text/html; charset=utf-8",
                       String("<!doctype html><meta charset=utf-8><body style=\"font:17px system-ui;background:#15161a;"
                              "color:#f6f4ee;max-width:560px;margin:32px auto;padding:0 16px\">") +
                           "<h1>" + tr(Str::WebSaved) + "</h1><p>" + esc(changed) + ".</p><p>" +
                           tr(newSource ? Str::WebRestartSource : Str::LanguageText) + "</p>");
        delay(300);
        board::restart();
        return;
    }
    sendSettings(changed.length() ? i18n::fmt(Str::WebSavedList, changed.c_str()) : String(tr(Str::WebNothingToChange)));
}

void handleForget() {
    g_trustedNow = false;
    if (!trustedBrowser() && !codeValid(g_server->arg("code"))) {
        sendSettings(tr(Str::WebNotForgotten), true, false);
        return;
    }
    settings::forgetWebTokens();    // every remembered browser, not only this one
    g_trustedNow = false;
    g_server->sendHeader("Set-Cookie", "tftpanel=; Max-Age=0; Path=/");
    Serial.println("[web] remembered browsers forgotten");
    sendSettings(tr(Str::WebForgotten));
}

void start() {
    g_server = new WebServer(80);
    static const char *kHeaders[] = {"Cookie"};   // trustedBrowser() reads it
    g_server->collectHeaders(kHeaders, 1);
    g_server->on("/", HTTP_GET, []() { g_trustedNow = false; sendSettings(""); });
    g_server->on("/settings", HTTP_POST, handleSettingsSave);
    g_server->on("/forget", HTTP_POST, handleForget);
    g_server->on("/update", HTTP_GET, []() {
        newCode();                                 // shown on the panel, typed in the browser
        String page = FPSTR(PAGE);
        i18n::fillPage(page);
        page.replace("%LANG%", i18n::code(i18n::lang()));
        page.replace("%VERSION%", FW_VERSION);
        page.replace("%ASSET%", board::BOARD_ASSET);
        page.replace("%SIGROW%", signature::enforced()
                                     ? "<p>" + String(tr(Str::WebSigLabel)) + " <input id=\"sig\" type=\"file\" accept=\".sig\"><br><small>" +
                                           i18n::fmt(Str::WebSigHint, board::BOARD_ASSET) + "</small></p>"
                                     : String());
        g_server->send(200, "text/html; charset=utf-8", page);
    });
    g_server->on("/update", HTTP_POST, handleUploadDone, handleUpload);
    g_server->on("/screenshot", HTTP_GET, []() {
        const uint16_t *fb = board::frameBuffer();
        if (!g_shots || !fb) {
            g_server->send(403, "text/plain", "Screenshots are off (console: shots on).");
            return;
        }
        const uint16_t w = board::LCD_H_RES, h = board::LCD_V_RES;
        const size_t bytes = (size_t)w * h * 2;
        uint8_t head[8] = {'T', 'F', 'T', 'S', (uint8_t)w, (uint8_t)(w >> 8), (uint8_t)h, (uint8_t)(h >> 8)};
        g_server->client().setNoDelay(true);
        g_server->setContentLength(sizeof head + bytes);
        g_server->send(200, "application/octet-stream", "");
        g_server->sendContent((const char *)head, sizeof head);
        for (size_t off = 0; off < bytes; off += 32768)
            g_server->sendContent((const char *)fb + off, std::min((size_t)32768, bytes - off));
    });
    g_server->begin();
    // The service record belongs to the net task, which owns mDNS and attaches the panel's name,
    // board, MAC and version to it (internal::startMdns). Announcing it from here was a second,
    // bare registration that ran before mDNS was up and so failed every boot: "addService():
    // Failed adding service http.tcp", printed and ignored. The panel was announced anyway, by
    // the net task a moment later - which is exactly why an error nobody reads survives for ever.
    Serial.println("[web] update page on port 80 (/update)");
}

void stop() {
    g_server->stop();
    delete g_server;
    g_server = nullptr;
    g_code = 0;
}

}  // namespace

namespace {
void loop(bool wifiUp, bool portalActive) {
    bool want = wifiUp && !portalActive;   // the portal owns port 80 while it runs
    if (want && !g_server) start();
    else if (!want && g_server) stop();
    if (g_server) g_server->handleClient();
    if (g_code && millis() - g_codeAt >= CODE_VALID_MS) g_code = 0;
}
}  // namespace

void begin() {
    xTaskCreatePinnedToCore([](void *) {
        for (;;) {
            loop(WiFi.status() == WL_CONNECTED && WiFi.getMode() != WIFI_AP, portal::active());
            delay(2);
        }
    }, "web", 6144, nullptr, 1, nullptr, 0);
}

uint32_t activeCode() { return g_code; }
bool serving() { return g_server != nullptr; }
void enableScreenshots(bool on) { g_shots = on; }
void cancelCode() { g_code = 0; }

}  // namespace webupdate
