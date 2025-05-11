#include "internals/varinfo.h"
#include "tests.h"
#include "varinfo.h"
#include <climits>
#include <cmath>
#include <cstring>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} tests("varinfo");

void Tests::register_tests()
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
    add_method("encode_doubles", []() {
        // Test encoding doubles to ints
        _Varinfo info;
        varinfo::set_bufr(info, WR_VAR(0, 6, 2),         // Var
                          "LONGITUDE (COARSE ACCURACY)", // Desc
                          "DEGREE",                      // Unit
                          16, -18000, 2); // bit_len, bit_ref, Scale
        wassert(actual(info.len) == 5u);
        wassert(actual(info.imin) == -18000);
        wassert(actual(info.imax) == -18000 + (1 << 16) - 2);
        wassert(actual(info.dmin) == -180.00);
        wassert(actual(info.dmax) == 475.34);
        wassert(actual(info.type) == Vartype::Decimal);
        // ensure_equals(info->decode_int(16755), -12.45);
        wassert(actual(info.decode_binary(16755)) == -12.45);

        varinfo::set_crex(info, WR_VAR(0, 6, 2),         // Var
                          "LONGITUDE (COARSE ACCURACY)", // Desc
                          "DEGREE",                      // Unit
                          5, 2);                         // Scale, Length
        wassert(actual(info.imin) == -99999);
        wassert(actual(info.imax) == 99998);
        wassert(actual(info.dmin) == -999.99);
        wassert(actual(info.dmax) == 999.98);
        wassert(actual(info.type) == Vartype::Decimal);
    });
    add_method("issue42", []() {
        // Test binary varinfos
        _Varinfo info;

        varinfo::set_bufr(info, WR_VAR(0, 13, 11), "TEST", "KG M-2", 14, -1, 1);
        wassert(actual(info.len) == 5u);
        wassert(actual(info.imin) == -1);
        wassert(actual(info.imax) == 16381);
        wassert(actual(info.dmin) == -0.1);
        wassert(actual(info.dmax) == 1638.1);

        wassert(actual(info.decode_decimal(1)) == 0.1);
        wassert(actual(info.decode_decimal(-1)) == -0.1);
        wassert(actual(info.decode_binary(0)) == -0.1);
        wassert(actual(info.decode_binary(0x3fff)) == 1638.2);

        varinfo::set_crex(info, WR_VAR(0, 13, 11), "TEST", "KG M-2", 5, 1);
        wassert(actual(info.imin) == -99999);
        wassert(actual(info.imax) == 99998);
        wassert(actual(info.dmin) == -9999.9);
        wassert(actual(info.dmax) == 9999.8);
    });
    add_method("issue59", []() {
        _Varinfo info;
        const char* desc = "EARTH'S LOCAL RADIUS OF CURVATURE";

        varinfo::set_bufr(info, WR_VAR(0, 10, 35), desc, "M", 22, 62000000, 1);
        wassert(actual(info.imin) == 62000000);
        wassert(actual(info.imax) == 66194302 /* 62000000 + (1 << 22) - 2 */);
        wassert(actual(info.dmin) == 6200000.0);
        wassert(actual(info.dmax) == 6619430.2);

        varinfo::set_crex(info, WR_VAR(0, 10, 35), desc, "M", 8, 1);
        wassert(actual(info.imin) == -99999999);
        wassert(actual(info.imax) == 99999998);
        wassert(actual(info.dmin) == -9999999.9);
        wassert(actual(info.dmax) == 9999999.8);
    });
    add_method("varinfo_create_bufr", []() {
        Varinfo info =
            varinfo_create_bufr(WR_VAR(0, 1, 2), "TEST", "NUMBER", 4);

        wassert(actual(info->len) == 2u);
        wassert(actual(info->imin) == 0);
        wassert(actual(info->imax) == 14);
        wassert(actual(info->dmin) == 0);
        wassert(actual(info->dmax) == 14);
        wassert(actual(info->type) == Vartype::Integer);

        varinfo_delete(std::move(info));
        wassert(actual(info) == nullptr);
    });
}

} // namespace
