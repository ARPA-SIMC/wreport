#include "tests.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

struct MsgTester
{
    std::function<void(const CrexBulletin&)> test;

    void run(const char* name)
    {
        WREPORT_TEST_INFO(test_info);

        // Read the whole contents of the test file
        std::string raw1 = wcallchecked(slurpfile(name));

        // Decode the original contents
        auto msg1 = wcallchecked(decode_checked<CrexBulletin>(raw1, name));
        test_info() << "orig";
        wassert(test(*msg1));

        // Encode it again
        std::string raw = wcallchecked(msg1->encode());

        // Decode our encoder's output
        auto msg2 = wcallchecked(decode_checked<CrexBulletin>(raw, name));

        // Test the decoded version
        test_info() << "reencoded";
        wassert(test(*msg2));

        // Ensure the two are the same
        notes::Collect c(std::cerr);
        unsigned diffs = msg1->diff(*msg2);
        if (diffs)
        {
            track_bulletin(*msg1, "orig", name);
            track_bulletin(*msg2, "reenc", name);
        }
        wassert(actual(diffs) == 0u);
    }
};

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("test-synop0", []() {
            MsgTester test;
            test.test = [](const CrexBulletin& msg) {
                wassert(actual(msg.edition_number) == 1);
                wassert(actual(msg.data_category) == 0);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 49u);

                wassert(actual_varcode(s[8].code()) == WR_VAR(0, 5, 1));
                wassert(actual(s[8].enqd()) == 48.22);
                wassert(actual_varcode(s[9].code()) == WR_VAR(0, 6, 1));
                wassert(actual(s[9].enqd()) == 9.92);
                wassert(actual_varcode(s[17].code()) == WR_VAR(0, 12, 4));
                wassert(actual(s[17].enqd()) == 3.0);
                wassert(actual_varcode(s[18].code()) == WR_VAR(0, 12, 6));
                wassert(actual(s[18].enqd()) == 0.7);
            };
            test.run("crex/test-synop0.crex");
        });
        add_method("test-synop1", []() {
            MsgTester test;
            test.test = [](const CrexBulletin& msg) {
                wassert(actual(msg.edition_number) == 1);
                wassert(actual(msg.data_category) == 0);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 21u);

                wassert(actual_varcode(s[8].code()) == WR_VAR(0, 5, 1));
                wassert(actual(s[8].enqd()) == 53.55);
                wassert(actual_varcode(s[9].code()) == WR_VAR(0, 6, 1));
                wassert(actual(s[9].enqd()) == 13.20);
            };
            test.run("crex/test-synop1.crex");
        });
        add_method("test-synop2", []() {
            MsgTester test;
            test.test = [](const CrexBulletin& msg) {
                wassert(actual(msg.edition_number) == 1);
                wassert(actual(msg.data_category) == 0);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 49u);

                wassert(actual_varcode(s[8].code()) == WR_VAR(0, 5, 1));
                wassert(actual(s[8].enqd()) == 47.83);
                wassert(actual_varcode(s[9].code()) == WR_VAR(0, 6, 1));
                wassert(actual(s[9].enqd()) == 10.87);
            };
            test.run("crex/test-synop2.crex");
        });
        add_method("test-synop3", []() {
            MsgTester test;
            test.test = [](const CrexBulletin& msg) {
                wassert(actual(msg.edition_number) == 1);
                wassert(actual(msg.data_category) == 0);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 27u);

                wassert(actual_varcode(s[8].code()) == WR_VAR(0, 5, 1));
                wassert(actual(s[8].enqd()) == 61.85);
                wassert(actual_varcode(s[9].code()) == WR_VAR(0, 6, 1));
                wassert(actual(s[9].enqd()) == 24.80);
            };
            test.run("crex/test-synop3.crex");
        });
        add_method("test-mare0", []() {
            MsgTester test;
            test.test = [](const CrexBulletin& msg) {
                wassert(actual(msg.edition_number) == 1);
                wassert(actual(msg.data_category) == 1);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 32u);

                wassert(actual_varcode(s[9].code()) == WR_VAR(0, 5, 2));
                wassert(actual(s[9].enqd()) == 68.27);
                wassert(actual_varcode(s[10].code()) == WR_VAR(0, 6, 2));
                wassert(actual(s[10].enqd()) ==  9.68);
            };
            test.run("crex/test-mare0.crex");
        });
        add_method("test-mare1", []() {
            MsgTester test;
            test.test = [](const CrexBulletin& msg) {
                wassert(actual(msg.edition_number) == 1);
                wassert(actual(msg.data_category) == 1);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 32u);

                wassert(actual_varcode(s[9].code()) == WR_VAR(0, 5, 2));
                wassert(actual(s[9].enqd()) ==  43.02);
                wassert(actual_varcode(s[10].code()) == WR_VAR(0, 6, 2));
                wassert(actual(s[10].enqd()) == -12.45);
            };
            test.run("crex/test-mare1.crex");
        });
        add_method("test-mare2", []() {
            MsgTester test;
            test.test = [](const CrexBulletin& msg) {
                wassert(actual(msg.edition_number) == 1);
                wassert(actual(msg.data_category) == 1);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 39u);

                wassert(actual_varcode(s[9].code()) == WR_VAR(0, 5, 2));
                wassert(actual(s[9].enqd()) == 33.90);
                wassert(actual_varcode(s[10].code()) == WR_VAR(0, 6, 2));
                wassert(actual(s[10].enqd()) == 29.00);
            };
            test.run("crex/test-mare2.crex");
        });
        add_method("test-temp0", []() {
            MsgTester test;
            test.test = [](const CrexBulletin& msg) {
                wassert(actual(msg.edition_number) == 1);
                wassert(actual(msg.data_category) == 2);
                wassert(actual(msg.data_subcategory) == 255);
                wassert(actual(msg.data_subcategory_local) == 0);
                wassert(actual(msg.subsets.size()) == 1u);

                const Subset& s = msg.subset(0);
                wassert(actual(s.size()) == 550u);

                wassert(actual_varcode(s[9].code()) == WR_VAR(0, 5, 1));
                wassert(actual(s[9].enqd()) == 55.75);
                wassert(actual_varcode(s[10].code()) == WR_VAR(0, 6, 1));
                wassert(actual(s[10].enqd()) == 12.52);
            };
            test.run("crex/test-temp0.crex");
        });
    };
} test("crex_decoder");

}
