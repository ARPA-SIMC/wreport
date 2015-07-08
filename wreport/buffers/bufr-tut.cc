#include <test-utils-wreport.h>
#include "bufr.h"

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("empty", [](Fixture& f) {
    }),
};

test_group newtg("buffers_bufr", tests);

}

