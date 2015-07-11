#include <test-utils-wreport.h>

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

typedef tests::MsgTester<CrexBulletin> MsgTester;

std::vector<Test> tests {
    Test("test-synop0", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const CrexBulletin& msg)
            {
                ensure_equals(msg.edition_number, 1);
                ensure_equals(msg.data_category, 0);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 49u);

                ensure_varcode_equals(s[8].code(), WR_VAR(0, 5, 1));
                ensure_equals(s[8].enqd(), 48.22);
                ensure_varcode_equals(s[9].code(), WR_VAR(0, 6, 1));
                ensure_equals(s[9].enqd(), 9.92);
                ensure_varcode_equals(s[17].code(), WR_VAR(0, 12, 4));
                ensure_equals(s[17].enqd(), 3.0);
                ensure_varcode_equals(s[18].code(), WR_VAR(0, 12, 6));
                ensure_equals(s[18].enqd(), 0.7);
            }
        } test;

        test.run("crex/test-synop0.crex");
    }),
    Test("test-synop1", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const CrexBulletin& msg)
            {
                ensure_equals(msg.edition_number, 1);
                ensure_equals(msg.data_category, 0);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 21u);

                ensure_varcode_equals(s[8].code(), WR_VAR(0, 5, 1));
                ensure_equals(s[8].enqd(), 53.55);
                ensure_varcode_equals(s[9].code(), WR_VAR(0, 6, 1));
                ensure_equals(s[9].enqd(), 13.20);
            }
        } test;

        test.run("crex/test-synop1.crex");
    }),
    Test("test-synop2", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const CrexBulletin& msg)
            {
                ensure_equals(msg.edition_number, 1);
                ensure_equals(msg.data_category, 0);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 49u);

                ensure_varcode_equals(s[8].code(), WR_VAR(0, 5, 1));
                ensure_equals(s[8].enqd(), 47.83);
                ensure_varcode_equals(s[9].code(), WR_VAR(0, 6, 1));
                ensure_equals(s[9].enqd(), 10.87);
            }
        } test;

        test.run("crex/test-synop2.crex");
    }),
    Test("test-synop3", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const CrexBulletin& msg)
            {
                ensure_equals(msg.edition_number, 1);
                ensure_equals(msg.data_category, 0);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 27u);

                ensure_varcode_equals(s[8].code(), WR_VAR(0, 5, 1));
                ensure_equals(s[8].enqd(), 61.85);
                ensure_varcode_equals(s[9].code(), WR_VAR(0, 6, 1));
                ensure_equals(s[9].enqd(), 24.80);
            }
        } test;

        test.run("crex/test-synop3.crex");
    }),
    Test("test-mare0", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const CrexBulletin& msg)
            {
                ensure_equals(msg.edition_number, 1);
                ensure_equals(msg.data_category, 1);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 32u);

                ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
                ensure_equals(s[9].enqd(), 68.27);
                ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
                ensure_equals(s[10].enqd(),  9.68);
            }
        } test;

        test.run("crex/test-mare0.crex");
    }),
    Test("test-mare1", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const CrexBulletin& msg)
            {
                ensure_equals(msg.edition_number, 1);
                ensure_equals(msg.data_category, 1);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 32u);

                ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
                ensure_equals(s[9].enqd(),  43.02);
                ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
                ensure_equals(s[10].enqd(), -12.45);
            }
        } test;

        test.run("crex/test-mare1.crex");
    }),
    Test("test-mare2", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const CrexBulletin& msg)
            {
                ensure_equals(msg.edition_number, 1);
                ensure_equals(msg.data_category, 1);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 39u);

                ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
                ensure_equals(s[9].enqd(), 33.90);
                ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
                ensure_equals(s[10].enqd(), 29.00);
            }
        } test;

        test.run("crex/test-mare2.crex");
    }),
    Test("test-temp0", [](Fixture& f) {
        struct Tester : public MsgTester {
            void test(const CrexBulletin& msg)
            {
                ensure_equals(msg.edition_number, 1);
                ensure_equals(msg.data_category, 2);
                ensure_equals(msg.data_subcategory, 255);
                ensure_equals(msg.data_subcategory_local, 0);
                ensure_equals(msg.subsets.size(), 1);

                const Subset& s = msg.subset(0);
                ensure_equals(s.size(), 550u);

                ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 1));
                ensure_equals(s[9].enqd(), 55.75);
                ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 1));
                ensure_equals(s[10].enqd(), 12.52);
            }
        } test;

        test.run("crex/test-temp0.crex");
    }),
};

test_group newtg("crex_decoder", tests);

}
