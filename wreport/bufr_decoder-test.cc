#include "tests.h"
#include <functional>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

typedef tests::TestCodec<BufrBulletin> TestBufr;

class Tests : public TestCase
{
    using TestCase::TestCase;

    void declare_test(std::string fname, std::function<void(const BufrBulletin&)> check_contents)
    {
        add_method(fname, [=]() {
            TestBufr test(fname);
            test.check_contents = check_contents;
            wassert(test.run());
        });
    };

    void declare_test_decode_fail(std::string fname, std::string errmsg)
    {
        add_method(fname, [=]() {
            // Read the whole contents of the test file
            std::string raw = wcallchecked(slurpfile(fname));

            bool failed = true;
            try {
                BufrBulletin::decode(raw, fname.c_str());
                failed = false;
            } catch (std::exception& e) {
                wassert(actual(e.what()).contains(errmsg));
            }
            wassert(actual(failed).istrue());
        });
    }

    void register_tests() override
    {
        add_method("bufr/corrupted.bufr", []() {
            // Corrupted BUFR

            // Read the whole contents of the test file
            std::string raw1 = tests::slurpfile("bufr/corrupted.bufr");

            // Decode the original contents
            try {
                BufrBulletin::decode(raw1, "bufr/corrupted.bufr");
            } catch (std::exception& e) {
                wassert(actual(e.what()).contains("Only BUFR edition 2, 3, and 4 are supported (this message is edition 47)"));
            }
        });

        declare_test("bufr/bufr1", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 1);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 21);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 35u);

            wassert(actual_varcode(s[9].code()) == WR_VAR(0, 5, 2));
            wassert(actual(s[9].enqd()) == 68.27);
            wassert(actual_varcode(s[10].code()) == WR_VAR(0, 6, 2));
            wassert(actual(s[10].enqd()) ==  9.68);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(msg.section_end[0]) == 8u);
            wassert(actual(msg.section_end[1]) == 26u);
            wassert(actual(msg.section_end[2]) == 78u);
            wassert(actual(msg.section_end[3]) == 102u);
            wassert(actual(msg.section_end[4]) == 178u);
            wassert(actual(msg.section_end[5]) == 182u);
        });

        declare_test("bufr/bufr2", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 1);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 21);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 35u);

            wassert(actual_varcode(s[9].code()) == WR_VAR(0, 5, 2));
            wassert(actual(s[9].enqd()) == 43.02);
            wassert(actual_varcode(s[10].code()) == WR_VAR(0, 6, 2));
            wassert(actual(s[10].enqd()) == -12.45);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs0-1.22.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 1);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 52u);

            wassert(actual_varcode(s[27].code()) == WR_VAR(0, 20, 13));
            wassert(actual(s[27].enqd()) == 250.0);
            wassert(actual_varcode(s[34].code()) == WR_VAR(0, 20, 13));
            wassert(actual(s[34].enqd()) == 320.0);
            wassert(actual_varcode(s[38].code()) == WR_VAR(0, 20, 13));
            wassert(actual(s[38].enqd()) == 620.0);
            wassert(actual_varcode(s[42].code()) == WR_VAR(0, 20, 13));
            wassert(actual(s[42].enqd()) == 920.0);
            wassert(actual_varcode(s[46].code()) == WR_VAR(0, 20, 13));
            wassert(actual(s[46].isset()).isfalse());

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs0-3.504.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 3);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 52u);

            wassert(actual_varcode(s[28].code()) == WR_VAR(0, 20, 12));
            wassert(actual(s[28].enqd()) == 37.0);
            wassert(actual_varcode(s[29].code()) == WR_VAR(0, 20, 12));
            wassert(actual(s[29].enqd()) == 22.0);
            wassert(actual_varcode(s[30].code()) == WR_VAR(0, 20, 12));
            wassert(actual(s[30].enqd()) == 60.0);
            wassert(actual_varcode(s[33].code()) == WR_VAR(0, 20, 12));
            wassert(actual(s[33].enqd()) ==  7.0);
            wassert(actual_varcode(s[37].code()) == WR_VAR(0, 20, 12));
            wassert(actual(s[37].enqd()) ==  5.0);
            wassert(actual_varcode(s[41].code()) == WR_VAR(0, 20, 12));
            wassert(actual(s[41].isset()).isfalse());
            wassert(actual_varcode(s[45].code()) == WR_VAR(0, 20, 12));
            wassert(actual(s[45].isset()).isfalse());

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs1-9.2.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 1);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 9);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 37u);

            wassert(actual_varcode(s[0].code()) == WR_VAR(0, 1, 11));
            wassert(actual(string(s[0].enqc())) == "DFPC");

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs1-11.16.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 1);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 11);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 37u);

            wassert(actual_varcode(s[33].code()) == WR_VAR(0, 10, 197));
            wassert(actual(s[33].enqd()) == 46.0);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs1-13.36.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 1);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 13);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 37u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs1-19.3.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 1);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 19);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 37u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/synop-old-buoy.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 1);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 21);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 35u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs2-101.16.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 2);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 101);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 619u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs2-102.1.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 2);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 102);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 403u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs2-91.2.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 2);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 91);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 127u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/airep-old-4-142.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 4);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 142);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 21u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs4-144.4.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 4);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 144);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 21u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs4-145.4.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 4);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 145);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 31u);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/obs3-3.1.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 3);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 3);
            wassert(actual(msg.subsets.size()) == 180u);

            wassert(actual(msg.subset(0).size()) == 127u);
            wassert(actual(msg.subset(1).size()) == 127u);
            wassert(actual(msg.subset(2).size()) == 127u);
            wassert(actual(msg.subset(179).size()) == 127u);
        });

        declare_test("bufr/obs3-56.2.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 3);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 56);
            wassert(actual(msg.subsets.size()) == 35u);

            wassert(actual(msg.subset(0).size()) == 225u);
            wassert(actual(msg.subset(1).size()) == 225u);
            wassert(actual(msg.subset(2).size()) == 225u);
            wassert(actual(msg.subset(34).size()) == 225u);
        });

        declare_test("bufr/crex-has-few-digits.bufr", [](const BufrBulletin& msg) {
            /*
             * In this case, the ECMWF table has 12 bits for BUFR in Kelvin (up to 409.6)
             * but 3 digits for CREX in Celsius (up to 99.0).  This means that BUFR can
             * encode values too big to fit in CREX, and when it happens dba_var range
             * checks kick in and abort decoding.
             */
        });

        declare_test("bufr/test-buoy1.bufr", [](const BufrBulletin& msg) {
            // Buoy who could not look up a D table
        });

        declare_test("bufr/ed4.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 8);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 171);
            wassert(actual(msg.subsets.size()) == 128u);

            wassert(actual(msg.subset(0).size()) == 26u);
            wassert(actual(msg.subset(1).size()) == 26u);
            wassert(actual(msg.subset(2).size()) == 26u);
            wassert(actual(msg.subset(127).size()) == 26u);
        });

        declare_test("bufr/ed4date.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 8);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 171);
            wassert(actual(msg.subsets.size()) == 128u);

            wassert(actual(msg.rep_year) == 2000);
            wassert(actual(msg.rep_month) == 1);
            wassert(actual(msg.rep_day) == 2);
            wassert(actual(msg.rep_hour) == 7);
            wassert(actual(msg.rep_minute) == 0);
            wassert(actual(msg.rep_second) == 0);

            wassert(actual(msg.subset(0).size()) == 26u);
            wassert(actual(msg.subset(1).size()) == 26u);
            wassert(actual(msg.subset(2).size()) == 26u);
            wassert(actual(msg.subset(127).size()) == 26u);
        });

        declare_test("bufr/ed2radar.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 2);
            wassert(actual(msg.data_category) == 6);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 0);
            wassert(actual(msg.subsets.size()) == 1u);

            wassert(actual(msg.rep_year) == 2007);
            wassert(actual(msg.rep_month) == 8);
            wassert(actual(msg.rep_day) == 13);
            wassert(actual(msg.rep_hour) == 18);
            wassert(actual(msg.rep_minute) == 30);
            wassert(actual(msg.rep_second) == 0);

            wassert(actual(msg.subset(0).size()) == 4606u);
        });

        declare_test("bufr/ed4-compr-string.bufr", [](const BufrBulletin& msg) {
            // BUFR4 with compressed strings
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 2);
            wassert(actual(msg.data_subcategory_local) == 0);
            wassert(actual(msg.subsets.size()) == 5u);

            wassert(actual(msg.rep_year) == 2009);
            wassert(actual(msg.rep_month) == 12);
            wassert(actual(msg.rep_day) == 3);
            wassert(actual(msg.rep_hour) == 3);
            wassert(actual(msg.rep_minute) == 0);
            wassert(actual(msg.rep_second) == 0);

            wassert(actual(msg.subset(0).size()) == 115u);
            wassert(actual(msg.subset(1).size()) == 115u);
            wassert(actual(msg.subset(2).size()) == 115u);
            wassert(actual(msg.subset(3).size()) == 115u);
            wassert(actual(msg.subset(4).size()) == 115u);
        });

        declare_test("bufr/ed4-parseerror1.bufr", [](const BufrBulletin& msg) {
            // BUFR4 which gives a parse error
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 1);
            wassert(actual(msg.data_subcategory_local) == 255);
            wassert(actual(msg.subsets.size()) == 5u);

            wassert(actual(msg.rep_year) == 2009);
            wassert(actual(msg.rep_month) == 12);
            wassert(actual(msg.rep_day) == 3);
            wassert(actual(msg.rep_hour) == 3);
            wassert(actual(msg.rep_minute) == 0);
            wassert(actual(msg.rep_second) == 0);

            wassert(actual(msg.subset(0).size()) == 107u);
            wassert(actual(msg.subset(1).size()) == 107u);
            wassert(actual(msg.subset(2).size()) == 107u);
            wassert(actual(msg.subset(3).size()) == 107u);
            wassert(actual(msg.subset(4).size()) == 107u);
        });

        declare_test("bufr/ed4-empty.bufr", [](const BufrBulletin& msg) {
            // BUFR4 which does not give a parse error but looks empty
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 1);
            wassert(actual(msg.data_subcategory_local) == 0);
            wassert(actual(msg.subsets.size()) == 7u);

            wassert(actual(msg.rep_year) == 2009);
            wassert(actual(msg.rep_month) == 12);
            wassert(actual(msg.rep_day) == 3);
            wassert(actual(msg.rep_hour) == 3);
            wassert(actual(msg.rep_minute) == 0);
            wassert(actual(msg.rep_second) == 0);

            wassert(actual(msg.subset(0).size()) == 120u);
            wassert(actual(msg.subset(1).size()) == 120u);
            wassert(actual(msg.subset(2).size()) == 120u);
            wassert(actual(msg.subset(3).size()) == 120u);
            wassert(actual(msg.subset(4).size()) == 120u);
            wassert(actual(msg.subset(5).size()) == 120u);
            wassert(actual(msg.subset(6).size()) == 120u);
        });

        declare_test("bufr/C05060.bufr", [](const BufrBulletin& msg) {
            // GTS temp message
        });

