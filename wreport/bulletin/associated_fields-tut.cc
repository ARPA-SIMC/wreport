#include "test-utils-wreport.h"
#include "associated_fields.h"

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
        // TODO
    }),
};

test_group newtg("bulletin_associated_fields", tests);

}



