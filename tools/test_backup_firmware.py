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
    """Answers esptool calls from a byte string. `drops` maps a read offset to how many times
    that read stops early, as the VIEWE's CH340 does."""

    def __init__(self, flash, flash_id="", drops=None):
        self.flash, self.flash_id, self.drops = flash, flash_id, dict(drops or {})
        self.reads = []

    def __call__(self, args, port, baud=None, capture=True):
        if args[0] == "flash-id":
            return 0, self.flash_id
        assert args[0] == "read-flash", args
        off, length, path = int(args[1], 16), int(args[2]), args[3]
        self.reads.append((off, length, baud))
        with open(path, "wb") as f:
            if self.drops.get(off, 0) > 0:
                self.drops[off] -= 1
                f.write(self.flash[off:off + 1000])
                return 2, "Serial data stream stopped"
            f.write(self.flash[off:off + length])
        return 0, ""


class ReadFlash(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.path = os.path.join(self.dir.name, "dump.bin")
        self.flash = random.Random(1).randbytes(16 * MB)

    def tearDown(self):
        self.dir.cleanup()

    def read(self, board, piece):
        with mock.patch.object(backup, "esptool", board), contextlib.redirect_stdout(io.StringIO()):
            return backup.read_flash("/dev/null", 230400, len(self.flash), piece, self.path)

    def test_viewe_is_read_in_32_pieces_that_rebuild_the_flash(self):
        board = FakeBoard(self.flash)
        self.assertTrue(self.read(board, VIEWE.piece))
        self.assertEqual([(o, n) for o, n, _ in board.reads],
                         [(i * 512 * 1024, 512 * 1024) for i in range(32)])
        with open(self.path, "rb") as f:
            self.assertEqual(f.read(), self.flash)

    def test_a_dropped_piece_is_read_again(self):
        board = FakeBoard(self.flash, drops={5 * 512 * 1024: 2})
        self.assertTrue(self.read(board, VIEWE.piece))
        self.assertEqual(len(board.reads), 34)
        with open(self.path, "rb") as f:
            self.assertEqual(f.read(), self.flash)

    def test_a_piece_that_never_arrives_leaves_no_file(self):
        board = FakeBoard(self.flash, drops={3 * 512 * 1024: 3})
        self.assertFalse(self.read(board, VIEWE.piece))
        self.assertEqual(os.listdir(self.dir.name), [])

    def test_other_boards_are_read_in_one_call(self):
        board = FakeBoard(self.flash)
        self.assertTrue(self.read(board, WAVESHARE.piece))
        self.assertEqual([(o, n) for o, n, _ in board.reads], [(0, 16 * MB)])


class Restore(unittest.TestCase):
    def restore_command(self, env):
        board = FakeBoard(b"", FLASH_ID[env])
        out = io.StringIO()
        with mock.patch.object(backup, "esptool", board), contextlib.redirect_stdout(out), \
                mock.patch("sys.argv", ["backup", "--port", "/dev/x", "--restore", "f.bin"]):
            self.assertEqual(backup.main(), 0)
        return [l for l in out.getvalue().splitlines() if "write-flash" in l][0]

    def test_restore_prints_each_boards_flash_speed(self):
        self.assertIn("--baud 460800 write-flash", self.restore_command("crowpanel7"))
        self.assertIn("--baud 921600 write-flash", self.restore_command("waveshare-p4-7b"))
        self.assertIn("--baud 230400 write-flash", self.restore_command("p4-s070-wba"))

    def test_flash_speeds_match_platformio_ini(self):
        ini = configparser.ConfigParser(interpolation=None, inline_comment_prefixes=(";",))
        ini.read(os.path.join(HERE, "..", "platformio.ini"))
        for board in backup.BOARDS.values():
            self.assertEqual(board.flash_baud, ini.getint("env:" + board.env, "upload_speed"),
                             board.env)


if __name__ == "__main__":
    unittest.main()
