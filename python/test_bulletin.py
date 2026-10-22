import wreport
import datetime as dt
import unittest
from pathlib import Path
from typing import ClassVar
import io
import os

testdata_path = Path(os.environ["WREPORT_TESTDATA"])


class TestCase(unittest.TestCase):
    maxDiff = None


class IterationTests(TestCase):
    def test_read_bufr_raw(self) -> None:
        path = testdata_path / "bufr" / "gts-buoy1.bufr"
        with path.open("rb") as fd:
            contents = list(wreport.read_bufr_raw(fd))
        self.assertEqual(contents, [(0, path.read_bytes())])

    def test_read_bufr_raw_short0(self) -> None:
        path = testdata_path / "bufr" / "short0.bufr"
        with path.open("rb") as fd:
            contents = list(wreport.read_bufr_raw(fd))
        self.assertEqual(contents, [])

    def test_read_bufr_raw_short1(self) -> None:
        path = testdata_path / "bufr" / "short1.bufr"
        with path.open("rb") as fd:
            with self.assertRaisesRegex(
                RuntimeError,
                r"cannot read BUFR section 0 \(4 bytes\)"
                r" from .+short1.bufr \(partial read\)",
            ):
                list(wreport.read_bufr_raw(fd))

    def test_read_bufr_raw_short2(self) -> None:
        path = testdata_path / "bufr" / "short2.bufr"
        with path.open("rb") as fd:
            with self.assertRaisesRegex(
                RuntimeError,
                r"cannot read BUFR message \(3618607 bytes\)"
                r" from .+short2.bufr \(partial read\)",
            ):
                list(wreport.read_bufr_raw(fd))

    def test_read_bufr_raw_short3(self) -> None:
        path = testdata_path / "bufr" / "short3.bufr"
        with path.open("rb") as fd:
            with self.assertRaisesRegex(
                RuntimeError,
                r"cannot read BUFR message \(647 bytes\)"
                r" from .+short3.bufr \(partial read\)",
            ):
                list(wreport.read_bufr_raw(fd))

    def test_read_crex_raw(self) -> None:
        path = testdata_path / "crex" / "test-synop0.crex"
        with path.open() as fd:
            contents = list(wreport.read_crex_raw(fd))
        self.assertEqual(contents, [(0, path.read_bytes().strip())])

    def test_read_crex_raw_truncated(self) -> None:
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
                    list(wreport.read_crex_raw(fd))


class SampleBufrTestCase(TestCase):
    sample_bufr = testdata_path / "bufr" / "MODE_12.bufr"

    sample_bufr_datadesc = (
        "D11010",
        "B25061",
        "B01015",
        "B01022",
        "B01065",
        "B33002",
    )

    def assertSampleBufrHeader(self, bulletin: wreport.Bulletin) -> None:
        self.assertEqual(bulletin.fname, self.sample_bufr.as_posix())
        self.assertEqual(bulletin.offset, 0)
        self.assertEqual(bulletin.master_table_number, 0)
        self.assertEqual(bulletin.data_category, 4)
        self.assertEqual(bulletin.data_subcategory, 2)
        self.assertEqual(bulletin.data_subcategory_local, 147)
        self.assertEqual(bulletin.originating_centre, 99)
        self.assertEqual(bulletin.originating_subcentre, 99)
        self.assertEqual(bulletin.update_sequence_number, 0)
        self.assertEqual(bulletin.edition_number, 4)
        self.assertEqual(bulletin.master_table_version_number, 33)
        self.assertEqual(bulletin.master_table_version_number_local, 0)
        self.assertIsNone(bulletin.master_table_version_number_bufr)
        self.assertTrue(bulletin.compression)
        self.assertFalse(bulletin.has_check_digit)
        self.assertIsNone(bulletin.optional_section)
        self.assertEqual(bulletin.reftime, dt.datetime(2022, 2, 14, 9, 0, 3))
        self.assertEqual(
            Path(bulletin.btable.pathname).name, "B0000000000000033000.txt"
        )
        self.assertEqual(bulletin.datadesc, self.sample_bufr_datadesc)


