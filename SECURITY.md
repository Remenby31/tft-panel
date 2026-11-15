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
