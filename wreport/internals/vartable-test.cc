#include "vartable.h"
#include "wreport/tests.h"

using namespace wreport::tests;

namespace {
class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("internals_vartable");

void Tests::register_tests() {}

} // namespace
