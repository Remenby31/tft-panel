#!/usr/bin/env python3
"""Host test of tools/backup-firmware.py with esptool replaced by an in-memory board.

    python3 -I tools/test_backup_firmware.py
"""
import configparser
import contextlib
import importlib.util
import io
import os
import random
import tempfile
import unittest
from unittest import mock

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("backup", os.path.join(HERE, "backup-firmware.py"))
backup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(backup)

MB = 1024 * 1024
VIEWE = backup.BOARDS[("ESP32-P4", 16)]
WAVESHARE = backup.BOARDS[("ESP32-P4", 32)]
FLASH_ID = {
    "crowpanel7": "Chip type: ESP32-S3 (QFN56) (revision v0.2)\nMAC: 24:0a:c4:00:00:01\n"
                  "Detected flash size: 4MB\n",
    "waveshare-p4-7b": "Chip type: ESP32-P4 (revision v3.2)\nMAC: 30:ed:a0:00:00:02\n"
                       "Detected flash size: 32MB\n",
    "p4-s070-wba": "Chip type: ESP32-P4 (revision v3.2)\nMAC: 80:f1:b2:00:00:03\n"
                   "Detected flash size: 16MB\n",
}


class FakeBoard:
    """Answers esptool calls from a byte string. `faults` maps a read offset to the ways that
    read fails, one per attempt: "stopped" (exit 2, no file: esptool 5), "cut" (exit 2, part of
    a file), "short" (exit 0, part of a file) or "error" (exit 2, a whole file)."""

    def __init__(self, flash=b"", flash_id="", faults=None):
        self.flash, self.flash_id = flash, flash_id
        self.faults = {k: list(v) for k, v in (faults or {}).items()}
        self.calls, self.reads = [], []

    def __call__(self, args, port, baud=None, capture=True, after=None):
        self.calls.append(args[0])
        if args[0] == "flash-id":
            return (0, self.flash_id) if self.flash_id else (2, "No serial data received.")
        if args[0] == "verify-flash":
            with open(args[2], "rb") as f:
                return (0, "") if f.read() == self.flash else (1, "verify failed")
        off, length, path = int(args[1], 16), int(args[2]), args[3]
        self.reads.append((off, length, after))
        fault = self.faults.get(off, [None]).pop(0) if self.faults.get(off) else None
        if fault == "stopped":
            return 2, "Serial data stream stopped"
        with open(path, "wb") as f:
            f.write(self.flash[off:off + (1000 if fault in ("cut", "short") else length)])
        return (0 if fault in (None, "short") else 2), ""


def quiet():
    return contextlib.redirect_stdout(io.StringIO())


class Backup(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.path = os.path.join(self.dir.name, "dump.bin")
        self.flash = random.Random(1).randbytes(16 * MB)

    def tearDown(self):
        self.dir.cleanup()

    def read(self, board, piece, size=None):
        with mock.patch.object(backup, "esptool", board), quiet():
            return backup.read_flash("/dev/x", 230400, size or len(self.flash), piece, self.path)

    def content(self):
        with open(self.path, "rb") as f:
            return f.read()

    def test_a_viewe_backup_is_32_pieces_with_the_board_held_in_its_bootloader(self):
        board = FakeBoard(self.flash, FLASH_ID["p4-s070-wba"])
        argv = ["backup", "--port", "/dev/x", "--out", self.dir.name]
        with mock.patch.object(backup, "esptool", board), quiet(), mock.patch("sys.argv", argv):
            self.assertEqual(backup.main(), 0)
        self.assertEqual(board.reads, [(i * 512 * 1024, 512 * 1024, "no-reset") for i in range(32)])
        files = sorted(os.listdir(self.dir.name))
        self.assertEqual([f.split("-factory-")[0] for f in files], ["p4-s070-wba"] * 2)
        self.assertEqual([os.path.splitext(f)[1] for f in files], [".bin", ".sha256"])
        self.path = os.path.join(self.dir.name, files[0])
        self.assertEqual(self.content(), self.flash)

    def test_every_kind_of_broken_piece_is_read_again(self):
        faults = {3 * 512 * 1024: ["stopped", "cut"], 9 * 512 * 1024: ["short"],
                  20 * 512 * 1024: ["error", "stopped"]}
        board = FakeBoard(self.flash, faults=faults)
        self.assertTrue(self.read(board, VIEWE.piece))
        self.assertEqual(len(board.reads), 32 + 5)
        self.assertEqual(self.content(), self.flash)
        self.assertEqual(os.listdir(self.dir.name), ["dump.bin"])

    def test_a_piece_that_never_arrives_leaves_no_file(self):
        board = FakeBoard(self.flash, faults={3 * 512 * 1024: ["stopped", "cut", "short"]})
        self.assertFalse(self.read(board, VIEWE.piece))
        self.assertEqual(os.listdir(self.dir.name), [])

    def test_the_last_piece_is_only_what_is_left(self):
        size = 3 * 512 * 1024 + 100
        board = FakeBoard(self.flash)
        self.assertTrue(self.read(board, VIEWE.piece, size))
        self.assertEqual(board.reads[-1][:2], (3 * 512 * 1024, 100))
        self.assertEqual(self.content(), self.flash[:size])

    def test_other_boards_are_read_in_one_call(self):
        board = FakeBoard(self.flash)
        self.assertTrue(self.read(board, WAVESHARE.piece))
        self.assertEqual(board.reads, [(0, 16 * MB, "no-reset")])


class Restore(unittest.TestCase):
    def restore(self, backup_file, flash_id="", baud=None):
        board = FakeBoard(flash_id=flash_id)
        argv = ["backup", "--port", "/dev/x", "--restore", backup_file] + (
            ["--baud", str(baud)] if baud else [])
        out = io.StringIO()
        with mock.patch.object(backup, "esptool", board), contextlib.redirect_stdout(out), \
                mock.patch("sys.argv", argv):
            self.assertEqual(backup.main(), 0)
        line = [l for l in out.getvalue().splitlines() if "write-flash" in l][0]
        return line.split("--baud ")[1].split()[0], board.calls

    def test_the_file_name_gives_the_flash_speed_without_touching_the_board(self):
        for env, baud in (("crowpanel7", "460800"), ("waveshare-p4-7b", "921600"),
                          ("p4-s070-wba", "230400")):
            self.assertEqual(self.restore("/b/%s-factory-80f1b2-2026-11-15.bin" % env),
                             (baud, []), env)

    def test_a_renamed_file_asks_the_board(self):
        self.assertEqual(self.restore("/b/old.bin", FLASH_ID["p4-s070-wba"]), ("230400", ["flash-id"]))

    def test_a_board_that_does_not_answer_still_gets_a_command(self):
        self.assertEqual(self.restore("/b/old.bin"), ("115200", ["flash-id"]))

    def test_baud_overrides(self):
        self.assertEqual(self.restore("/b/p4-s070-wba-factory-x.bin", baud=115200)[0], "115200")

    def test_flash_speeds_match_platformio_ini(self):
        ini = configparser.ConfigParser(interpolation=None, inline_comment_prefixes=(";",))
        ini.read(os.path.join(HERE, "..", "platformio.ini"))
        for board in backup.BOARDS.values():
            self.assertEqual(board.flash_baud, ini.getint("env:" + board.env, "upload_speed"),
                             board.env)


if __name__ == "__main__":
    unittest.main()