#if 0
        declare_test("bufr/tempforecast.bufr", [](const BufrBulletin& msg) {
            // Custom ARPA temp forecast message saved as ARPA generic
            // TODO: we cannot test this unless we ship dballe's tables
            //       reenable after deciding whether to ship them or not
        });
#endif

        declare_test("bufr/C23000.bufr", [](const BufrBulletin& msg) {
            // C23000 modifier
        });

        declare_test("bufr/segfault1.bufr", [](const BufrBulletin& msg) {
            // BUFR that gave segfault
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 2);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 101);
            wassert(actual(msg.subsets.size()) == 1u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 129u);

            wassert(actual_varcode(s[9].code()) == WR_VAR(0, 5, 1));
            wassert(actual(s[9].enqd()) == 41.65);
            wassert(actual_varcode(s[10].code()) == WR_VAR(0, 6, 1));
            wassert(actual(s[10].enqd()) == 12.43);

            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);

            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(s[1].enqa(WR_VAR(0, 33, 7))->enqi()) == 70);
        });

        declare_test("bufr/C08022.bufr", [](const BufrBulletin& msg) {
            // C08xxx modifier
        });

        declare_test("bufr/C23000-1.bufr", [](const BufrBulletin& msg) {
            // C23xxx modifier on a message that gave problems in some machine
        });

        declare_test("bufr/C08032-toolong.bufr", [](const BufrBulletin& msg) {
        });

        declare_test("bufr/synop-longname.bufr", [](const BufrBulletin& msg) {
            // Synop with a very long station name
        });

        declare_test("bufr/C04004.bufr", [](const BufrBulletin& msg) {
            // C04yyy modifier, B32021=6
        });

        declare_test("bufr/C04-B31021-1.bufr", [](const BufrBulletin& msg) {
            // C04yyy modifier, B32021=1
        });

        declare_test("bufr/C06006.bufr", [](const BufrBulletin& msg) {
            // C06006 modifier
        });

        declare_test("bufr/gps_zenith.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 14);
            wassert(actual(msg.subsets.size()) == 94u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 175u);

            wassert(actual_varcode(s[0].code()) == WR_VAR(0, 1, 15));
            wassert(actual(s[0].enq<string>()) == "AQUI-BKG_");
        });

        declare_test("bufr/ascat1.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 12);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 223);
            wassert(actual(msg.subsets.size()) == 1722u);

            const Subset& s = msg.subset(0);
            wassert(actual(s.size()) == 124u);

            wassert(actual_varcode(s[0].code()) == WR_VAR(0, 1, 33));
            wassert(actual(s[0].enq<int>()) == 254);
        });

        declare_test("bufr/unparsable1.bufr", [](const BufrBulletin& msg) {
        });

        declare_test("bufr/C04type21.bufr", [](const BufrBulletin& msg) {
        });

        declare_test("bufr/noassoc.bufr", [](const BufrBulletin& msg) {
            // Buffer with a seemingly missing associated field significance
        });

        declare_test("bufr/atms1.bufr", [](const BufrBulletin& msg) {
            // ATMS data (http://npp.gsfc.nasa.gov/atms.html)
        });

        declare_test("bufr/atms2.bufr", [](const BufrBulletin& msg) {
            // ATMS data (http://npp.gsfc.nasa.gov/atms.html)
        });

        declare_test("bufr/table17.bufr", [](const BufrBulletin& msg) {
            // BUFR data using table 17
        });

        declare_test("bufr/A_ISMN02LFPW080000RRA_C_RJTD_20140808000319_100.bufr", [](const BufrBulletin& msg) {
        });

        declare_test("bufr/bitmap-B33035.bufr", [](const BufrBulletin& msg) {
        });

        declare_test("bufr/gts-buoy1.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 3);
            wassert(actual(msg.data_category) == 1);
            wassert(actual(msg.data_subcategory) == 255);
            wassert(actual(msg.data_subcategory_local) == 0);
            wassert(actual(msg.subsets.size()) == 1u);
        });

        declare_test("bufr/gts-synop-rad1.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 1);
            wassert(actual(msg.data_subcategory_local) == 0);
            wassert(actual(msg.subsets.size()) == 25u);
        });

        declare_test("bufr/gts-synop-rad2.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 6);
            wassert(actual(msg.data_subcategory_local) == 150);
            wassert(actual(msg.subsets.size()) == 1u);
        });

        declare_test("bufr/gts-synop-tchange.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 0);
            wassert(actual(msg.data_subcategory) == 1);
            wassert(actual(msg.data_subcategory_local) == 0);
            wassert(actual(msg.subsets.size()) == 1u);
        });

        declare_test("bufr/new-003.bufr", [](const BufrBulletin& msg) {
            wassert(actual(msg.edition_number) == 4);
            wassert(actual(msg.data_category) == 2);
            wassert(actual(msg.data_subcategory) == 10);
            wassert(actual(msg.data_subcategory_local) == 1);
            wassert(actual(msg.subsets.size()) == 1u);
        });

        declare_test_decode_fail("bufr/afl-src01flip1-pos10.bufr", "looking for data descriptor list");
    }
} testnewtg("bufr_decoder");


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
	wassert(actual(msg->rep_year) == 2007);
	wassert(actual(msg->rep_month) == 8);
	wassert(actual(msg->rep_day) == 13);
	wassert(actual(msg->rep_hour) == 18);
	wassert(actual(msg->rep_minute) == 30);
	wassert(actual(msg->rep_second) == 0);

	test.subset(0).vars = 4606;
	*/

	//bufrex_msg msg1 = reencode_test(msg);
	//ensureBufrexRawEquals(test, msg1);

	struct Tester : public MsgTester {
		void test(const BufrBulletin& msg)
		{
			/*
			wassert(actual(msg.edition_number) == 2);
			wassert(actual(msg.data_category) == 6);
			wassert(actual(msg.data_subcategory) == 255);
			wassert(actual(msg.data_subcategory_local) == 0);
			wassert(actual(msg.subsets.size()) == 1u);

			wassert(actual(msg.rep_year) == 2007);
			wassert(actual(msg.rep_month) == 8);
			wassert(actual(msg.rep_day) == 13);
			wassert(actual(msg.rep_hour) == 18);
			wassert(actual(msg.rep_minute) == 30);
			wassert(actual(msg.rep_second) == 0);

			wassert(actual(msg.subset(0).size()) == 4606u);

			const Subset& s = msg.subset(0);

			// FIXME Does it have this?
			wassert(actual(s[0].enqa(WR_VAR(0, 33, 7))).istrue());
			wassert(actual(s[0].enqa(WR_VAR(0) == 33, 7))->enqi(), 70);

			wassert(actual(s[5].enqa(WR_VAR(0, 33, 7))).istrue());
			wassert(actual(s[5].enqa(WR_VAR(0) == 33, 7))->enqi(), 70);
			*/
		}
	} test;

	// FIXME: recoding might not work
	declare_test("bufr/test-soil1.bufr", [](const BufrBulletin& msg)
#endif
}

#endif

}
