#include "tests.h"
#include <wreport/vartable.h>
#include <wibble/string.h>
#include <cstring>
#include <cstdlib>

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

string testdata_pathname(const std::string& basename)
{
    const char* dir = getenv("WREPORT_TESTDATA");
    if (!dir) dir = ".";
    return wibble::str::joinpath(dir, basename);
}

string table_pathname(const std::string& basename)
{
    const char* dir = getenv("WREPORT_TABLES");
    if (!dir) dir = TABLE_DIR;
    return wibble::str::joinpath(dir, basename);
}

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("crex", [](Fixture& f) {
        // Test querying CREX tables
        const Vartable* table = Vartable::load_crex(testdata_pathname("test-crex-table.txt"));

        try {
            table->query(WR_VAR(0, 2, 99));
        } catch (error_notfound& e) {
            ensure_contains(e.what(), "002099");
        }

        Varinfo info = table->query(WR_VAR(0, 1, 6));
        ensure_equals(info->code, WR_VAR(0, 1, 6));
        ensure_equals(string(info->desc), string("AIRCRAFT FLIGHT NUMBER"));
        ensure_equals(string(info->unit), string("CHARACTER"));
        ensure_equals(info->scale, 0) ;
        ensure_equals(info->len, 8);
        wassert(actual(info->type) == Vartype::String);

        info = table->query(WR_VAR(0, 2, 114));
        ensure_equals(info->code, WR_VAR(0, 2, 114));
        ensure_equals(strcmp(info->desc, "ANTENNA EFFECTIVE SURFACE AREA"), 0);
        ensure_equals(strcmp(info->unit, "M**2"), 0);
        ensure_equals(info->scale, 0) ;
        ensure_equals(info->len, 5);
        wassert(actual(info->type) == Vartype::Integer);

        info = table->query(WR_VAR(0, 2, 153));
        ensure_equals(info->code, WR_VAR(0, 2, 153));
        ensure_equals(strcmp(info->desc, "SATELLITE CHANNEL CENTRE FREQUENCY"), 0);
        ensure_equals(strcmp(info->unit, "Hz"), 0);
        ensure_equals(info->scale, -8) ;
        ensure_equals(info->len, 8);
        wassert(actual(info->type) == Vartype::Decimal);
    }),
    Test("bufr", [](Fixture& f) {
        // Test querying BUFR tables
        const Vartable* table = Vartable::load_bufr(testdata_pathname("test-bufr-table.txt"));

        try {
            table->query(WR_VAR(0, 2, 99));
        } catch (error_notfound& e) {
            ensure(string(e.what()).find("002099") != string::npos);
        }

        Varinfo info = table->query(WR_VAR(0, 1, 6));
        ensure_equals(info->code, WR_VAR(0, 1, 6));
        ensure_equals(string(info->desc), string("AIRCRAFT FLIGHT NUMBER"));
        ensure_equals(string(info->unit), string("CCITTIA5"));
        ensure_equals(info->scale, 0) ;
        ensure_equals(info->len, 8);
        ensure_equals(info->bit_len, 64);
        wassert(actual(info->type) == Vartype::String);

        info = table->query(WR_VAR(0, 2, 114));
        ensure_equals(info->code, WR_VAR(0, 2, 114));
        ensure_equals(strcmp(info->desc, "ANTENNA EFFECTIVE SURFACE AREA"), 0);
        ensure_equals(strcmp(info->unit, "M**2"), 0);
        ensure_equals(info->scale, 0) ;
        ensure_equals(info->len, 5);
        ensure_equals(info->bit_len, 15);
        ensure_equals(info->imin, 0);
        ensure_equals(info->imax, 32766);
        wassert(actual(info->type) == Vartype::Integer);

        info = table->query(WR_VAR(0, 11, 35));
        ensure_equals(info->code, WR_VAR(0, 11, 35));
        ensure_equals(strcmp(info->desc, "VERTICAL GUST ACCELERATION"), 0);
        ensure_equals(strcmp(info->unit, "M/S**2"), 0);
        ensure_equals(info->scale, 2) ;
        ensure_equals(info->bit_ref, -8192);
        ensure_equals(info->bit_len, 14);
        ensure_equals(info->len, 5);
        wassert(actual(info->type) == Vartype::Decimal);

        info = table->query(WR_VAR(0, 7, 31));
        ensure_equals(info->code, WR_VAR(0, 7, 31));
        ensure_equals(string(info->desc), string("HEIGHT OF BAROMETER ABOVE MEAN SEA LEVEL"));
        ensure_equals(string(info->unit), "M");
        ensure_equals(info->scale, 1) ;
        ensure_equals(info->bit_ref, -4000);
        ensure_equals(info->bit_len, 17);
        ensure_equals(info->len, 6);
        wassert(actual(info->type) == Vartype::Decimal);
    }),
    Test("bufr4", [](Fixture& f) {
        // Test reading BUFR edition 4 tables
        const Vartable* table = Vartable::load_bufr(table_pathname("B0000000000098013102.txt"));

        try {
            table->query(WR_VAR(0, 2, 99));
        } catch (error_notfound& e) {
            ensure(string(e.what()).find("002099") != string::npos);
        }

        Varinfo info = table->query(WR_VAR(0, 1, 6));
        ensure_equals(info->code, WR_VAR(0, 1, 6));
        ensure_equals(string(info->desc), string("AIRCRAFT FLIGHT NUMBER"));
        ensure_equals(string(info->unit), string("CCITTIA5"));
        ensure_equals(info->scale, 0) ;
        ensure_equals(info->len, 8);
        ensure_equals(info->bit_len, 64);
        wassert(actual(info->type) == Vartype::String);

        info = table->query(WR_VAR(0, 2, 114));
        ensure_equals(info->code, WR_VAR(0, 2, 114));
        ensure_equals(strcmp(info->desc, "ANTENNA EFFECTIVE SURFACE AREA"), 0);
        ensure_equals(strcmp(info->unit, "M**2"), 0);
        ensure_equals(info->scale, 0) ;
        ensure_equals(info->len, 5);
        ensure_equals(info->bit_len, 15);
        ensure_equals(info->imin, 0);
        ensure_equals(info->imax, 32766);
        wassert(actual(info->type) == Vartype::Integer);

        info = table->query(WR_VAR(0, 11, 35));
        ensure_equals(info->code, WR_VAR(0, 11, 35));
        ensure_equals(strcmp(info->desc, "VERTICAL GUST ACCELERATION"), 0);
        ensure_equals(strcmp(info->unit, "M/S**2"), 0);
        ensure_equals(info->scale, 2) ;
        ensure_equals(info->bit_ref, -8192);
        ensure_equals(info->bit_len, 14);
        ensure_equals(info->len, 5);
        wassert(actual(info->type) == Vartype::Decimal);

        table = Vartable::load_bufr(table_pathname("B0000000000000014000.txt"));

        info = table->query(WR_VAR(0, 15, 12));
        ensure_equals(info->code, WR_VAR(0, 15, 12));
        ensure_equals(strcmp(info->desc, "TOTAL ELECTRON COUNT PER SQUARE METER"), 0);
        ensure_equals(strcmp(info->unit, "1/M**2"), 0);
        ensure_equals(info->scale, -16) ;
        ensure_equals(info->bit_ref, 0);
        ensure_equals(info->bit_len, 6);
        ensure_equals(info->len, 2);
        wassert(actual(info->type) == Vartype::Decimal);
    }),
    Test("wmo", [](Fixture& f) {
        // Test reading WMO standard tables
        //const Vartable* table = NULL;
        /* table = */ Vartable::get_bufr("B0000000000000012000");
        /* table = */ Vartable::get_bufr("B0000000000000013000");
        /* table = */ Vartable::get_bufr("B0000000000000014000");
    }),
};

test_group newtg("vartable", tests);

}
