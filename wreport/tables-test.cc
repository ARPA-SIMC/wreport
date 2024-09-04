#include "tests.h"
#include "tables.h"
#include "wreport/dtable.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("empty", []() noexcept {
            // TODO: add test
        });
    }
} test("tables");

}
