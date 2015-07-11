#include <test-utils-wreport.h>
#include <functional>

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

typedef tests::TestCodec<BufrBulletin> TestBufr;

std::vector<Test> make_tests()
{
    std::vector<Test> res;

    res.emplace_back("bufr/corrupted.bufr", [](Fixture& f) {
        // Corrupted BUFR

        // Read the whole contents of the test file
        std::string raw1 = tests::slurpfile("bufr/corrupted.bufr");

        // Decode the original contents
        unique_ptr<BufrBulletin> msg1(BufrBulletin::create());
        try {
            msg1->decode(raw1, "bufr/corrupted.bufr");
        } catch (std::exception& e) {
            ensure_contains(e.what(), "Only BUFR edition 2, 3, and 4 are supported (this message is edition 47)");
        }
    });

    auto _declare_test = [&](WIBBLE_TEST_LOCPRM, std::string fname, std::function<void(wibble::tests::Location, const BufrBulletin&)> check_contents) {
        res.emplace_back(fname, [=](Fixture& f) {
            TestBufr test(fname);
            test.check_contents = check_contents;
            wruntest(test.run);
        });
    };

    #define declare_test(fname, check_contents) wruntest(_declare_test, fname, check_contents)


    declare_test("bufr/bufr1", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 1);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 21);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 35u);

        ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
        ensure_equals(s[9].enqd(), 68.27);
        ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
        ensure_equals(s[10].enqd(),  9.68);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/bufr2", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 1);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 21);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 35u);

        ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
        ensure_equals(s[9].enqd(), 43.02);
        ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
        ensure_equals(s[10].enqd(), -12.45);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs0-1.22.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 1);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 52u);

        ensure_varcode_equals(s[27].code(), WR_VAR(0, 20, 13));
        ensure_equals(s[27].enqd(), 250.0);
        ensure_varcode_equals(s[34].code(), WR_VAR(0, 20, 13));
        ensure_equals(s[34].enqd(), 320.0);
        ensure_varcode_equals(s[38].code(), WR_VAR(0, 20, 13));
        ensure_equals(s[38].enqd(), 620.0);
        ensure_varcode_equals(s[42].code(), WR_VAR(0, 20, 13));
        ensure_equals(s[42].enqd(), 920.0);
        ensure_varcode_equals(s[46].code(), WR_VAR(0, 20, 13));
        wassert(actual(s[46].isset()).isfalse());

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs0-3.504.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 3);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 52u);

        ensure_varcode_equals(s[28].code(), WR_VAR(0, 20, 12));
        ensure_equals(s[28].enqd(), 37.0);
        ensure_varcode_equals(s[29].code(), WR_VAR(0, 20, 12));
        ensure_equals(s[29].enqd(), 22.0);
        ensure_varcode_equals(s[30].code(), WR_VAR(0, 20, 12));
        ensure_equals(s[30].enqd(), 60.0);
        ensure_varcode_equals(s[33].code(), WR_VAR(0, 20, 12));
        ensure_equals(s[33].enqd(),  7.0);
        ensure_varcode_equals(s[37].code(), WR_VAR(0, 20, 12));
        ensure_equals(s[37].enqd(),  5.0);
        ensure_varcode_equals(s[41].code(), WR_VAR(0, 20, 12));
        wassert(actual(s[41].isset()).isfalse());
        ensure_varcode_equals(s[45].code(), WR_VAR(0, 20, 12));
        wassert(actual(s[45].isset()).isfalse());

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs1-9.2.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 1);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 9);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 37u);

        ensure_varcode_equals(s[0].code(), WR_VAR(0, 1, 11));
        ensure_equals(string(s[0].enqc()), "DFPC");

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs1-11.16.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 1);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 11);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 37u);

        ensure_varcode_equals(s[33].code(), WR_VAR(0, 10, 197));
        ensure_equals(s[33].enqd(), 46.0);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs1-13.36.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 1);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 13);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 37u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs1-19.3.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 1);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 19);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 37u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/synop-old-buoy.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 1);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 21);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 35u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs2-101.16.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 2);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 101);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 619u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs2-102.1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 2);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 102);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 403u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[5].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[5].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs2-91.2.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 2);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 91);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 127u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[5].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[5].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/airep-old-4-142.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 4);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 142);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 21u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[5].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[5].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs4-144.4.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 4);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 144);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 21u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[5].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[5].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs4-145.4.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 4);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 145);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 31u);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[5].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[5].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/obs3-3.1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 3);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 3);
        ensure_equals(msg.subsets.size(), 180);

        ensure_equals(msg.subset(0).size(), 127u);
        ensure_equals(msg.subset(1).size(), 127u);
        ensure_equals(msg.subset(2).size(), 127u);
        ensure_equals(msg.subset(179).size(), 127u);
    });

    declare_test("bufr/obs3-56.2.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 3);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 56);
        ensure_equals(msg.subsets.size(), 35u);

        ensure_equals(msg.subset(0).size(), 225u);
        ensure_equals(msg.subset(1).size(), 225u);
        ensure_equals(msg.subset(2).size(), 225u);
        ensure_equals(msg.subset(34).size(), 225u);
    });

    declare_test("bufr/crex-has-few-digits.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        /*
         * In this case, the ECMWF table has 12 bits for BUFR in Kelvin (up to 409.6)
         * but 3 digits for CREX in Celsius (up to 99.0).  This means that BUFR can
         * encode values too big to fit in CREX, and when it happens dba_var range
         * checks kick in and abort decoding.
         */
    });

    declare_test("bufr/test-buoy1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // Buoy who could not look up a D table
    });

    declare_test("bufr/ed4.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 8);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 171);
        ensure_equals(msg.subsets.size(), 128u);

        ensure_equals(msg.subset(0).size(), 26u);
        ensure_equals(msg.subset(1).size(), 26u);
        ensure_equals(msg.subset(2).size(), 26u);
        ensure_equals(msg.subset(127).size(), 26u);
    });

    declare_test("bufr/ed4date.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 8);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 171);
        ensure_equals(msg.subsets.size(), 128u);

        ensure_equals(msg.rep_year, 2000);
        ensure_equals(msg.rep_month, 1);
        ensure_equals(msg.rep_day, 2);
        ensure_equals(msg.rep_hour, 7);
        ensure_equals(msg.rep_minute, 0);
        ensure_equals(msg.rep_second, 0);

        ensure_equals(msg.subset(0).size(), 26u);
        ensure_equals(msg.subset(1).size(), 26u);
        ensure_equals(msg.subset(2).size(), 26u);
        ensure_equals(msg.subset(127).size(), 26u);
    });

    declare_test("bufr/ed2radar.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 2);
        ensure_equals(msg.data_category, 6);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 0);
        ensure_equals(msg.subsets.size(), 1u);

        ensure_equals(msg.rep_year, 2007);
        ensure_equals(msg.rep_month, 8);
        ensure_equals(msg.rep_day, 13);
        ensure_equals(msg.rep_hour, 18);
        ensure_equals(msg.rep_minute, 30);
        ensure_equals(msg.rep_second, 0);

        ensure_equals(msg.subset(0).size(), 4606u);
    });

    declare_test("bufr/ed4-compr-string.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // BUFR4 with compressed strings
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 2);
        ensure_equals(msg.data_subcategory_local, 0);
        ensure_equals(msg.subsets.size(), 5u);

        ensure_equals(msg.rep_year, 2009);
        ensure_equals(msg.rep_month, 12);
        ensure_equals(msg.rep_day, 3);
        ensure_equals(msg.rep_hour, 3);
        ensure_equals(msg.rep_minute, 0);
        ensure_equals(msg.rep_second, 0);

        ensure_equals(msg.subset(0).size(), 115u);
        ensure_equals(msg.subset(1).size(), 115u);
        ensure_equals(msg.subset(2).size(), 115u);
        ensure_equals(msg.subset(3).size(), 115u);
        ensure_equals(msg.subset(4).size(), 115u);
    });

    declare_test("bufr/ed4-parseerror1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // BUFR4 which gives a parse error
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 1);
        ensure_equals(msg.data_subcategory_local, 255);
        ensure_equals(msg.subsets.size(), 5u);

        ensure_equals(msg.rep_year, 2009);
        ensure_equals(msg.rep_month, 12);
        ensure_equals(msg.rep_day, 3);
        ensure_equals(msg.rep_hour, 3);
        ensure_equals(msg.rep_minute, 0);
        ensure_equals(msg.rep_second, 0);

        ensure_equals(msg.subset(0).size(), 107u);
        ensure_equals(msg.subset(1).size(), 107u);
        ensure_equals(msg.subset(2).size(), 107u);
        ensure_equals(msg.subset(3).size(), 107u);
        ensure_equals(msg.subset(4).size(), 107u);
    });

    declare_test("bufr/ed4-empty.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // BUFR4 which does not give a parse error but looks empty
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 1);
        ensure_equals(msg.data_subcategory_local, 0);
        ensure_equals(msg.subsets.size(), 7u);

        ensure_equals(msg.rep_year, 2009);
        ensure_equals(msg.rep_month, 12);
        ensure_equals(msg.rep_day, 3);
        ensure_equals(msg.rep_hour, 3);
        ensure_equals(msg.rep_minute, 0);
        ensure_equals(msg.rep_second, 0);

        ensure_equals(msg.subset(0).size(), 120u);
        ensure_equals(msg.subset(1).size(), 120u);
        ensure_equals(msg.subset(2).size(), 120u);
        ensure_equals(msg.subset(3).size(), 120u);
        ensure_equals(msg.subset(4).size(), 120u);
        ensure_equals(msg.subset(5).size(), 120u);
        ensure_equals(msg.subset(6).size(), 120u);
    });

    declare_test("bufr/C05060.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // GTS temp message
    });

