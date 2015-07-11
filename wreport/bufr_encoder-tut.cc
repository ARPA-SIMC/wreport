#include <test-utils-wreport.h>
#include <cstring>

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

bool memfind(const std::string& rmsg, const char* str, size_t len)
{
    for (size_t i = 0; true; ++i)
    {
        if (i + len >= rmsg.size()) return false;
        if (memcmp((const char*)rmsg.data() + i, str, len) == 0)
            return true;
    }
}

typedef tests::MsgTester<BufrBulletin> MsgTester;


std::vector<Test> tests {
    Test("encode", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const BufrBulletin& msg)
            {
                ensure_equals(msg.edition_number, 3);
                ensure_equals(msg.data_category, 0);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 1u);

                ensure_varcode_equals(s[0].code(), WR_VAR(0, 0, 13));
                ensure_equals(string(s[0].enqc()), "abcdefg");

                // Ensure that the decoded strings are zero-padded
                ensure(memcmp(s[0].enqc(), "abcdefg\0\0\0\0\0\0\0", 7+7) == 0);
            }
        } test;

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
        test.run("orig", msg);

        // Encode
        string rmsg = wcallchecked(msg.encode());

        // Ensure that the encoded strings are space-padded
        ensure(memfind(rmsg, "abcdefg       ", 14));

        // Decode the message and retest
        unique_ptr<BufrBulletin> pmsg1(BufrBulletin::create());
        BufrBulletin& msg1 = *pmsg1;
        msg1.decode(rmsg);
        test.run("reencoded", msg1);
    }),
    Test("encode_optsec", [](Fixture& f) {
        // Encode a BUFR with an optional section
        struct Tester : public MsgTester {
            void test(const BufrBulletin& msg)
            {
                ensure_equals(msg.edition_number, 3);
                ensure_equals(msg.data_category, 0);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 1u);

                ensure_varcode_equals(s[0].code(), WR_VAR(0, 0, 13));
                ensure_equals(string(s[0].enqc()), "abcdefg");

                // Ensure that the decoded strings are zero-padded
                ensure(memcmp(s[0].enqc(), "abcdefg\0\0\0\0\0\0\0", 7+7) == 0);
            }
        } test;

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
        test.run("orig", msg);

        // Encode
        string rmsg = wcallchecked(msg.encode());

        // Ensure that the encoded strings are space-padded
        ensure(memfind(rmsg, "abcdefg       ", 14));

        // Decode the message and retest
        unique_ptr<BufrBulletin> pmsg1(BufrBulletin::create());
        BufrBulletin& msg1 = *pmsg1;
        msg1.decode(rmsg);

        // Check that the optional section has been padded
        wassert(actual(msg1.optional_section.size()) == 6);
        ensure_equals(memcmp(msg1.optional_section.data(), "Ciao\0", 6), 0);

        test.run("reencoded", msg1);
    }),
    Test("var_ranges", [](Fixture& f) {
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
            ensure(false);
        } catch (error_domain& e) {
            ensure_contains(e.what(), "B01001");

        }

#if 0
        /* Encode gives error because of overflow */
        dba_rawmsg rmsg = NULL;
        dba_err err = bufrex_msg_encode(msg, &rmsg);
        ensure(err == DBA_ERROR);
#endif
    }),
};

test_group newtg("bufr_encoder", tests);

}
