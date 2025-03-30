#include "test-utils-lua.h"
#include "tests.h"
#include "var.h"
#include "vartable.h"

using namespace wreport;
using namespace wreport::tests;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        // Test variable access
        add_method("var", []() {
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");
            Var var(table->query(WR_VAR(0, 12, 101)), 12.3);

            tests::Lua test("function test() \n"
                            "  if var:code() ~= 'B12101' then return 'code is "
                            "'..var:code()..' instead of B12101' end \n"
                            "  if var:enqi() ~= 1230 then return 'enqi is "
                            "'..var:enqi()..' instead of 1230' end \n"
                            "  if var:enqd() ~= 12.3 then return 'enqd is "
                            "'..var:enqd()..' instead of 12.3' end \n"
                            "  if var:enqc() ~= '1230' then return 'enqc is "
                            "'..var:enqc()..' instead of 1230' end \n"
                            "end \n");

            // Push the variable as a global
            var.lua_push(test.L);
            lua_setglobal(test.L, "var");

            // Check that we can retrieve it
            lua_getglobal(test.L, "var");
            Var* pvar = Var::lua_check(test.L, 1);
            lua_pop(test.L, 1);
            wassert(actual(&var == pvar).istrue());

            wassert(actual(test.run()) == "");
        });
    }
} tests("lua");

} // namespace
