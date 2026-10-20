import wreport
import unittest
from pathlib import Path
import io
import os

testdata_path = Path(os.environ["WREPORT_TESTDATA"])


class Bulletin(unittest.TestCase):
    def test_read_bufr(self) -> None:
        path = testdata_path / "bufr" / "gts-buoy1.bufr"
        with path.open("rb") as fd:
            contents = list(wreport.read_bufr(fd))
        self.assertEqual(contents, [(0, path.read_bytes())])

    def test_read_bufr_short0(self) -> None:
        path = testdata_path / "bufr" / "short0.bufr"
        with path.open("rb") as fd:
            contents = list(wreport.read_bufr(fd))
        self.assertEqual(contents, [])

    def test_read_bufr_short1(self) -> None:
        path = testdata_path / "bufr" / "short1.bufr"
        with path.open("rb") as fd:
            with self.assertRaisesRegex(
                RuntimeError,
                r"cannot read BUFR section 0 \(4 bytes\)"
                r" from .+short1.bufr \(partial read\)",
            ):
                list(wreport.read_bufr(fd))

    def test_read_bufr_short2(self) -> None:
        path = testdata_path / "bufr" / "short2.bufr"
        with path.open("rb") as fd:
            with self.assertRaisesRegex(
                RuntimeError,
                r"cannot read BUFR message \(3618607 bytes\)"
                r" from .+short2.bufr \(partial read\)",
            ):
                list(wreport.read_bufr(fd))

    def test_read_bufr_short3(self) -> None:
        path = testdata_path / "bufr" / "short3.bufr"
        with path.open("rb") as fd:
            with self.assertRaisesRegex(
                RuntimeError,
                r"cannot read BUFR message \(647 bytes\)"
                r" from .+short3.bufr \(partial read\)",
            ):
                list(wreport.read_bufr(fd))

    def test_read_crex(self) -> None:
        path = testdata_path / "crex" / "test-synop0.crex"
        with path.open() as fd:
            contents = list(wreport.read_crex(fd))
        self.assertEqual(contents, [(0, path.read_bytes().strip())])

    def test_truncated_crex(self) -> None:
        for src in (
            "CREX++",
            "CREX++\nT000103 A001 D08003++\n",
            "CREX++\nT000103 A001 D08003++///++\n777",
        ):
            with self.subTest(src=src), io.StringIO(src) as fd:
                with self.assertRaisesRegex(
                    RuntimeError,
                    r"CREX message is incomplete in <_io\.StringIO object",
                ):
                    list(wreport.read_crex(fd))
