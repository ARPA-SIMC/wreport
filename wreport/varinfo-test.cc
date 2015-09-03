#include "tests.h"
#include "varinfo.h"
#include <cstring>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

ostream& operator<<(ostream& out, Vartype t)
{
    return out << vartype_format(t);
}

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        // Test varcode encoding functions
        add_method("encode", []() {
            wassert(actual(WR_VAR(0, 0, 0)) == 0);
            wassert(actual(WR_VAR(0, 0, 255)) == 0xff);
            wassert(actual(WR_VAR(0, 1, 0)) == 0x100);
            wassert(actual(WR_VAR(0, 63, 0)) == 0x3f00);
            wassert(actual(WR_VAR(0, 63, 255)) == 0x3fff);
            wassert(actual(WR_VAR(1, 0, 0)) == 0x4000);
            wassert(actual(WR_VAR(2, 0, 255)) == 0x80ff);
            wassert(actual(WR_VAR(3, 1, 0)) == 0xc100);
            wassert(actual(WR_VAR(1, 63, 0)) == 0x7f00);
            wassert(actual(WR_VAR(2, 63, 255)) == 0xbfff);
            wassert(actual(WR_VAR(3, 63, 255)) == 0xffff);

            wassert(actual(WR_STRING_TO_VAR("12345")) == WR_VAR(0, 12, 345));
            wassert(actual(WR_STRING_TO_VAR("00345")) == WR_VAR(0, 0, 345));
            wassert(actual(WR_STRING_TO_VAR("00000")) == WR_VAR(0, 0, 0));
            wassert(actual(WR_STRING_TO_VAR("63255")) == WR_VAR(0, 63, 255));

            wassert(actual(varcode_parse("B12345")) == WR_VAR(0, 12, 345));
            wassert(actual(varcode_parse("R00345")) == WR_VAR(1, 0, 345));
            wassert(actual(varcode_parse("C00000")) == WR_VAR(2, 0, 0));
            wassert(actual(varcode_parse("D63255")) == WR_VAR(3, 63, 255));
            wassert(actual(varcode_parse("012345")) == WR_VAR(0, 12, 345));
            wassert(actual(varcode_parse("100345")) == WR_VAR(1, 0, 345));
            wassert(actual(varcode_parse("200000")) == WR_VAR(2, 0, 0));
            wassert(actual(varcode_parse("363255")) == WR_VAR(3, 63, 255));

            wassert(actual(varcode_format(WR_VAR(0, 1, 2))) == "B01002");
            wassert(actual(varcode_format(WR_VAR(1, 1, 2))) == "R01002");
            wassert(actual(varcode_format(WR_VAR(2, 1, 2))) == "C01002");
            wassert(actual(varcode_format(WR_VAR(3, 1, 2))) == "D01002");
            wassert(actual(varcode_format(WR_VAR(4000, 1, 2))) == "B01002");
        });
        add_method("set", []() {
            // Test varinfo set
            _Varinfo info;
            info.set_string(WR_VAR(2, 20, 0), "test", 10);
            wassert(actual(info.code) == WR_VAR(2, 20, 0));
            wassert(actual(string(info.desc)) == "test");
            wassert(actual(string(info.unit)) == "CCITTIA5");
            wassert(actual(info.len) == 10u);
            wassert(actual(info.bit_len) == 80u);
        });
        add_method("bounds", []() {
            // Test the calculation of bounds
            _Varinfo info;
            info.set_bufr(WR_VAR(0, 15, 194),        // Var
                  "[SIM] O3 Concentration", // Desc
                  "KG/M**3",            // Unit
                  10, 5, 0, 17);     // Scale, len, bit_ref, bit_len
            info.compute_range();
            wassert(actual(info.dmin) == 0);
            wassert(actual(info.dmax) == 9.9998e-06);
            wassert(actual(info.type) == Vartype::Decimal);
        });
        add_method("encode_doubles", []() {
            // Test encoding doubles to ints
            _Varinfo info;
            info.set_bufr(WR_VAR(0, 6, 2),      // Var
                  "LONGITUDE (COARSE ACCURACY)",// Desc
                  "DEGREE",         // Unit
                  2, 5, -18000, 16);     // Scale, len, bit_ref, bit_len
            info.compute_range();
            wassert(actual(info.dmin) == -180);
            wassert(actual(info.dmax) == 475.34);
            wassert(actual(info.type) == Vartype::Decimal);
            // ensure_equals(info->decode_int(16755), -12.45);
            wassert(actual(info.decode_binary(16755)) == -12.45);
        });
        add_method("range_crex", []() {
            // Test range checking for CREX
            _Varinfo info;
            // 012003 DEW-POINT TEMPERATURE in B000103.txt
            info.set_crex(WR_VAR(0, 12, 3),
                     "DEW-POINT TEMPERATURE", "C",
                     1, 3);
            wassert(actual(info.dmin) == -99.9);
            wassert(actual(info.dmax) == 99.8);
        });
        add_method("set_binary", []() {
            // Test binary varinfos
            _Varinfo info;
            info.set_binary(WR_VAR(0, 0, 0), "TEST", 6);
            wassert(actual(info.len) == 1u);
            wassert(actual(info.bit_len) == 6u);
            wassert(actual(info.imin) == 0);
            wassert(actual(info.imax) == 0);
            wassert(actual(info.dmin) == 0);
            wassert(actual(info.dmax) == 0);
        });
    }
} tests("varinfo");

}
