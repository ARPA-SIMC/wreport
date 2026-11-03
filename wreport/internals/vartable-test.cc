#include "vartable.h"
#include "wreport/tests.h"

using namespace wreport;
using namespace wreport::tests;

namespace {
class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("internals_vartable");

void Tests::register_tests()
{

    add_method("contains_varinfo", [] {
        vartable::Base table("test");

        Varinfo base    = table.obtain(0, WR_VAR(0, 1, 2));
        Varinfo altered = table.query_altered(WR_VAR(0, 1, 2), 1, 1, 1);

        wassert_true(table.contains_varinfo(base));
        wassert_true(table.contains_varinfo(altered));

        auto other_table = Vartable::get_bufr("B0000000000000014000");

        Varinfo other_info = other_table->query(WR_VAR(0, 1, 2));
        wassert_false(table.contains_varinfo(other_info));

        other_info = other_table->query(WR_VAR(0, 1, 1));
        wassert_false(table.contains_varinfo(other_info));
    });
}

} // namespace
