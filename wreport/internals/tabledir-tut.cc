#include "test-utils-wreport.h"
#include "tabledir.h"
#include "vartable.h"
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
    Test("tabledir", [](Fixture& f) {
        // Test Tabledir

        // Get the default Tabledir
        auto& td = tabledir::Tabledir::get();

        const tabledir::Table* t;
        const tabledir::BufrTable* bt;
        const tabledir::CrexTable* ct;

        t = td.find_bufr(BufrTableID(0, 0, 0, 10, 0));
        wassert(actual(t != 0).istrue());
        bt = dynamic_cast<const tabledir::BufrTable*>(t);
        wassert(actual(bt != 0).istrue());
        wassert(actual(bt->id.originating_centre) == 0);
        wassert(actual(bt->id.originating_subcentre) == 0);
        wassert(actual(bt->id.master_table) == 0);
        wassert(actual(bt->id.master_table_version_number) == 11);
        wassert(actual(bt->id.master_table_version_number_local) == 0);

        t = td.find_bufr(BufrTableID(98, 0, 0, 6, 1));
        wassert(actual(t != 0).istrue());
        bt = dynamic_cast<const tabledir::BufrTable*>(t);
        wassert(actual(bt != 0).istrue());
        wassert(actual((int)bt->id.originating_centre) == 98);
        wassert(actual((int)bt->id.originating_subcentre) == 0);
        wassert(actual((int)bt->id.master_table) == 0);
        wassert(actual((int)bt->id.master_table_version_number) == 6);
        wassert(actual((int)bt->id.master_table_version_number_local) == 1);

        t = td.find_bufr(BufrTableID(0, 0, 0, 24, 0));
        wassert(actual(t != 0).istrue());
        bt = dynamic_cast<const tabledir::BufrTable*>(t);
        wassert(actual(bt != 0).istrue());
        wassert(actual((int)bt->id.originating_centre) == 0);
        wassert(actual((int)bt->id.originating_subcentre) == 0);
        wassert(actual((int)bt->id.master_table) == 0);
        wassert(actual((int)bt->id.master_table_version_number) == 24);
        wassert(actual((int)bt->id.master_table_version_number_local) == 0);
        const Vartable* vt = Vartable::load_bufr(t->btable_pathname);
        wassert(actual(vt->query(WR_VAR(0, 12, 101))->unit) == "K");

        /// Find a CREX table
        t = td.find_crex(CrexTableID(1, 0, 0, 0, 3, 0, 3));
        wassert(actual(t != 0).istrue());
        ct = dynamic_cast<const tabledir::CrexTable*>(t);
        wassert(actual(ct != 0).istrue());
        wassert(actual((int)ct->id.originating_centre) == 0xffff);
        wassert(actual((int)ct->id.originating_subcentre) == 0xffff);
        wassert(actual((int)ct->id.master_table) == 0);
        wassert(actual((int)ct->id.master_table_version_number) == 3);
        wassert(actual((int)ct->id.master_table_version_number_local) == 0xff);
        wassert(actual((int)ct->id.master_table_version_number_bufr) == 3);

        /// Find a CREX table from BUFR
        t = td.find_crex(CrexTableID(1, 0, 0, 0, 24, 0, 24));
        wassert(actual(t != 0).istrue());
        bt = dynamic_cast<const tabledir::BufrTable*>(t);
        wassert(actual(bt != 0).istrue());
        wassert(actual((int)bt->id.originating_centre) == 0);
        wassert(actual((int)bt->id.originating_subcentre) == 0);
        wassert(actual((int)bt->id.master_table) == 0);
        wassert(actual((int)bt->id.master_table_version_number) == 24);
        wassert(actual((int)bt->id.master_table_version_number_local) == 0);
        vt = Vartable::load_crex(t->btable_pathname);
        wassert(actual(vt->query(WR_VAR(0, 12, 101))->unit) == "C");

        // Find a non-BUFR non-CREX table by name
        t = td.find("test");
        wassert(actual(t != 0).istrue());
    }),
};

test_group newtg("tabledir", tests);

}
