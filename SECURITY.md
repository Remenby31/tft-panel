# Security

## Reporting something

Please report a vulnerability privately, through GitHub's
[security advisories](https://github.com/r45635/tft-panel/security/advisories/new), not in a public
issue. Tell me what the panel does and what you expected; a serial log (`pio device monitor`, or the
console's `status`) usually says the rest. I answer as fast as a side project allows, and I will
credit you in the advisory unless you would rather I did not.

## What this firmware assumes

The panel is a device **on a home network**. It trusts that network, as a printer or a NAS does,
and nothing more:

- **No account, no cloud, no telemetry.** It talks to the music server or companion you point it
  at, to the lyrics server you name, and — if you ask for them — to Open-Meteo for the weather, to
  the news feed you type, and to GitHub to see whether a newer release exists. Nothing else.
- **Wi-Fi credentials, tokens and pairings live in NVS on the device**, never in the repository, a
  build flag or a log. `forget-wifi` clears the network, `erase-all` clears everything.
- **The panel's own page is plain HTTP on your LAN.** Anyone who can reach it can *read* the
  settings; changing one needs the six digits shown on that panel's screen, so being on the network
  is not enough — you have to be in front of it. A browser that typed them once is remembered for a
  month in an `HttpOnly`, `SameSite=Lax` cookie, which the page's *Forget this browser* button,
  `forget-wifi` and `erase-all` all revoke. A firmware upload always asks for a fresh code, and a
  wrong one is burned rather than left to be guessed at.
- **Because it is plain HTTP, a device that can watch your LAN traffic can take that cookie.** On a
  network where that matters, keep the panel on a segment you trust.
- **`GET /screenshot` is off until the USB console turns it on** (`shots on`), and goes off again at
  the next restart: it would otherwise hand the screen's contents to anyone on the network.
- **Updates are checked over HTTPS against GitHub**, verified by SHA-256 before they are installed,
  and rolled back automatically if the new image never reports itself healthy.
- **Once the maintainer compiles in a signing key, updates must be signed by it**: an image without
  a valid ECDSA P-256 signature is refused, from GitHub and from the panel's upload page, before it
  is committed. Until then, the SHA-256 only proves the image is the one the release published, so
  whoever can publish a release can ship any image. See `docs/RELEASING.md`, *Signed updates*.
  A signature says who made an image, not which version or screen it is for: the upload page,
  behind the code on the screen, still installs an older signed release or another board's.
  Secure boot and flash encryption are not used: they burn eFuses, which cannot be undone.

## Turning signed updates on

A panel without a key installs any update whose SHA-256 matches its release. So the build that
first carries the key is installed on that trust alone, and whatever key it carries, the panel
trusts from then on. That build must come from a release the maintainer built and signed on
their own machine.

1. **Create the key**, once: `tools/signing-key.sh`. The private key lands in
   `~/.config/tft-panel/signing-key.pem`, outside the repository. Put two copies offline (a USB
   stick and a password manager, say) before going further. Commit `src/net/signing_key.h`.
2. **Publish the first keyed release yourself**: `tools/release.sh`, on your machine, with that
   private key. It checks that the key matches the header, signs every `.bin`, verifies each
   signature with `openssl` and uploads the `.sig` beside it. Nobody else and no CI builds this one.
3. **Check what GitHub serves**, right after publishing: download each board's `.bin` and `.sig`
   from the release and run
   `openssl dgst -sha256 -verify <(openssl pkey -in ~/.config/tft-panel/signing-key.pem -pubout) -signature <base>-<tag>.sig <base>-<tag>.bin`.
   It must print `Verified OK`. If it does not, delete the release at once: panels look for a new
   one 30 s after boot and every 24 h.
4. **From then on, every release goes through `tools/release.sh` with the key.** A panel that
   installed step 2's release, or any later one, accepts only images that key signed.

When a signature is missing or wrong, nothing is installed and the panel keeps running its version:

- **Update from GitHub**: a release without a `.sig` shows no update badge. *Settings › Updates*
  reads `Last attempt failed: update vX.Y.Z is not signed, so it is not installed`, or, when the
  `.sig` does not match, `Last attempt failed: signature does not match: nothing installed`.
- **The panel's upload page**: the browser says `No signature: nothing installed` or
  `Signature does not match: nothing installed`, and the screen says
  `That file came without its signature: nothing installed.` or
  `That file is not signed by the maintainer: nothing installed.`

**A lost private key means USB only, for good.** No update over the air can replace the key without
a signature from it, so every panel built with it takes new firmware only by USB (`pio run -t upload`
or the browser installer). **A leaked key** lets its holder sign updates for every panel: publish
at once a release, signed with the old key, that carries a new one, and tell users to update.

## A full-flash backup is a secret

`tools/backup-firmware.py` copies the whole flash, which is what makes it restorable — and that
includes NVS: the Wi-Fi password the panel was given, any access token, and its pairing with your
music server. Verified by restoring one: the panel came back on the network and paired, with no
setup. Keep those files as you keep passwords, and never attach one to an issue. A firmware image
built from this repository carries none of it.

## What is out of scope

An attacker who already has your Wi-Fi password, physical access to the panel's USB port, or
control of the music server it is paired with. Those are all "game over" by design, as they are for
most things on a home network.
