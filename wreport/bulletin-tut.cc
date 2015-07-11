#include "test-utils-wreport.h"
#include "bulletin.h"

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
    Test("obtain_subset", [](Fixture& f) {
        unique_ptr<BufrBulletin> b(BufrBulletin::create());
        wassert(actual(b->subsets.size()) == 0);
        try {
            b->obtain_subset(0);
            ensure(false);
        } catch (error_consistency& e) {
            ensure_contains(e.what(), "BUFR/CREX tables not loaded");
        }
    }),
};

test_group newtg("bulletin", tests);

}
