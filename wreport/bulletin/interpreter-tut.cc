#include <test-utils-wreport.h>
#include "interpreter.h"
#include "wreport/dtable.h"
#include <wibble/string.h>

using namespace wibble::tests;
using namespace wibble;
using namespace wreport;
using namespace wreport::tests;
using namespace std;
using namespace wibble;

namespace {

struct VisitCounter : public bulletin::Visitor
{
    unsigned count_b;
    unsigned count_r_plain;
    unsigned count_r_delayed;
    unsigned count_c;
    unsigned count_d;

    VisitCounter()
        : count_b(0), count_r_plain(0), count_r_delayed(0), count_c(0), count_d(0) {}

    void b_variable(Varcode code) { ++count_b; }
    void c_modifier(Varcode code) { ++count_c; }
    void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
    {
        if (delayed_code)
            ++count_r_delayed;
        else
            ++count_r_plain;
        bulletin::Interpreter interpreter(*dtable, ops, *this);
        interpreter.run();
    }
    void d_group_begin(Varcode code) { ++count_d; }
};

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("visitor", [](Fixture& f) {
        // Test visitor
        const char* testdatadir = getenv("WREPORT_TABLES");
        if (!testdatadir) testdatadir = TABLE_DIR;
        const DTable* table = DTable::load_bufr(str::joinpath(testdatadir, "D0000000000000014000.txt"));
        Opcodes ops = table->query(WR_VAR(3, 0, 10));
        ensure_equals(ops.size(), 4);

        VisitCounter c;
        bulletin::Interpreter interpreter(*table, ops, c);
        interpreter.run();

        ensure_equals(c.count_b, 4u);
        ensure_equals(c.count_c, 0u);
        ensure_equals(c.count_r_plain, 0u);
        ensure_equals(c.count_r_delayed, 1u);
        ensure_equals(c.count_d, 1u);
    }),
};

test_group newtg("bulletin_interpreter", tests);

}

