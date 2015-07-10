#include "test-utils-wreport.h"
#include "tabledir.h"
#include <set>

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

struct BufrQueryTester
{
    tabledir::BufrQuery q;
    vector<tabledir::BufrTable> candidates;

    BufrQueryTester(const BufrTableID& id) : q(id)
    {
        // Reserve a large amount to avoid reallocations that invalidate q.result
        candidates.reserve(100);
    }

    bool try_entry(const BufrTableID& id)
    {
        using namespace wreport::tabledir;
        candidates.push_back(BufrTable(id, "a", "a"));
        bool res = q.is_better(candidates.back());
        if (res) q.result = &candidates.back();
        return res;
    }
};

struct CrexQueryTester
{
    tabledir::CrexQuery q;
    vector<tabledir::CrexTable> candidates;

    CrexQueryTester(const CrexTableID& id)
        : q(id)
    {
        // Reserve a large amount to avoid reallocations that invalidate q.result
        candidates.reserve(100);
    }

    bool try_entry(const CrexTableID& id)
    {
        using namespace wreport::tabledir;
        candidates.push_back(CrexTable(id, "a", "a"));
        bool res = q.is_better(candidates.back());
        if (res) q.result = &candidates.back();
        return res;
    }
};

std::vector<Test> tests {
    Test("internals", [](Fixture& f) {
        // Test Table, BufrTable, CrexTable
        using namespace wreport::tabledir;

        BufrTable t(BufrTableID(0, 0, 0, 0, 0), "/antani", "B12345.txt");
        wassert(actual(t.btable_id) == "B12345");
        wassert(actual(t.dtable_id) == "D12345");
        wassert(actual(t.btable_pathname) == "/antani/B12345.txt");
        wassert(actual(t.dtable_pathname) == "/antani/D12345.txt");
    }),
    Test("bufrquery", [](Fixture& f) {
        // Test BufrQuery
        using namespace wreport::tabledir;

        BufrQueryTester qt(BufrTableID(98, 1, 0, 15, 3));
        wassert(actual(qt.try_entry(BufrTableID( 0, 0, 0, 14, 0))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID( 0, 0, 0, 20, 0))).istrue());
        wassert(actual(qt.try_entry(BufrTableID( 3, 1, 0, 20, 3))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID( 3, 1, 0, 19, 3))).istrue());
        wassert(actual(qt.try_entry(BufrTableID( 3, 1, 0, 20, 3))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID( 0, 0, 0, 20, 0))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID( 0, 0, 0, 19, 0))).istrue());
        wassert(actual(qt.try_entry(BufrTableID( 0, 0, 0, 16, 0))).istrue());
        wassert(actual(qt.try_entry(BufrTableID( 0, 0, 0, 15, 0))).istrue());
        wassert(actual(qt.try_entry(BufrTableID( 0, 0, 0, 15, 3))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID( 0, 1, 0, 15, 0))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID( 0, 1, 0, 15, 3))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID(98, 1, 0, 16, 3))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID(98, 0, 0, 15, 0))).istrue());
        wassert(actual(qt.try_entry(BufrTableID(98, 1, 0, 15, 0))).istrue());
        wassert(actual(qt.try_entry(BufrTableID(98, 0, 0, 15, 5))).istrue());
        wassert(actual(qt.try_entry(BufrTableID(98, 0, 0, 15, 7))).isfalse());
        wassert(actual(qt.try_entry(BufrTableID(98, 0, 0, 15, 3))).istrue());
        wassert(actual(qt.try_entry(BufrTableID(98, 1, 0, 15, 3))).istrue());
    }),
    Test("crexquery", [](Fixture& f) {
        // Test CrexQuery
        using namespace wreport::tabledir;

        CrexQueryTester qt(CrexTableID(0, 0, 0, 15, 3, 0));
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 3, 15, 3, 0))).isfalse());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 14, 0, 0))).isfalse());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 20, 0, 0))).istrue());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 21, 0, 0))).isfalse());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 15, 0, 0))).istrue());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 15, 0, 0))).isfalse());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 15, 1, 0))).istrue());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 15, 6, 0))).istrue());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 15, 8, 0))).isfalse());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 15, 5, 0))).istrue());
        wassert(actual(qt.try_entry(CrexTableID(0, 0, 0, 15, 3, 0))).istrue());
    }),
    Test("tabledir", [](Fixture& f) {
        // Test Tabledir

        // Get the default Tabledir
        auto td = tabledir::Tabledir::get();

        const tabledir::BufrTable* bt;
        const tabledir::CrexTable* ct;

        bt = td.find_bufr(BufrTableID(0, 0, 0, 10, 0));
        wassert(actual(bt != 0).istrue());
        wassert(actual(bt->id.originating_centre) == 0);
        wassert(actual(bt->id.originating_subcentre) == 0);
        wassert(actual(bt->id.master_table) == 0);
        wassert(actual(bt->id.master_table_version_number) == 11);
        wassert(actual(bt->id.master_table_version_number_local) == 0);

        bt = td.find_bufr(BufrTableID(98, 0, 0, 6, 1));
        wassert(actual(bt != 0).istrue());
        wassert(actual((int)bt->id.originating_centre) == 98);
        wassert(actual((int)bt->id.originating_subcentre) == 0);
        wassert(actual((int)bt->id.master_table) == 0);
        wassert(actual((int)bt->id.master_table_version_number) == 6);
        wassert(actual((int)bt->id.master_table_version_number_local) == 1);

        /// Find a CREX table
        ct = td.find_crex(CrexTableID(0, 0, 0, 15, 0, 0));
        wassert(actual(ct != 0).istrue());
        wassert(actual(ct->id.originating_centre) == 0);
        wassert(actual(ct->id.originating_subcentre) == 0);
        wassert(actual(ct->id.master_table) == 0);
        wassert(actual(ct->id.master_table_version_number) == 16);
        wassert(actual(ct->id.master_table_version_number_local) == 0);
    }),
};

test_group newtg("tabledir", tests);

}
