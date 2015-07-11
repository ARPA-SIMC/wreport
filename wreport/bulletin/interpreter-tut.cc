#include "test-utils-wreport.h"
#include "interpreter.h"
#include "wreport/vartable.h"
#include "wreport/dtable.h"
#include <wibble/string.h>

using namespace wibble::tests;
using namespace wibble;
using namespace wreport;
using namespace wreport::tests;
using namespace std;
using namespace wibble;

namespace {

struct VisitCounter : public bulletin::DDSInterpreter
{
    unsigned count_b;
    unsigned count_r_plain;
    unsigned count_r_delayed;
    unsigned count_c;
    unsigned count_d;

    VisitCounter(const Tables& tables, const Opcodes& opcodes)
        : bulletin::DDSInterpreter(tables, opcodes), count_b(0), count_r_plain(0), count_r_delayed(0), count_c(0), count_d(0) {}

    void b_variable(Varcode code) override { ++count_b; }
    void c_modifier(Varcode code, Opcodes& next) override { ++count_c; bulletin::DDSInterpreter::c_modifier(code, next); }
    void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops) override
    {
        if (delayed_code)
            ++count_r_delayed;
        else
            ++count_r_plain;
        opcode_stack.push(ops);
        run();
        opcode_stack.pop();
    }
    void d_group_begin(Varcode code) override { ++count_d; }
};

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("visitor", [](Fixture& f) {
        // Test visitor
        const char* testdatadir = getenv("WREPORT_TABLES");
        if (!testdatadir) testdatadir = TABLE_DIR;
        Tables tables;
        tables.btable = Vartable::load_bufr(str::joinpath(testdatadir, "B0000000000000014000.txt"));
        tables.dtable = DTable::load_bufr(str::joinpath(testdatadir, "D0000000000000014000.txt"));
        Opcodes ops = tables.dtable->query(WR_VAR(3, 0, 10));
        ensure_equals(ops.size(), 4);

        VisitCounter c(tables, ops);
        c.run();

        ensure_equals(c.count_b, 4u);
        ensure_equals(c.count_c, 0u);
        ensure_equals(c.count_r_plain, 0u);
        ensure_equals(c.count_r_delayed, 1u);
        ensure_equals(c.count_d, 1u);
    }),
};

test_group newtg("bulletin_interpreter", tests);

}

