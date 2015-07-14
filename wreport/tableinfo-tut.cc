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
        CompareTester<CrexTableID> ct(CrexTableID(1, 98, 0, 0, 15, 0, 3));
        wassert(actual(ct(CrexTableID(0,  0, 0, 0,  0,  0, 0), CrexTableID(1,  0, 0, 3, 15, 15, 3))) == "none");
        wassert(actual(ct(CrexTableID(1,  0, 0, 3, 15, 15, 3), CrexTableID(1,  0, 0, 0, 14, 14, 0))) == "none");
        wassert(actual(ct(CrexTableID(1,  0, 0, 3, 15, 15, 3), CrexTableID(1,  0, 0, 0, 20, 20, 0))) == "secondonly");
        wassert(actual(ct(CrexTableID(1,  0, 0, 0, 20, 20, 0), CrexTableID(1,  0, 0, 0, 21, 21, 0))) == "first");
        wassert(actual(ct(CrexTableID(1,  0, 0, 0, 20, 20, 0), CrexTableID(1,  0, 0, 0, 15, 15, 0))) == "second");
        wassert(actual(ct(CrexTableID(1,  0, 0, 0, 15, 15, 0), CrexTableID(1,  0, 0, 0, 15, 15, 0))) == "same");
        wassert(actual(ct(CrexTableID(1,  0, 0, 0, 15, 15, 0), CrexTableID(1, 98, 0, 0, 15, 15, 0))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 15, 0), CrexTableID(1, 98, 0, 0, 15, 15, 1))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 15, 1), CrexTableID(1, 98, 0, 0, 15, 15, 6))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 15, 6), CrexTableID(1, 98, 0, 0, 15, 15, 8))) == "first");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 15, 6), CrexTableID(1, 98, 0, 0, 15, 15, 5))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 15, 5), CrexTableID(1, 98, 0, 0, 15, 15, 3))) == "second");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 15, 3), CrexTableID(0, 98, 0, 0, 15, 15, 3))) == "firstonly");
        wassert(actual(ct(CrexTableID(1, 98, 0, 0, 15, 15, 3), CrexTableID(2, 98, 0, 0, 15, 15, 3))) == "firstonly");
    }),
};

test_group newtg("tableinfo", tests);

}

