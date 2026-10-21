import wreport
import datetime as dt
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

    def assertGtsBuoy1Header(self, bulletin: wreport.Bulletin) -> None:
        self.assertEqual(bulletin.master_table_number, 0)
        self.assertEqual(bulletin.data_category, 1)
        self.assertIsNone(bulletin.data_subcategory)
        self.assertEqual(bulletin.data_subcategory_local, 0)
        self.assertEqual(bulletin.originating_centre, 214)
        self.assertEqual(bulletin.originating_subcentre, 0)
        self.assertEqual(bulletin.update_sequence_number, 0)
        self.assertEqual(bulletin.edition_number, 3)
        self.assertEqual(bulletin.master_table_version_number, 11)
        self.assertEqual(bulletin.master_table_version_number_local, 0)
        self.assertIsNone(bulletin.master_table_version_number_bufr)
        self.assertFalse(bulletin.compression)
        self.assertFalse(bulletin.has_check_digit)
        self.assertIsNone(bulletin.optional_section)
        self.assertEqual(bulletin.reftime, dt.datetime(2015, 3, 5, 0, 0, 0))
        self.assertEqual(
            bulletin.datadesc,
            ("B01003", "B01020", "B01005", "B02001", "B02036", "B02149")
            + ("D01011", "D01012", "B08021", "D01011", "D01012", "B08021")
            + ("D01021", "B27004", "B28004", "B07030", "B01051", "B02148")
            + ("B01012", "B01014", "B02040", "B33022", "B33023", "B33027")
            + ("B22063", "D02021", "D02022", "D02023", "B08081", "B25026")
            + ("B08081", "B25026", "B08081", "B25026", "B08081", "B02034")
            + ("B22060", "B07070", "B02190", "B25086", "B02035", "B02168")
            + ("B20031", "B02038", "B02032", "B02033", "R03000", "B31001")
            + ("B07062", "B22043", "B22062", "B02030", "B02031", "R03000")
            + ("B31001", "B07062", "B22004", "B22031", "B07031", "B08081")
            + ("B12064", "D02001", "B08081", "B07032", "B07033", "B12101")
            + ("B12103", "B13003", "B07032", "B07033", "B08082", "B07033")
            + ("B02169", "B02002", "B08021", "B04025", "B11001", "B11002")
            + ("B08021", "B04025", "B11043", "B11041", "B08082", "B07033")
            + ("B07032", "B04024", "B13011", "B07032", "B08021", "B04024")
            + ("B14021", "B08021", "B25028", "B25028", "B25028", "B22073")
            + ("B22070", "B22074", "B22076", "B22077", "B22071", "B22078")
            + ("B22082", "B22084", "B25043", "B25044", "R12000", "B31001")
            + ("B22080", "C01134", "B22096", "C01000", "B22090", "B22086")
            + ("B22087", "B22095", "B22085", "B22088", "B22089", "B22092"),
        )

    def test_decode_bufr_header(self) -> None:
        path = testdata_path / "bufr" / "gts-buoy1.bufr"
        decoded: list[wreport.Bulletin] = []
        with path.open("rb") as fd:
            for offset, data in wreport.read_bufr(fd):
                decoded.append(
                    wreport.Bulletin.decode_bufr_header(
                        data, fname=path, offset=offset
                    )
                )

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(bulletin.fname, path.as_posix())
        self.assertEqual(bulletin.offset, 0)
        self.assertEqual(len(bulletin), 0)
        self.assertGtsBuoy1Header(bulletin)

    def test_decode_bufr(self) -> None:
        path = testdata_path / "bufr" / "gts-buoy1.bufr"
        decoded: list[wreport.Bulletin] = []
        with path.open("rb") as fd:
            for offset, data in wreport.read_bufr(fd):
                decoded.append(
                    wreport.Bulletin.decode_bufr(
                        data, fname=path, offset=offset
                    )
                )

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(bulletin.fname, path.as_posix())
        self.assertEqual(bulletin.offset, 0)
        self.assertEqual(len(bulletin), 1)
        self.assertGtsBuoy1Header(bulletin)

    def assertCrexSynop0Header(self, bulletin: wreport.Bulletin) -> None:
        self.assertEqual(bulletin.master_table_number, 0)
        self.assertEqual(bulletin.data_category, 0)
        self.assertIsNone(bulletin.data_subcategory)
        self.assertEqual(bulletin.data_subcategory_local, 0)
        self.assertIsNone(bulletin.originating_centre)
        self.assertIsNone(bulletin.originating_subcentre)
        self.assertEqual(bulletin.update_sequence_number, 0)
        self.assertEqual(bulletin.edition_number, 1)
        self.assertEqual(bulletin.master_table_version_number, 3)
        self.assertEqual(bulletin.master_table_version_number_local, 0)
        self.assertEqual(bulletin.master_table_version_number_bufr, 0)
        self.assertFalse(bulletin.compression)
        self.assertFalse(bulletin.has_check_digit)
        self.assertIsNone(bulletin.optional_section)
        self.assertIsNone(bulletin.reftime)
        self.assertEqual(bulletin.datadesc, ("D07005", "B13023", "B13013"))

    def test_decode_crex_header(self) -> None:
        path = testdata_path / "crex" / "test-synop0.crex"
        decoded: list[wreport.Bulletin] = []
        with path.open("rb") as fd:
            for offset, data in wreport.read_crex(fd):
                decoded.append(
                    wreport.Bulletin.decode_crex_header(
                        data, fname=path, offset=offset
                    )
                )

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(bulletin.fname, path.as_posix())
        self.assertEqual(bulletin.offset, 0)
        self.assertEqual(len(bulletin), 0)
        self.assertCrexSynop0Header(bulletin)

    def test_decode_crex(self) -> None:
        path = testdata_path / "crex" / "test-synop0.crex"
        decoded: list[wreport.Bulletin] = []
        with path.open("rb") as fd:
            for offset, data in wreport.read_crex(fd):
                decoded.append(
                    wreport.Bulletin.decode_crex(
                        data, fname=path, offset=offset
                    )
                )

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(bulletin.fname, path.as_posix())
        self.assertEqual(bulletin.offset, 0)
        self.assertEqual(len(bulletin), 1)
        self.assertCrexSynop0Header(bulletin)
