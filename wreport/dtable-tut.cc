#include "tests.h"
#include "dtable.h"
#include <wibble/string.h>

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;
using namespace wibble;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("query", [](Fixture& f) {
        // Test basic queries
        const char* testdatadir = getenv("WREPORT_TESTDATA");
        if (!testdatadir) testdatadir = ".";
        const DTable* table = DTable::load_crex(str::joinpath(testdatadir, "test-crex-d-table.txt"));

        /* Try querying a nonexisting item */
        try {
            table->query(WR_VAR(3, 0, 9));
        } catch (error_notfound& e) {
            ensure_contains(e.what(), "300009");
        }

        /* Query the first item */
        Opcodes chain = table->query(WR_VAR(3, 0, 2));
        ensure_equals(chain.size(), 2u);
        ensure_equals(chain.head(), WR_VAR(0, 0, 2));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(0, 0, 3));
        chain = chain.next();
        ensure_equals(chain.head(), 0);
        ensure_equals(chain.size(), 0);

        /* Now query an existing item */
        chain = table->query(WR_VAR(3, 35, 6));
        ensure_equals(chain.size(), 7u);

        ensure_equals(chain.head(), WR_VAR(0, 8, 21));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(0, 4, 4));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(0, 8, 21));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(0, 4, 4));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(0, 35, 0));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(0, 1, 3));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(0, 35, 11));
        chain = chain.next();
        ensure_equals(chain.head(), 0);
        chain = chain.next();
        ensure_equals(chain.head(), 0);
        ensure_equals(chain.size(), 0);

        /* Then query the last item */
        chain = table->query(WR_VAR(3, 35, 10));
        ensure_equals(chain.size(), 3u);

        ensure_equals(chain.head(), WR_VAR(3, 35, 2));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(3, 35, 3));
        chain = chain.next();
        ensure_equals(chain.head(), WR_VAR(3, 35, 7));
        chain = chain.next();
        ensure_equals(chain.head(), 0);
        ensure_equals(chain.size(), 0);
    }),
    Test("bufr4", [](Fixture& f) {
        const char* testdatadir = getenv("WREPORT_TABLES");
        if (!testdatadir) testdatadir = TABLE_DIR;
        const DTable* table = DTable::load_crex(str::joinpath(testdatadir, "D0000000000098013102.txt"));

        /* Try querying a nonexisting item */
        try {
            table->query(WR_VAR(3, 0, 9));
        } catch (error_notfound& e) {
            ensure_contains(e.what(), "300009");
        }

        /* Now query an existing item */
        Opcodes chain = table->query(WR_VAR(3, 1, 24));

        ensure_varcode_equals(chain.head(), WR_VAR(0, 5, 2)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 6, 2)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 7, 1)); chain = chain.next();
        ensure_varcode_equals(chain.head(), 0); chain = chain.next();
        ensure_varcode_equals(chain.head(), 0);
        ensure_equals(chain.size(), 0);
        /*fprintf(stderr, "VAL: %d %02d %03d\n", WR_VAR_F(cur->val), WR_VAR_X(cur->val), WR_VAR_Y(cur->val));*/

        /* Then query the last item */
        chain = table->query(WR_VAR(3, 21, 28));
        ensure_varcode_equals(chain.head(), WR_VAR(0, 21, 118)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(2,  2, 129)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(2,  1, 132)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0,  2, 112)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(2,  1,   0)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(2,  1, 131)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0,  2, 111)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(2,  1,   0)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(2,  2,   0)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0,  2, 104)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 21, 123)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 21, 106)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 21, 107)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 21, 114)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 21, 115)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 21, 116)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0,  8,  18)); chain = chain.next();
        ensure_varcode_equals(chain.head(), WR_VAR(0, 21, 117)); chain = chain.next();
        ensure_varcode_equals(chain.head(), 0); chain = chain.next();
        ensure_varcode_equals(chain.head(), 0);
        ensure_equals(chain.size(), 0);
    }),
};

test_group newtg("dtable", tests);

}