class DecodeTests(SampleBufrTestCase):
    sample_crex = testdata_path / "crex" / "test-synop0.crex"

    def assertSampleCrexHeader(self, bulletin: wreport.Bulletin) -> None:
        self.assertEqual(bulletin.fname, self.sample_crex.as_posix())
        self.assertEqual(bulletin.offset, 0)
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

    def test_decode_raw_bufr_header(self) -> None:
        decoded: list[wreport.Bulletin] = []
        with self.sample_bufr.open("rb") as fd:
            for offset, data in wreport.read_bufr_raw(fd):
                decoded.append(
                    wreport.Bulletin.decode_bufr_header(
                        data, fname=self.sample_bufr, offset=offset
                    )
                )

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(len(bulletin), 0)
        self.assertSampleBufrHeader(bulletin)

    def test_decode_raw_bufr(self) -> None:
        decoded: list[wreport.Bulletin] = []
        with self.sample_bufr.open("rb") as fd:
            for offset, data in wreport.read_bufr_raw(fd):
                decoded.append(
                    wreport.Bulletin.decode_bufr(
                        data, fname=self.sample_bufr, offset=offset
                    )
                )

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(len(bulletin), 14)
        self.assertEqual(len(bulletin[0]), 44)
        self.assertSampleBufrHeader(bulletin)

    def test_decode_bufr_header(self) -> None:
        decoded: list[wreport.Bulletin] = []
        with self.sample_bufr.open("rb") as fd:
            decoded = list(wreport.read_bufr_headers(fd))

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(len(bulletin), 0)
        self.assertSampleBufrHeader(bulletin)

    def test_decode_bufr(self) -> None:
        decoded: list[wreport.Bulletin] = []
        with self.sample_bufr.open("rb") as fd:
            decoded = list(wreport.read_bufr(fd))

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(len(bulletin), 14)
        self.assertEqual(len(bulletin[0]), 44)
        self.assertSampleBufrHeader(bulletin)

    def test_decode_raw_crex_header(self) -> None:
        decoded: list[wreport.Bulletin] = []
        with self.sample_crex.open("rb") as fd:
            for offset, data in wreport.read_crex_raw(fd):
                decoded.append(
                    wreport.Bulletin.decode_crex_header(
                        data, fname=self.sample_crex, offset=offset
                    )
                )

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(len(bulletin), 0)
        self.assertSampleCrexHeader(bulletin)

    def test_decode_raw_crex(self) -> None:
        decoded: list[wreport.Bulletin] = []
        with self.sample_crex.open("rb") as fd:
            for offset, data in wreport.read_crex_raw(fd):
                decoded.append(
                    wreport.Bulletin.decode_crex(
                        data, fname=self.sample_crex, offset=offset
                    )
                )

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(len(bulletin), 1)
        self.assertEqual(len(bulletin[0]), 49)
        self.assertSampleCrexHeader(bulletin)

    def test_decode_crex_header(self) -> None:
        decoded: list[wreport.Bulletin] = []
        with self.sample_crex.open("rb") as fd:
            decoded = list(wreport.read_crex_headers(fd))

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(len(bulletin), 0)
        self.assertSampleCrexHeader(bulletin)

    def test_decode_crex(self) -> None:
        decoded: list[wreport.Bulletin] = []
        with self.sample_crex.open("rb") as fd:
            decoded = list(wreport.read_crex(fd))

        self.assertEqual(len(decoded), 1)
        bulletin = decoded[0]
        self.assertIsInstance(bulletin, wreport.Bulletin)

        self.assertEqual(len(bulletin), 1)
        self.assertEqual(len(bulletin[0]), 49)
        self.assertSampleCrexHeader(bulletin)