#if 0
    declare_test("bufr/tempforecast.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // Custom ARPA temp forecast message saved as ARPA generic
        // TODO: we cannot test this unless we ship dballe's tables
        //       reenable after deciding whether to ship them or not
    });
#endif

    declare_test("bufr/C23000.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // C23000 modifier
    });

    declare_test("bufr/segfault1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // BUFR that gave segfault
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 2);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 101);
        ensure_equals(msg.subsets.size(), 1);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 129u);

        ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 1));
        ensure_equals(s[9].enqd(), 41.65);
        ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 1));
        ensure_equals(s[10].enqd(), 12.43);

        ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

        ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
    });

    declare_test("bufr/C08022.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // C08xxx modifier
    });

    declare_test("bufr/C23000-1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // C23xxx modifier on a message that gave problems in some machine
    });

    declare_test("bufr/C08032-toolong.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
    });

    declare_test("bufr/synop-longname.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // Synop with a very long station name
    });

    declare_test("bufr/C04004.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // C04yyy modifier, B32021=6
    });

    declare_test("bufr/C04-B31021-1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // C04yyy modifier, B32021=1
    });

    declare_test("bufr/C06006.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // C06006 modifier
    });

    declare_test("bufr/gps_zenith.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 14);
        ensure_equals(msg.subsets.size(), 94);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 175u);

        ensure_varcode_equals(s[0].code(), WR_VAR(0, 1, 15));
        ensure_equals(s[0].enq<string>(), "AQUI-BKG_");
    });

    declare_test("bufr/ascat1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 12);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 223);
        ensure_equals(msg.subsets.size(), 1722);

        const Subset& s = msg.subset(0);
        ensure_equals(s.size(), 124u);

        ensure_varcode_equals(s[0].code(), WR_VAR(0, 1, 33));
        ensure_equals(s[0].enq<int>(), 254);
    });

    declare_test("bufr/unparsable1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
    });

    declare_test("bufr/C04type21.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
    });

    declare_test("bufr/noassoc.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // Buffer with a seemingly missing associated field significance
    });

    declare_test("bufr/atms1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // ATMS data (http://npp.gsfc.nasa.gov/atms.html)
    });

    declare_test("bufr/atms2.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // ATMS data (http://npp.gsfc.nasa.gov/atms.html)
    });

    declare_test("bufr/table17.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        // BUFR data using table 17
    });

    declare_test("bufr/A_ISMN02LFPW080000RRA_C_RJTD_20140808000319_100.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
    });

    declare_test("bufr/bitmap-B33035.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
    });

    declare_test("bufr/gts-buoy1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 3);
        ensure_equals(msg.data_category, 1);
        ensure_equals(msg.data_subcategory, 255);
        ensure_equals(msg.data_subcategory_local, 0);
        ensure_equals(msg.subsets.size(), 1);
    });

    declare_test("bufr/gts-synop-rad1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 1);
        ensure_equals(msg.data_subcategory_local, 0);
        ensure_equals(msg.subsets.size(), 25);
    });

    declare_test("bufr/gts-synop-rad2.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 6);
        ensure_equals(msg.data_subcategory_local, 150);
        ensure_equals(msg.subsets.size(), 1);
    });

    declare_test("bufr/gts-synop-tchange.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 0);
        ensure_equals(msg.data_subcategory, 1);
        ensure_equals(msg.data_subcategory_local, 0);
        ensure_equals(msg.subsets.size(), 1);
    });

    declare_test("bufr/new-003.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
        ensure_equals(msg.edition_number, 4);
        ensure_equals(msg.data_category, 2);
        ensure_equals(msg.data_subcategory, 10);
        ensure_equals(msg.data_subcategory_local, 1);
        ensure_equals(msg.subsets.size(), 1);
    });

    return res;
}

