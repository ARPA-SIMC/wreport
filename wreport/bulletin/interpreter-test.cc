#include "config.h"
#include "interpreter.h"
#include "wreport/dtable.h"
#include "wreport/tests.h"
#include "wreport/utils/string.h"
#include "wreport/vartable.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

struct VisitCounter : public bulletin::Interpreter
{
    unsigned count_b;
    unsigned count_r_plain;
    unsigned count_r_delayed;
    unsigned count_c;
    unsigned count_d;

    VisitCounter(const Tables& tables, const Opcodes& opcodes)
        : bulletin::Interpreter(tables, opcodes), count_b(0), count_r_plain(0),
          count_r_delayed(0), count_c(0), count_d(0)
    {
    }

    void b_variable(Varcode code) override { ++count_b; }
    void c_modifier(Varcode code, Opcodes& next) override
    {
        ++count_c;
        bulletin::Interpreter::c_modifier(code, next);
    }
    void r_replication(Varcode code, Varcode delayed_code,
                       const Opcodes& ops) override
    {
        if (delayed_code)
            ++count_r_delayed;
        else
            ++count_r_plain;
        opcode_stack.push(ops);
        run();
        opcode_stack.pop();
    }
    void run_d_expansion(Varcode code) override
    {
        ++count_d;
        bulletin::Interpreter::run_d_expansion(code);
    }
};

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("visitor", []() {
            // Test visitor
            auto testdatadir = path_from_env("WREPORT_TABLES", TABLE_DIR);
            Tables tables;
            tables.btable =
                Vartable::load_bufr(testdatadir / "B0000000000000014000.txt");
            tables.dtable =
                DTable::load_bufr(testdatadir / "D0000000000000014000.txt");
            Opcodes ops = tables.dtable->query(WR_VAR(3, 0, 10));
            wassert(actual(ops.size()) == 4u);

            VisitCounter c(tables, ops);
            c.run();

            wassert(actual(c.count_b) == 4u);
            wassert(actual(c.count_c) == 0u);
            wassert(actual(c.count_r_plain) == 0u);
            wassert(actual(c.count_r_delayed) == 1u);
            wassert(actual(c.count_d) == 1u);
        });
    }
} test("bulletin_interpreter");

} // namespace
