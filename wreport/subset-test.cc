#include "subset.h"
#include "tests.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("subset");

void Tests::register_tests()
{
    add_method("modify", [] {
        std::string raw = tests::slurpfile("bufr/MODE_12.bufr");
        auto bulletin   = BufrBulletin::decode(raw, "bufr/MODE_12.bufr");
        auto& subset    = bulletin->subsets[0];

        wassert(actual(subset.size()) == 44u);
        wassert(actual(subset[0].code()) == WR_VAR(0, 1, 8));
        wassert(actual(subset[43].code()) == WR_VAR(0, 33, 2));

        // Erase the first element
        subset.erase(subset.begin());
        wassert(actual(subset.size()) == 43u);
        wassert(actual_varcode(subset[0].code()) == WR_VAR(0, 1, 23));
        wassert(actual_varcode(subset[42].code()) == WR_VAR(0, 33, 2));

        // Erase from the middle
        subset.erase(subset.begin() + 3, subset.begin() + 5);
        wassert(actual(subset.size()) == 41u);
        wassert(actual(subset[0].code()) == WR_VAR(0, 1, 23));
        wassert(actual(subset[40].code()) == WR_VAR(0, 33, 2));
    });
}

} // namespace
