#include "tests.h"
#include <cstring>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

bool memfind(const std::string& rmsg, const char* str, size_t len)
{
    for (size_t i = 0; true; ++i)
    {
        if (i + len >= rmsg.size()) return false;
        if (memcmp(rmsg.data() + i, str, len) == 0)
            return true;
    }
}

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("encode", []() {
            auto test = [](const BufrBulletin& msg) {
                wassert(actual(msg.edition_number) == 3);
                wassert(actual(msg.data_category) == 0);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 1u);

                wassert(actual_varcode(s[0].code()) == WR_VAR(0, 0, 13));
                wassert(actual(string(s[0].enqc())) == "abcdefg");

                // Ensure that the decoded strings are zero-padded
                wassert(actual(memcmp(s[0].enqc(), "abcdefg\0\0\0\0\0\0\0", 7+7)) == 0);
            };

            WREPORT_TEST_INFO(test_info);

            unique_ptr<BufrBulletin> pmsg(BufrBulletin::create());
            BufrBulletin& msg = *pmsg;
            msg.clear();

            /* Initialise common message bits */
            msg.edition_number = 3;            // BUFR ed.4
            msg.data_category = 0;               // Template 8.255.171
            msg.data_subcategory = 255;
            msg.data_subcategory_local = 0;
            msg.originating_centre = 98;
            msg.originating_subcentre = 0;
            msg.master_table_version_number = 12;
            msg.master_table_version_number_local = 1;
            msg.compression = true;
            msg.rep_year = 2008;
            msg.rep_month = 5;
            msg.rep_day = 3;
            msg.rep_hour = 12;
            msg.rep_minute = 30;
            msg.rep_second = 0;

            // Load encoding tables
            msg.load_tables();

            // Fill up the data descriptor section
            msg.datadesc.push_back(WR_VAR(0,  0,  13));

            // Get the working subset
            Subset& s = msg.obtain_subset(0);

            // Set a text variable
            s.store_variable_c(WR_VAR(0, 0, 13), "12345678901234567890");

            // Set it to a shorter text, to see if the encoder encodes the trailing garbage
            s[0].setc("abcdefg");

            // Run tests on the original
            test_info() << "orig";
            wassert(test(msg));

            // Encode
            string rmsg = wcallchecked(msg.encode());

            // Ensure that the encoded strings are space-padded
            wassert(actual(memfind(rmsg, "abcdefg       ", 14)).istrue());

            // Decode the message and retest
            auto msg1 = BufrBulletin::decode(rmsg);
            test_info() << "reencoded";
            wassert(test(*msg1));
        });
        add_method("encode_optsec", []() {
            // Encode a BUFR with an optional section
            auto test = [](const BufrBulletin& msg) {
                wassert(actual(msg.edition_number) == 3);
                wassert(actual(msg.data_category) == 0);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 1u);

                wassert(actual_varcode(s[0].code()) == WR_VAR(0, 0, 13));
                wassert(actual(string(s[0].enqc())) == "abcdefg");

                // Ensure that the decoded strings are zero-padded
                wassert(actual(memcmp(s[0].enqc(), "abcdefg\0\0\0\0\0\0\0", 7+7)) == 0);
            };

            WREPORT_TEST_INFO(test_info);

            unique_ptr<BufrBulletin> pmsg(BufrBulletin::create());
            BufrBulletin& msg = *pmsg;
            msg.clear();

            // Initialise common message bits
            msg.edition_number = 3;            // BUFR ed.4
            msg.data_category = 0;               // Template 8.255.171
            msg.data_subcategory = 255;
            msg.data_subcategory_local = 0;
            msg.originating_centre = 98;
            msg.originating_subcentre = 0;
            msg.master_table_version_number = 12;
            msg.master_table_version_number_local = 1;
            msg.compression = true;
            msg.optional_section = string("Ciao", 5);
            msg.rep_year = 2008;
            msg.rep_month = 5;
            msg.rep_day = 3;
            msg.rep_hour = 12;
            msg.rep_minute = 30;
            msg.rep_second = 0;

            // Load encoding tables
            msg.load_tables();

            // Fill up the data descriptor section
            msg.datadesc.push_back(WR_VAR(0, 0, 13));

            // Get the working subset
            Subset& s = msg.obtain_subset(0);

            // Set a text variable
            s.store_variable_c(WR_VAR(0, 0, 13), "12345678901234567890");

            // Set it to a shorter text, to see if the encoder encodes the trailing garbage
            s[0].setc("abcdefg");

            // Run tests on the original
            test_info() << "orig";
            wassert(test(msg));

            // Encode
            string rmsg = wcallchecked(msg.encode());

            // Ensure that the encoded strings are space-padded
            wassert(actual(memfind(rmsg, "abcdefg       ", 14)).istrue());

            // Decode the message and retest
            unique_ptr<BufrBulletin> pmsg1(BufrBulletin::decode(rmsg));
            BufrBulletin& msg1 = *pmsg1;

            // Check that the optional section has been padded
            wassert(actual(msg1.optional_section.size()) == 6u);
            wassert(actual(memcmp(msg1.optional_section.data(), "Ciao\0", 6)) == 0);

            test_info() << "reencoded";
            wassert(test(msg1));
        });
        add_method("var_ranges", []() {
            // Test variable ranges during encoding
            unique_ptr<BufrBulletin> pmsg(BufrBulletin::create());
            BufrBulletin& msg = *pmsg;

            // Initialise common message bits
            msg.edition_number = 3;            // BUFR ed.4
            msg.data_category = 0;               // Template 8.255.171
            msg.data_subcategory = 255;
            msg.data_subcategory_local = 0;
            msg.originating_centre = 98;
            msg.originating_subcentre = 0;
            msg.master_table_version_number = 12;
            msg.master_table_version_number_local = 1;
            msg.compression = false;
            msg.rep_year = 2008;
            msg.rep_month = 5;
            msg.rep_day = 3;
            msg.rep_hour = 12;
            msg.rep_minute = 30;
            msg.rep_second = 0;

            // Load encoding tables
            msg.load_tables();

            // Fill up the data descriptor section
            msg.datadesc.push_back(WR_VAR(0, 1, 1));

            /* Get the working subset */
            Subset& s = msg.obtain_subset(0);

            /* Set the test variable */
            //CHECKED(bufrex_subset_store_variable_d(s, WR_VAR(0, 1, 1), -1.0));
            /* Now it errors here, because the range check is appropriately strict */
            try {
                s.store_variable_d(WR_VAR(0, 1, 1), -1.0);
                throw TestFailed("function should have thrown error_domain");
            } catch (error_domain& e) {
                wassert(actual(e.what()).contains("001001"));
            }

#if 0
            /* Encode gives error because of overflow */
            dba_rawmsg rmsg = NULL;
            dba_err err = bufrex_msg_encode(msg, &rmsg);
            ensure(err == DBA_ERROR);
#endif
        });
    }
} test("bufr_encoder");

}
