#!/usr/bin/env python3
"""Identify the board, then save its whole flash and prove the copy is good.

Run this **before the first flash**: whatever the board came with — a vendor demo, a previous
firmware — is overwritten and cannot be downloaded again from anywhere.

    tools/backup-firmware.py --port /dev/cu.usbserial-XXXX

It reads the chip, its revision, the flash size and the MAC, says which TFT Panel image that
board takes, dumps the entire flash, writes a `.sha256` beside it, and then **reads the flash
back and compares it against the file**. A hash of what we just wrote proves nothing about the
device; `verify-flash` does, because it is the board that is asked again.

Nothing is written to the board. Exit code 0 only when the copy is verified.

    --out DIR     where to keep it (default ~/tft-panel-backups)
    --baud N      serial speed for the read (default: what the board's adapter tolerates over a
                  whole flash dump, which is slower than the speed it is flashed at)
    --restore F   print the exact command to put a backup back, and stop
"""
import argparse
import hashlib
import os
import re
import subprocess
import sys
from datetime import date

PY = sys.executable

# What each chip and flash size means here. The two ESP32-P4 boards differ only by flash size,
# which is exactly why the browser installer cannot tell them apart.
# The last field is the speed for *reading*, which is not the speed for writing: a CH340 that
# flashes happily at 460800 loses bytes somewhere in four megabytes of read ("Serial data stream
# stopped"), and a backup that stops early is not a backup. Measured on the boards.
BOARDS = {
    ("ESP32-S3", 4): ("Elecrow CrowPanel 7\" V3.0", "crowpanel7", "tft-panel", 230400),
    ("ESP32-P4", 32): ("Waveshare ESP32-P4-WIFI6-Touch-LCD-7B", "waveshare-p4-7b",
                       "tft-panel-waveshare-p4-7b", 921600),
    ("ESP32-P4", 16): ("VIEWE UEP4S070H1024V600C-WB-C", "p4-s070-wba",
                       "tft-panel-p4-s070-wba", 230400),
}


def esptool(args, port, baud=None, capture=True):
    cmd = [PY, "-m", "esptool", "--port", port]
    if baud:
        cmd += ["--baud", str(baud)]
    cmd += args
    if capture:
        r = subprocess.run(cmd, capture_output=True, text=True)
        return r.returncode, r.stdout + r.stderr
    return subprocess.call(cmd), ""


def identify(port):
    """Chip, revision, flash megabytes and MAC, read from the board itself."""
    code, out = esptool(["flash-id"], port)
    if code != 0:
        sys.exit("Could not talk to the board on %s.\n%s" % (port, out.strip()))
    chip = rev = mac = None
    mb = 0
    for line in out.splitlines():
        # esptool 5 prints "Chip type: ESP32-S3 (QFN56) (revision v0.2)"; esptool 4, "Chip is …".
        m = (re.search(r"Chip type:\s*(ESP32\S*)", line) or
             re.search(r"Detecting chip type\.\.\.\s*(ESP32\S*)", line) or
             re.search(r"^Chip is (ESP32\S*)", line))
        if m and not chip:
            chip = m.group(1)
        m = re.search(r"\(revision v?([\d.]+)\)", line)
        if m and not rev:
            rev = m.group(1)
        m = re.search(r"MAC:\s*([0-9a-fA-F:]{17})", line)
        if m and not mac:
            mac = m.group(1).lower()
        m = re.search(r"Detected flash size:\s*(\d+)MB", line)
        if m:
            mb = int(m.group(1))
    if not chip or not mb:
        sys.exit("The board answered, but not with a chip and a flash size:\n" + out.strip())
    return chip, rev, mb, mac


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", required=True)
    ap.add_argument("--out", default=os.path.expanduser("~/tft-panel-backups"))
    ap.add_argument("--baud", type=int)
    ap.add_argument("--restore", metavar="FILE")
    a = ap.parse_args()

    if a.restore:
        print("Put this backup back on the board, then check it:\n")
        print("  %s -m esptool --port %s --baud 460800 write-flash 0x0 %s" % (PY, a.port, a.restore))
        print("  %s -m esptool --port %s verify-flash 0x0 %s" % (PY, a.port, a.restore))
        return 0

    print("Asking the board what it is...")
    chip, rev, mb, mac = identify(a.port)
    known = BOARDS.get((chip, mb))
    print("  chip        %s%s" % (chip, " rev " + rev if rev else ""))
    print("  flash       %d MB" % mb)
    print("  MAC         %s" % (mac or "unknown"))
    if known:
        name, env, asset, baud = known
        print("  board       %s" % name)
        print("  its image   %s  (pio run -e %s)" % (asset, env))
    else:
        name, env, asset, baud = "unknown board", "", "", 115200
        print("  board       not one of the three this firmware supports.")
        print("              Back it up anyway, but do not flash TFT Panel onto it: a panel")
        print("              driven by the wrong settings stays dark. docs/HARDWARE.md explains.")

    baud = a.baud or baud
    os.makedirs(a.out, exist_ok=True)
    stem = "%s-factory-%s-%s" % (env or chip.lower(), (mac or "nomac").replace(":", ""), date.today())
    path = os.path.join(a.out, stem + ".bin")
    size = mb * 1024 * 1024

    if os.path.exists(path):
        sys.exit("There is already a backup at %s — it is not overwritten. Move it or pass --out."
                 % path)

    print("\nReading all %d MB at %d baud. This takes a few minutes; do not unplug the board." %
          (mb, baud))
    code, _ = esptool(["read-flash", "0x0", str(size), path], a.port, baud, capture=False)
    if code != 0 or not os.path.exists(path):
        sys.exit("The read failed, so there is no backup. Nothing was written to the board.")
    got = os.path.getsize(path)
    if got != size:
        sys.exit("The file is %d bytes, not the %d the board reports: the read was cut short, "
                 "and a partial backup is not a backup. Delete %s and run this again."
                 % (got, size, path))

    digest = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            digest.update(block)
    sha = digest.hexdigest()
    with open(path + ".sha256", "w") as f:
        f.write("%s  %s\n" % (sha, os.path.basename(path)))

    print("\nReading the flash back to compare it with the file...")
    code, out = esptool(["verify-flash", "0x0", path], a.port, baud)
    if code != 0:
        print(out.strip())
        sys.exit("The board does not read back the same bytes, so this copy cannot be trusted. "
                 "Delete %s and run this again, with --baud 115200 if it keeps happening." % path)

    print("\nVerified against the board.")
    print("  %s" % path)
    print("  sha256 %s" % sha)
    print("\nPut it back one day with:")
    print("  tools/backup-firmware.py --port %s --restore %s" % (a.port, path))
    return 0


if __name__ == "__main__":
    sys.exit(main())
