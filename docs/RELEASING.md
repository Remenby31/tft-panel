# Releasing

A release is a git tag `vX.Y.Z` on `main` with three assets per supported screen on its GitHub
release, plus the web installer on GitHub Pages. Panels in the field update from the **latest** (non-pre-release)
GitHub release, so publishing one ships it to everyone: test it on a panel first.

```bash
tools/release.sh v0.2.0 "What changed, for users"
```

The script refuses a dirty tree, a branch other than `main`, an unpushed `main` or an existing
tag. It then:

1. tags, builds each board's environment, and checks that the built version is exactly the tag;
2. writes `dist/<tag>/`, per board, with `<base>` = the board's `BOARD_ASSET`: `tft-panel` for
   the CrowPanel (the first board kept the plain name, which its panels already look for),
   `tft-panel-waveshare-p4-7b` for the Waveshare, `tft-panel-p4-s070-wba` for the VIEWE:
   - `<base>-<tag>.bin` — app image, what the panels install over the air;
   - `<base>-<tag>.sha256` — `<hex>  <file>`; the panel compares it before committing the image;
   - `<base>-<tag>.sig` — only once a signing key is set (see *Signed updates*): the ECDSA P-256
     signature of that SHA-256, DER, as `openssl dgst -sha256 -sign` writes it;
   - `<base>-<tag>-factory.bin` — bootloader + partition table + app (PlatformIO's
     `firmware.factory.bin`), for a manual flash at the bootloader offset (0x0 on the S3, 0x2000 on the P4);
3. pushes the tag and creates the GitHub release with these assets;
4. publishes the website with `tools/site.sh` (landing page, user guide rendered from
   `docs/USER_GUIDE.md`, installer page): an orphan `gh-pages` commit with the HTML,
   one `manifest-<env>.json` **per board** and, per board, its bootloader, partition table,
   `boot_app0` and app — latest version only,
   so binaries never pile up in git. Four parts rather than the merged factory image: the merged
   image fills the NVS area (0x9000) with 0xFF and would wipe a reinstalled panel's Wi-Fi and
   settings (the installer's own "erase" choice still wipes everything for a fresh start).

GitHub Pages serves `gh-pages` at <https://r45635.github.io/tft-panel/>. To publish doc changes
without a release, run `tools/site.sh` alone: it keeps the firmware already on the site. ESP Web Tools needs the
image on the same origin (release assets have no CORS headers), hence the copy there. It flashes
at 115200 baud, which the CrowPanel's CH340 handles (921600 fails on that board); version 10.4 is
the first that knows the ESP32-P4.

Building both environments one after the other makes pioarduino redo the CrowPanel's framework
rebuild (its `custom_sdkconfig`), and that first pass can fail on a missing `.sconsign` file: the
script retries once, and copies each board's files right after its build.

## What a panel does with it

- 30 s after boot and every 24 h it reads `releases/latest`. A tag greater than its own
  `FW_VERSION` (a dev build `vX.Y.Z-n-gsha` counts as `vX.Y.Z`) with both assets shows the badge.
- Install: SHA-256 of the stream must match the `.sha256` asset, else nothing is committed.
  With a signing key compiled in, the `.sig` must also verify that SHA-256, else nothing is
  committed; a release without a `.sig` gets no badge and the settings screen says it is not signed.
- First boot of the new image: confirmed once Wi-Fi is up and the companion answers; if not
  within 3 minutes, the bootloader rolls back to the previous image.

## Signed updates

Without a key, a panel trusts whatever the latest GitHub release holds: whoever can publish a
release, or tamper with its files, can ship any image. With a key compiled in, a panel installs only
an image signed by the maintainer, from GitHub and from its own upload page alike.

1. Once: `tools/signing-key.sh`. It creates the private key in `~/.config/tft-panel/signing-key.pem`
   (or `$TFT_SIGNING_KEY`, never inside the repository) and writes its public half into
   `src/net/signing_key.h`. Commit that header. Keep a copy of the private key offline: without it,
   panels built with this key take no update but a USB flash or the browser installer.
2. Every release: `tools/release.sh` sees the key in the header, checks that the private key is
   its pair (before tagging), signs each `.bin`, verifies the signature with `openssl` and uploads
   the `.sig` with the other files.

The first release that carries the key is installed on trust like any before it (SECURITY.md,
*Turning signed updates on*, says how to roll it out); from then on,
panels refuse unsigned or badly signed images. Signing covers the image, not the bootloader:
secure boot and flash encryption burn eFuses for good and stay out of scope.

ECDSA P-256 rather than Ed25519: the mbedTLS of ESP-IDF 5.5 has no EdDSA, and P-256 is already in
the image for TLS, so the check costs almost no flash.

## Testing an update without publishing

The local upload page exercises the same install and rollback path as a GitHub update:

1. `GET http://tft-panel-xxxx.local/update` (or the panel's IP) — the panel shows a 6-digit code.
2. `curl -F firmware=@.pio/build/<env>/firmware.bin "http://<panel>/update?code=<code>"`
   (~30 s for 1.8 MB). The log shows `first boot of …: rollback in 3 min…`, then `confirmed working`.
   A build with a signing key also wants the signature, as hex:
   `&sig=$(openssl dgst -sha256 -sign ~/.config/tft-panel/signing-key.pem firmware.bin | xxd -p -c0)`.
3. Rollback: put `#define OTA_TEST_NEVER_HEALTHY 1` at the top of `src/net/ota.cpp` (temporarily —
   `PLATFORMIO_SRC_BUILD_FLAGS` did not reach it), build, upload: after 3 min the log shows
   `never got healthy` and the panel boots the previous image, without the "first boot" line.
   Verified 2026-11-07.

The GitHub download path (TLS stream + SHA-256) needs a real release: build with an older
version, `TFT_FW_VERSION=v0.1.0-test pio run -e <env> -t upload`, then console `ota check` and
`ota install`. Verified 2026-11-07 with v0.1.1 (the first real release, v0.1.0, had a broken
check: always test a release this way before announcing it).

## Waveshare P4 specifics

- The P4 image embeds the ESP32-C6 coprocessor firmware (`assets/hosted/esp32c6.bin`, not in git):
  `tools/hosted_fw.py` downloads it at build time and checks its pinned SHA-256.
- The P4 receives the whole update into PSRAM before writing it (flash writes stall its Wi-Fi link).
  v0.1.11 did not, so a P4 on v0.1.11 cannot update itself: it needs the web installer once.

## Rules

- The partition tables (`partitions.csv`, `partitions_p4.csv`) are final: an OTA cannot change them.
- No credentials, companion address or lyrics in a release (settings live in NVS only).
- Build on the Mac with the pinned toolchain. CI is not set up yet: the pioarduino hybrid build
  needs the platform-reinstall workaround described in `CLAUDE.md`.
