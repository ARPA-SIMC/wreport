#include "tests.h"
#include "tableinfo.h"

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

template<typename ID>
struct CompareTester
{
    ID base;

    CompareTester(const ID& id) : base(id) {}

    template<typename ID1, typename ID2>
    const char* operator()(const ID1& first, const ID2& second) const
    {
        if (base.is_acceptable_replacement(first))
            if (base.is_acceptable_replacement(second))
            {
                int cmp = base.closest_match(first, second);
                if (cmp < 0) return "first";
                if (cmp > 0) return "second";
                return "same";
            } else
                return "firstonly";
        else
            if (base.is_acceptable_replacement(second))
                return "secondonly";
            else
                return "none";
    }
};


std::vector<Test> tests {
    Test("bufrtableid", [](Fixture& f) {
        // Test BufrTableID comparisons
        CompareTester<BufrTableID> ct(BufrTableID(98, 1, 0, 15, 3));
        wassert(actual(ct(BufrTableID( 0, 0, 0,  0, 0), BufrTableID( 0, 0, 0, 14, 0))) == "none");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 14, 0), BufrTableID( 0, 0, 0, 20, 0))) == "secondonly");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 20, 0), BufrTableID( 3, 1, 0, 20, 3))) == "first");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 20, 0), BufrTableID( 3, 1, 0, 19, 3))) == "second");
        wassert(actual(ct(BufrTableID( 3, 1, 0, 19, 3), BufrTableID( 3, 1, 0, 20, 3))) == "first");
        wassert(actual(ct(BufrTableID( 3, 1, 0, 19, 3), BufrTableID( 0, 0, 0, 20, 0))) == "first");
        wassert(actual(ct(BufrTableID( 3, 1, 0, 19, 3), BufrTableID( 0, 0, 0, 19, 0))) == "second");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 19, 0), BufrTableID( 0, 0, 0, 16, 0))) == "second");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 16, 0), BufrTableID( 0, 0, 0, 15, 0))) == "second");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 15, 0), BufrTableID( 0, 0, 0, 15, 3))) == "same");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 15, 0), BufrTableID( 0, 1, 0, 15, 0))) == "same");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 15, 0), BufrTableID( 1, 1, 0, 15, 0))) == "first");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 15, 0), BufrTableID( 0, 1, 0, 15, 3))) == "same");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 15, 0), BufrTableID(98, 1, 0, 16, 3))) == "first");
        wassert(actual(ct(BufrTableID( 0, 0, 0, 15, 0), BufrTableID(98, 0, 0, 15, 0))) == "second");
        wassert(actual(ct(BufrTableID(98, 0, 0, 15, 0), BufrTableID(98, 1, 0, 15, 0))) == "second");
        wassert(actual(ct(BufrTableID(98, 1, 0, 15, 0), BufrTableID(98, 0, 0, 15, 5))) == "second");
        wassert(actual(ct(BufrTableID(98, 0, 0, 15, 5), BufrTableID(98, 0, 0, 15, 7))) == "first");
        wassert(actual(ct(BufrTableID(98, 0, 0, 15, 5), BufrTableID(98, 0, 0, 15, 3))) == "second");
        wassert(actual(ct(BufrTableID(98, 0, 0, 15, 3), BufrTableID(98, 1, 0, 15, 3))) == "second");
    }),
    Test("crextableid", [](Fixture& f) {
        // Test CrexTableID comparisons
        CompareTester<CrexTableID> ct(CrexTableID(1, 98, 0, 0, 15, 3, 0));
        wassert(actual(ct(CrexTableID(0,  0, 0, 0,  0, 0,  0), CrexTableID(1,  0, 0, 3, 15, 3, 15))) == "none");
        wassert(actual(ct(CrexTableID(1,  0, 0, 3, 15, 3, 15), CrexTableID(1,  0, 0, 0, 14, 0, 14))) == "none");
        wassert(actual(ct(CrexTableID(1,  0, 0, 3, 15, 3, 15), CrexTableID(1,  0, 0, 0, 20, 0, 20))) == "secondonly");
        wassert(actual(ct(CrexTableID(1,  0, 0, 0, 20, 0, 20), CrexTableID(1,  0, 0, 0, 21, 0, 21))) == "first");
        wassert(actual(ct(CrexTableID(1,  0, 0, 0, 20, 0, 20), CrexTableID(1,  0, 0, 0, 15, 0, 15))) == "second");
        wassert(actual(ct(CrexTableID(1,  0, 0, 0, 15, 0, 15), CrexTableID(1,  0, 0, 0, 15, 0, 15))) == "same");
        wassert(actual(ct(CrexTableID(1,  0, 0, 0, 15, 0, 15), CrexTableID(1, 98, 0, 0, 15, 0, 15))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 0, 15), CrexTableID(1, 98, 0, 0, 15, 1, 15))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 1, 15), CrexTableID(1, 98, 0, 0, 15, 6, 15))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 6, 15), CrexTableID(1, 98, 0, 0, 15, 8, 15))) == "first");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 6, 15), CrexTableID(1, 98, 0, 0, 15, 5, 15))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 5, 15), CrexTableID(1, 98, 0, 0, 15, 3, 15))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 3, 15), CrexTableID(0, 98, 0, 0, 15, 3, 15))) == "firstonly");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 3, 15), CrexTableID(2, 98, 0, 0, 15, 3, 15))) == "firstonly");
    }),
};

test_group newtg("tableinfo", tests);

}