class BulletinTestCase(SampleBufrTestCase):
    bulletin: ClassVar[wreport.Bulletin]

    @classmethod
    def read_bulletin(cls) -> wreport.Bulletin:
        with cls.sample_bufr.open("rb") as fd:
            return list(wreport.read_bufr(fd))[0]

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()
        cls.bulletin = cls.read_bulletin()


class BulletinTests(BulletinTestCase):
    def test_str(self) -> None:
        fname_escaped = (
            self.sample_bufr.as_posix().encode("unicode-escape").decode()
        )
        self.assertEqual(str(self.bulletin), f"BUFR@{fname_escaped}+0")

    def test_repr(self) -> None:
        fname_escaped = (
            self.sample_bufr.as_posix().encode("unicode-escape").decode()
        )
        self.assertEqual(
            repr(self.bulletin),
            "wreport.Bulletin("
            f'type="BUFR", fname="{fname_escaped}", offset=0)',
        )

    def test_subset_access(self) -> None:
        self.assertEqual(self.bulletin[-1], self.bulletin[13])
        with self.assertRaisesRegex(IndexError, "invalid subset index"):
            self.bulletin[14]
        with self.assertRaisesRegex(IndexError, "invalid subset index"):
            self.bulletin[-15]

    def test_change_datadesc(self) -> None:
        bulletin = self.read_bulletin()
        orig_datadesc = bulletin.datadesc
        updated_datadesc = ("B01150",) + orig_datadesc
        bulletin.datadesc = updated_datadesc
        self.assertEqual(bulletin.datadesc, updated_datadesc)

    def test_iterate_subsets(self) -> None:
        count = 0
        for subset in self.bulletin:
            self.assertIsInstance(subset, wreport.Subset)
            count += 1
        self.assertEqual(count, 14)

    def test_var(self) -> None:
        var = self.bulletin.var("B01128", "Test value")
        self.assertEqual(var.code, "B01128")
        self.assertEqual(var.format(), "Test value")

        var = self.bulletin.var("B01128")
        self.assertEqual(var.code, "B01128")
        self.assertFalse(var.isset)

        var = self.bulletin.var("B01128", None)
        self.assertEqual(var.code, "B01128")
        self.assertFalse(var.isset)


