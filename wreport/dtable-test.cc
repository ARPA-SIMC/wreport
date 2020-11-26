#include "tests.h"
#include "dtable.h"
#include "utils/string.h"
#include "config.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("query", []() {
            // Test basic queries
            const char* testdatadir = getenv("WREPORT_TESTDATA");
            if (!testdatadir) testdatadir = ".";
            const DTable* table = DTable::load_crex(str::joinpath(testdatadir, "test-crex-d-table.txt"));

            /* Try querying a nonexisting item */
            try {
                table->query(WR_VAR(3, 0, 9));
            } catch (error_notfound& e) {
                wassert(actual(e.what()).contains("300009"));
            }

            /* Query the first item */
            Opcodes chain = table->query(WR_VAR(3, 0, 2));
            wassert(actual(chain.size()) == 2u);
            wassert(actual(chain.head()) == WR_VAR(0, 0, 2));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(0, 0, 3));
            chain = chain.next();
            wassert(actual(chain.head()) == 0);
            wassert(actual(chain.size()) == 0u);

            /* Now query an existing item */
            chain = table->query(WR_VAR(3, 35, 6));
            wassert(actual(chain.size()) == 7u);

            wassert(actual(chain.head()) == WR_VAR(0, 8, 21));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(0, 4, 4));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(0, 8, 21));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(0, 4, 4));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(0, 35, 0));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(0, 1, 3));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(0, 35, 11));
            chain = chain.next();
            wassert(actual(chain.head()) == 0);
            chain = chain.next();
            wassert(actual(chain.head()) == 0);
            wassert(actual(chain.size()) == 0u);

            /* Then query the last item */
            chain = table->query(WR_VAR(3, 35, 10));
            wassert(actual(chain.size()) == 3u);

            wassert(actual(chain.head()) == WR_VAR(3, 35, 2));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(3, 35, 3));
            chain = chain.next();
            wassert(actual(chain.head()) == WR_VAR(3, 35, 7));
            chain = chain.next();
            wassert(actual(chain.head()) == 0);
            wassert(actual(chain.size()) == 0u);
        });
        add_method("bufr4", []() {
            const char* testdatadir = getenv("WREPORT_TABLES");
            if (!testdatadir) testdatadir = TABLE_DIR;
            const DTable* table = DTable::load_crex(str::joinpath(testdatadir, "D0000000000098013102.txt"));

            /* Try querying a nonexisting item */
            try {
                table->query(WR_VAR(3, 0, 9));
            } catch (error_notfound& e) {
                wassert(actual(e.what()).contains("300009"));
            }

            /* Now query an existing item */
            Opcodes chain = table->query(WR_VAR(3, 1, 24));

            wassert(actual_varcode(chain.head()) == WR_VAR(0, 5, 2)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 6, 2)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 7, 1)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == 0); chain = chain.next();
            wassert(actual_varcode(chain.head()) == 0);
            wassert(actual(chain.size()) == 0u);
            /*fprintf(stderr, "VAL: %d %02d %03d\n", WR_VAR_F(cur->val), WR_VAR_X(cur->val), WR_VAR_Y(cur->val));*/

            /* Then query the last item */
            chain = table->query(WR_VAR(3, 21, 28));
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 21, 118)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(2,  2, 129)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(2,  1, 132)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0,  2, 112)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(2,  1,   0)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(2,  1, 131)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0,  2, 111)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(2,  1,   0)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(2,  2,   0)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0,  2, 104)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 21, 123)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 21, 106)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 21, 107)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 21, 114)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 21, 115)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 21, 116)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0,  8,  18)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == WR_VAR(0, 21, 117)); chain = chain.next();
            wassert(actual_varcode(chain.head()) == 0); chain = chain.next();
            wassert(actual_varcode(chain.head()) == 0);
            wassert(actual(chain.size()) == 0u);
        });
    }
} test("dtable");

}
