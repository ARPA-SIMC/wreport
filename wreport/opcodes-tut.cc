#include <test-utils-wreport.h>
#include <wreport/opcodes.h>

using namespace wibble::tests;
using namespace wibble;
using namespace wreport;
using namespace wreport::tests;
using namespace std;
using namespace wibble;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("simple", [](Fixture& f) {
        // Test simple access
        vector<Varcode> ch0_vec;
        ch0_vec.push_back('A');
        ch0_vec.push_back('n');
        ch0_vec.push_back('t');

        Opcodes ch0(ch0_vec);
        ensure_equals(ch0.head(), 'A');
        ensure_equals(ch0.next().head(), 'n');
        ensure_equals(ch0.next().next().head(), 't');
        ensure_equals(ch0.next().next().next().head(), 0);

        ensure_equals(ch0[1], 'n');
        ensure_equals(ch0[10], 0);
    }),
};

test_group newtg("opcode", tests);

}