class SubsetTests(BulletinTestCase):
    subset: ClassVar[wreport.Subset]

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()
        cls.subset = cls.bulletin[0]

    def test_index_checks(self) -> None:
        with self.assertRaisesRegex(IndexError, r"invalid var index"):
            self.subset[len(self.subset)]
        with self.assertRaisesRegex(IndexError, r"invalid var index"):
            self.subset[-len(self.subset) - 1]
        with self.assertRaisesRegex(
            TypeError, r"invalid subscript 'test': expecting int or slice"
        ):
            self.subset["test"]
        with self.assertRaisesRegex(ValueError, r"slice step cannot be zero"):
            self.subset[::0]

    def test_index(self) -> None:
        self.assertEqual(self.subset[0].code, "B01008")
        self.assertEqual(self.subset[-1].code, "B33002")

    def test_iterate_vars(self) -> None:
        count = 0
        for var in self.subset:
            self.assertIsInstance(var, wreport.Var)
            count += 1
        self.assertEqual(count, 44)

    def test_subset_slice(self) -> None:
        varcodes = tuple(var.code for var in self.subset)

        self.assertEqual(
            tuple(var.code for var in self.subset[0:3]), varcodes[0:3]
        )
        self.assertEqual(
            tuple(var.code for var in self.subset[0:4:2]), varcodes[0:4:2]
        )
        self.assertEqual(
            tuple(var.code for var in self.subset[::-1]), varcodes[::-1]
        )
        self.assertEqual(
            tuple(var.code for var in self.subset[::3]), varcodes[::3]
        )
        self.assertEqual(
            tuple(var.code for var in self.subset[::-3]), varcodes[::-3]
        )
        self.assertEqual(self.subset[3:3], ())

    def test_subset_str(self) -> None:
        fname_escaped = (
            self.sample_bufr.as_posix().encode("unicode-escape").decode()
        )
        self.assertEqual(str(self.subset), f"BUFR@{fname_escaped}+0[0]")

    def test_subset_repr(self) -> None:
        fname_escaped = (
            self.sample_bufr.as_posix().encode("unicode-escape").decode()
        )
        self.assertEqual(
            repr(self.subset),
            f'wreport.Bulletin(type="BUFR", fname="{fname_escaped}", offset=0)'
            "[0]",
        )

    def test_assign_checks(self) -> None:
        bulletin = self.read_bulletin()
        subset = bulletin[0]

        with self.assertRaisesRegex(IndexError, r"invalid var index"):
            subset[len(subset)] = subset[0]
        with self.assertRaisesRegex(IndexError, r"invalid var index"):
            subset[-len(subset) - 1] = subset[0]
        with self.assertRaisesRegex(
            TypeError, r"variable to assign is 42: expected wreport.Var"
        ):
            subset[0] = 42
        with self.assertRaisesRegex(
            TypeError, r"variable to assign is 42: expected wreport.Var"
        ):
            subset[0] = 42
        with self.assertRaisesRegex(
            TypeError, r"invalid subscript 'test': expecting int or slice"
        ):
            subset["test"] = subset[0]

    def test_assign(self) -> None:
        bulletin = self.read_bulletin()
        subset = bulletin[0]
        subset[0] = subset[0]
        subset[0:2] = subset[0:2]
        subset[-1] = subset[-1]

        self.assertEqual(len(subset), len(self.subset))
        for pos in range(len(subset)):
            with self.subTest(pos=pos):
                self.assertEqual(subset[pos], self.subset[pos])

    def test_assign_empty_subset(self) -> None:
        bulletin = self.read_bulletin()
        subset = bulletin[0]
        subset[::] = []
        self.assertEqual(len(subset), 0)

    def test_assign_delete_items(self) -> None:
        varcodes = tuple(var.code for var in self.subset)

        bulletin = self.read_bulletin()
        subset = bulletin[0]
        subset[10:20] = []
        self.assertEqual(len(subset), len(self.subset) - 10)

        self.assertEqual(
            tuple(var.code for var in subset), varcodes[0:10] + varcodes[20:]
        )

    def test_assign_insert_items(self) -> None:
        varcodes = tuple(var.code for var in self.subset)

        bulletin = self.read_bulletin()
        subset = bulletin[0]
        subset[0:0] = subset[0:10]
        self.assertEqual(len(subset), len(self.subset) + 10)

        self.assertEqual(
            tuple(var.code for var in subset), varcodes[0:10] + varcodes
        )

    def test_assign_replace_sequence_with_shorter(self) -> None:
        varcodes = tuple(var.code for var in self.subset)

        bulletin = self.read_bulletin()
        subset = bulletin[0]
        subset[7:15] = subset[13:15]
        self.assertEqual(len(subset), len(self.subset) - 6)

        self.assertEqual(
            tuple(var.code for var in subset), varcodes[0:7] + varcodes[13:]
        )

    def test_assign_replace_sequence_with_longer(self) -> None:
        varcodes = tuple(var.code for var in self.subset)

        bulletin = self.read_bulletin()
        subset = bulletin[0]
        subset[13:15] = subset[7:13]
        self.assertEqual(len(subset), len(self.subset) + 4)

        self.assertEqual(
            tuple(var.code for var in subset),
            varcodes[0:13] + varcodes[7:13] + varcodes[15:],
        )

    def test_var(self) -> None:
        subset = self.bulletin[0]
        var = subset.var("B01128", "Test value")
        self.assertEqual(var.code, "B01128")
        self.assertEqual(var.format(), "Test value")

        var = self.bulletin.var("B01128")
        self.assertEqual(var.code, "B01128")
        self.assertFalse(var.isset)

        var = self.bulletin.var("B01128", None)
        self.assertEqual(var.code, "B01128")
        self.assertFalse(var.isset)