#if 0
template<> template<>
void to::test<3>()
{
#if 0
	*** Disabled because this test data uses a template that we do not support

	TestBufrexRaw test;
	test.setEdition(3);
	test.setCat(1);
	test.setSubcat(12);
	test.setVars(119);
	test.set(WR_VAR(0, 5, 2), 54.10);
	test.set(WR_VAR(0, 6, 2), 12.10);

	bufrex_msg msg = read_test_msg_raw("bufr/bufr3", BUFR);
	ensureBufrexRawEquals(test, msg);

	bufrex_msg msg1 = reencode_test(msg, BUFR);
	ensureBufrexRawEquals(test, msg1);

	bufrex_msg_delete(msg);
	bufrex_msg_delete(msg1);
#endif
}

// Soil temperature message
template<> template<>
void to::test<24>()
{
#if 0
	// TODO: this seems to be a problem in the input message.
	//       we can disregard it until someone proves it's our problem and not
	//       the problem of who generated the message

	/*
	TestBufrexMsg test;
	test.edition = 2;
	test.cat = 6;
	test.subcat = 255;
	test.localsubcat = 0;
	test.subsets = 1;
	*/

	//bufrex_msg msg = read_test_msg_header_raw("bufr/test-soil1.bufr", BUFR);
	/*
	ensureBufrexRawEquals(test, msg);
	ensure_equals(msg->rep_year, 2007);
	ensure_equals(msg->rep_month, 8);
	ensure_equals(msg->rep_day, 13);
	ensure_equals(msg->rep_hour, 18);
	ensure_equals(msg->rep_minute, 30);
	ensure_equals(msg->rep_second, 0);

	test.subset(0).vars = 4606;
	*/

	//bufrex_msg msg1 = reencode_test(msg);
	//ensureBufrexRawEquals(test, msg1);

	struct Tester : public MsgTester {
		void test(const BufrBulletin& msg)
		{
			/*
			ensure_equals(msg.edition_number, 2);
			ensure_equals(msg.data_category, 6);
			ensure_equals(msg.data_subcategory, 255);
			ensure_equals(msg.data_subcategory_local, 0);
			ensure_equals(msg.subsets.size(), 1u);

			ensure_equals(msg.rep_year, 2007);
			ensure_equals(msg.rep_month, 8);
			ensure_equals(msg.rep_day, 13);
			ensure_equals(msg.rep_hour, 18);
			ensure_equals(msg.rep_minute, 30);
			ensure_equals(msg.rep_second, 0);

			ensure_equals(msg.subset(0).size(), 4606u);

			const Subset& s = msg.subset(0);

			// FIXME Does it have this?
			ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
			ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

			ensure(s[5].enqa(WR_VAR(0, 33, 7)) != NULL);
			ensure_equals(s[5].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
			*/
		}
	} test;

	// FIXME: recoding might not work
	declare_test("bufr/test-soil1.bufr", [](WIBBLE_TEST_LOCPRM, const BufrBulletin& msg) {
#endif
}

#endif

test_group newtg("bufr_decoder", make_tests);

}
