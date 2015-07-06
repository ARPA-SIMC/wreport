#include "test-utils-wreport.h"
#include "tables.h"
#include "wreport/dtable.h"
#include <wibble/string.h>

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
    Test("empty", [](Fixture& f) {
        // TODO: add test
    }),
};

test_group newtg("tables", tests);

}


