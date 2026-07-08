#include "opcodes.h"
#include "tests.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("simple", []() {
            // Test simple access
            vector<Varcode> ch0_vec;
            ch0_vec.push_back('A');
            ch0_vec.push_back('n');
            ch0_vec.push_back('t');

            Opcodes ch0(ch0_vec);
            wassert(actual(ch0.head()) == 'A');
            wassert(actual(ch0.next().head()) == 'n');
            wassert(actual(ch0.next().next().head()) == 't');
            wassert(actual(ch0.next().next().next().head()) == 0);

            wassert(actual(ch0[1]) == 'n');
            wassert(actual(ch0[10]) == 0);
        });
    }
} test("opcode");

} // namespace
