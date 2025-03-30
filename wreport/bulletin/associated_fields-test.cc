#include "associated_fields.h"
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
        add_method("empty", []() noexcept {});
    }
} test("bulletin_associated_fields");

} // namespace
