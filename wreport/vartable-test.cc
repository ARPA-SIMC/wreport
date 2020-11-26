#include "tests.h"
#include "vartable.h"
#include "utils/string.h"
#include "config.h"
#include <cstring>
#include <cstdlib>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

ostream& operator<<(ostream& out, Vartype t)
{
    return out << vartype_format(t);
}

string testdata_pathname(const std::string& basename)
{
    const char* dir = getenv("WREPORT_TESTDATA");
    if (!dir) dir = ".";
    return str::joinpath(dir, basename);
}

string table_pathname(const std::string& basename)
{
    const char* dir = getenv("WREPORT_TABLES");
    if (!dir) dir = TABLE_DIR;
    return str::joinpath(dir, basename);
}

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("crex", []() {
            // Test querying CREX tables
            const Vartable* table = Vartable::load_crex(testdata_pathname("test-crex-table.txt"));

            try {
                table->query(WR_VAR(0, 2, 99));
            } catch (error_notfound& e) {
                wassert(actual(e.what()).contains("002099"));
            }

            Varinfo info = table->query(WR_VAR(0, 1, 6));
            wassert(actual(info->code) == WR_VAR(0, 1, 6));
            wassert(actual(info->desc) == "AIRCRAFT FLIGHT NUMBER");
            wassert(actual(info->unit) == "CHARACTER");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->len) == 8u);
            wassert(actual(info->type) == Vartype::String);

            info = table->query(WR_VAR(0, 2, 114));
            wassert(actual(info->code) == WR_VAR(0, 2, 114));
            wassert(actual(info->desc) == "ANTENNA EFFECTIVE SURFACE AREA");
            wassert(actual(info->unit) == "M**2");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->len) == 5u);
            wassert(actual(info->type) == Vartype::Integer);

            info = table->query(WR_VAR(0, 2, 153));
            wassert(actual(info->code) == WR_VAR(0, 2, 153));
            wassert(actual(info->desc) == "SATELLITE CHANNEL CENTRE FREQUENCY");
            wassert(actual(info->unit) == "Hz");
            wassert(actual(info->scale) == -8) ;
            wassert(actual(info->len) == 8u);
            wassert(actual(info->type) == Vartype::Decimal);

            info = table->query(WR_VAR(0, 1, 3));
            wassert(actual(info->code) == WR_VAR(0, 1, 3));
            wassert(actual(info->desc) == "WMO REGION NUMBER/GEOGRAPHICAL AREA");
            wassert(actual(info->unit) == "CODE TABLE");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->len) == 1u);
            wassert(actual(info->type) == Vartype::Integer);

            info = table->query(WR_VAR(0, 2, 2));
            wassert(actual(info->code) == WR_VAR(0, 2, 2));
            wassert(actual(info->desc) == "TYPE OF INSTRUMENTATION FOR WIND MEASUREMENT");
            wassert(actual(info->unit) == "FLAG TABLE");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->len) == 2u);
            wassert(actual(info->type) == Vartype::Integer);
        });
        add_method("bufr", []() {
            // Test querying BUFR tables
            const Vartable* table = Vartable::load_bufr(testdata_pathname("test-bufr-table.txt"));

            try {
                table->query(WR_VAR(0, 2, 99));
            } catch (error_notfound& e) {
                wassert(actual(e.what()).contains("002099"));
            }

            Varinfo info = table->query(WR_VAR(0, 1, 6));
            wassert(actual(info->code) == WR_VAR(0, 1, 6));
            wassert(actual(info->desc) == "AIRCRAFT FLIGHT NUMBER");
            wassert(actual(info->unit) == "CCITTIA5");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->len) == 8u);
            wassert(actual(info->bit_len) == 64u);
            wassert(actual(info->type) == Vartype::String);

            info = table->query(WR_VAR(0, 2, 114));
            wassert(actual(info->code) == WR_VAR(0, 2, 114));
            wassert(actual(info->desc) == "ANTENNA EFFECTIVE SURFACE AREA");
            wassert(actual(info->unit) == "M**2");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->len) == 5u);
            wassert(actual(info->bit_len) == 15u);
            wassert(actual(info->imin) == 0);
            wassert(actual(info->imax) == 32766);
            wassert(actual(info->type) == Vartype::Integer);

            info = table->query(WR_VAR(0, 11, 35));
            wassert(actual(info->code) == WR_VAR(0, 11, 35));
            wassert(actual(info->desc) == "VERTICAL GUST ACCELERATION");
            wassert(actual(info->unit) == "M/S**2");
            wassert(actual(info->scale) == 2) ;
            wassert(actual(info->bit_ref) == -8192);
            wassert(actual(info->bit_len) == 14u);
            wassert(actual(info->len) == 5u);
            wassert(actual(info->type) == Vartype::Decimal);

            info = table->query(WR_VAR(0, 7, 31));
            wassert(actual(info->code) == WR_VAR(0, 7, 31));
            wassert(actual(info->desc) == "HEIGHT OF BAROMETER ABOVE MEAN SEA LEVEL");
            wassert(actual(info->unit) == "M");
            wassert(actual(info->scale) == 1) ;
            wassert(actual(info->bit_ref) == -4000);
            wassert(actual(info->bit_len) == 17u);
            wassert(actual(info->len) == 6u);
            wassert(actual(info->type) == Vartype::Decimal);

            info = table->query(WR_VAR(0, 1, 3));
            wassert(actual(info->code) == WR_VAR(0, 1, 3));
            wassert(actual(info->desc) == "WMO REGION NUMBER/GEOGRAPHICAL AREA");
            wassert(actual(info->unit) == "CODE TABLE");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->bit_ref) == 0);
            wassert(actual(info->bit_len) == 3u);
            wassert(actual(info->len) == 1u);
            wassert(actual(info->type) == Vartype::Integer);

            info = table->query(WR_VAR(0, 2, 2));
            wassert(actual(info->code) == WR_VAR(0, 2, 2));
            wassert(actual(info->desc) == "TYPE OF INSTRUMENTATION FOR WIND MEASUREMENT");
            wassert(actual(info->unit) == "FLAG TABLE");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->bit_ref) == 0);
            wassert(actual(info->bit_len) == 4u);
            wassert(actual(info->len) == 2u);
            wassert(actual(info->type) == Vartype::Integer);
        });
        add_method("bufr4", []() {
            // Test reading BUFR edition 4 tables
            const Vartable* table = Vartable::load_bufr(table_pathname("B0000000000098013102.txt"));

            try {
                table->query(WR_VAR(0, 2, 99));
            } catch (error_notfound& e) {
                wassert(actual(e.what()).contains("002099"));
            }

            Varinfo info = table->query(WR_VAR(0, 1, 6));
            wassert(actual(info->code) == WR_VAR(0, 1, 6));
            wassert(actual(info->desc) == "AIRCRAFT FLIGHT NUMBER");
            wassert(actual(info->unit) == "CCITTIA5");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->len) == 8u);
            wassert(actual(info->bit_len) == 64u);
            wassert(actual(info->type) == Vartype::String);

            info = table->query(WR_VAR(0, 2, 114));
            wassert(actual(info->code) == WR_VAR(0, 2, 114));
            wassert(actual(info->desc) == "ANTENNA EFFECTIVE SURFACE AREA");
            wassert(actual(info->unit) == "M**2");
            wassert(actual(info->scale) == 0) ;
            wassert(actual(info->len) == 5u);
            wassert(actual(info->bit_len) == 15u);
            wassert(actual(info->imin) == 0);
            wassert(actual(info->imax) == 32766);
            wassert(actual(info->type) == Vartype::Integer);

            info = table->query(WR_VAR(0, 11, 35));
            wassert(actual(info->code) == WR_VAR(0, 11, 35));
            wassert(actual(info->desc) == "VERTICAL GUST ACCELERATION");
            wassert(actual(info->unit) == "M/S**2");
            wassert(actual(info->scale) == 2) ;
            wassert(actual(info->bit_ref) == -8192);
            wassert(actual(info->bit_len) == 14u);
            wassert(actual(info->len) == 5u);
            wassert(actual(info->type) == Vartype::Decimal);

            table = Vartable::load_bufr(table_pathname("B0000000000000014000.txt"));

            info = table->query(WR_VAR(0, 15, 12));
            wassert(actual(info->code) == WR_VAR(0, 15, 12));
            wassert(actual(info->desc) == "TOTAL ELECTRON COUNT PER SQUARE METER");
            wassert(actual(info->unit) == "1/M**2");
            wassert(actual(info->scale) == -16) ;
            wassert(actual(info->bit_ref) == 0);
            wassert(actual(info->bit_len) == 6u);
            wassert(actual(info->len) == 2u);
            wassert(actual(info->type) == Vartype::Decimal);
        });
        add_method("wmo", []() {
            // Test reading WMO standard tables
            //const Vartable* table = NULL;
            /* table = */ Vartable::get_bufr("B0000000000000012000");
            /* table = */ Vartable::get_bufr("B0000000000000013000");
            /* table = */ Vartable::get_bufr("B0000000000000014000");
        });
    }
} test("vartable");

}
