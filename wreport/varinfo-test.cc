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
    add_method("compute_type", []() {
        _Varinfo info;
        info.set_bufr(WR_VAR(2, 20, 0), "test", "CCITTIA5", 0, 0, 24);
        wassert(actual(info.type) == Vartype::String);

        info.set_bufr(WR_VAR(0, 15, 194), "test", "?", 10, 0, 17);
        wassert(actual(info.type) == Vartype::Decimal);

        info.set_bufr(WR_VAR(0, 15, 194), "test", "?", 0, 0, 17);
        wassert(actual(info.type) == Vartype::Integer);
    });

    add_method("set_string", []() {
        _Varinfo info;
        info.set_string(WR_VAR(2, 20, 0), "test", 10);
        wassert(actual(info.code) == WR_VAR(2, 20, 0));
        wassert(actual(string(info.desc)) == "test");
        wassert(actual(string(info.unit)) == "CCITTIA5");
        wassert(actual(info.len) == 10u);
        wassert(actual(info.bit_len) == 80u);
        wassert(actual(info.bit_ref) == 0);
        wassert(actual(info.type) == Vartype::String);
        wassert(actual(info.imin) == 0);
        wassert(actual(info.imax) == 0);
        wassert(actual(info.dmin) == 0.0);
        wassert(actual(info.dmax) == 0.0);
    });

    add_method("set_binary", []() {
        _Varinfo info;
        info.set_binary(WR_VAR(0, 0, 0), "TEST", 6);
        wassert(actual(info.len) == 1u);
        wassert(actual(info.bit_len) == 6u);
        info.set_binary(WR_VAR(0, 0, 0), "TEST", 12);
        wassert(actual(info.len) == 2u);
        info.set_binary(WR_VAR(0, 0, 0), "TEST", 16);
        wassert(actual(info.len) == 2u);
        info.set_binary(WR_VAR(0, 0, 0), "TEST", 17);
        wassert(actual(info.len) == 3u);

        wassert(actual(info.type) == Vartype::Binary);
        wassert(actual(info.imin) == 0);
        wassert(actual(info.imax) == 0);
        wassert(actual(info.dmin) == 0.0);
        wassert(actual(info.dmax) == 0.0);
    });

    add_method("set_bufr_string", []() {
        _Varinfo info;
        info.set_bufr(WR_VAR(0, 0, 0), "test", "CCITTIA5", 0, 0, 24);
        wassert(actual(info.len) == 3u);
        wassert(actual(info.bit_len) == 24u);
        info.set_bufr(WR_VAR(0, 0, 0), "test", "CCITTIA5", 0, 0, 8);
        wassert(actual(info.len) == 1u);
        wassert(actual(info.bit_len) == 8u);
        info.set_bufr(WR_VAR(0, 0, 0), "test", "CCITTIA5", 0, 0, 4);
        wassert(actual(info.len) == 1u);
        wassert(actual(info.bit_len) == 4u);
        info.set_bufr(WR_VAR(0, 0, 0), "test", "CCITTIA5", 0, 0, 25);
        wassert(actual(info.len) == 4u);
        wassert(actual(info.bit_len) == 25u);

        wassert(actual(info.type) == Vartype::String);
        wassert(actual(info.bit_ref) == 0);
        wassert(actual(info.imin) == 0);
        wassert(actual(info.imax) == 0);
        wassert(actual(info.dmin) == 0.0);
        wassert(actual(info.dmax) == 0.0);
    });

    add_method("set_crex_string", []() {
        _Varinfo info;
        info.set_crex(WR_VAR(0, 0, 0), "test", "CHARACTER", 0, 24);
        wassert(actual(info.len) == 24u);
        info.set_crex(WR_VAR(0, 0, 0), "test", "CHARACTER", 0, 8);
        wassert(actual(info.len) == 8u);
        info.set_crex(WR_VAR(0, 0, 0), "test", "CHARACTER", 0, 4);
        wassert(actual(info.len) == 4u);
        info.set_crex(WR_VAR(0, 0, 0), "test", "CHARACTER", 0, 25);
        wassert(actual(info.len) == 25u);

        wassert(actual(info.type) == Vartype::String);
        wassert(actual(info.bit_ref) == 0);
        wassert(actual(info.bit_len) == 0u);
        wassert(actual(info.imin) == 0);
        wassert(actual(info.imax) == 0);
        wassert(actual(info.dmin) == 0.0);
        wassert(actual(info.dmax) == 0.0);
    });

    add_method("set_bufr_no_ref", []() {
        struct Testdata
        {
            int bit_len;
            unsigned len;
        };
        std::vector<Testdata> testdata{
            {1,  1 },
            {3,  1 },
            {4,  2 },
            {31, 10},
        };
        for (const auto& td : testdata)
        {
            WREPORT_TEST_INFO(info);
            info << "bit_len: " << td.bit_len;

            _Varinfo vi;
            vi.set_bufr(WR_VAR(0, 0, 0), "Desc", "M", 0, 0, td.bit_len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == 0);
            wassert(actual(vi.imax) == (1 << td.bit_len) - 2);
            wassert(actual(vi.dmin) == 0);
            wassert(actual(vi.dmax) == (1 << td.bit_len) - 2);
            wassert(actual(vi.type) == Vartype::Integer);

            vi.set_bufr(WR_VAR(0, 0, 0), "Desc", "M", -3, 0, td.bit_len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == 0);
            wassert(actual(vi.imax) == (1 << td.bit_len) - 2);
            wassert(actual(vi.dmin) == 0);
            wassert(actual(vi.dmax) == ((1 << td.bit_len) - 2) * 1000.0);
            wassert(actual(vi.type) == Vartype::Decimal);

            vi.set_bufr(WR_VAR(0, 0, 0), "Desc", "M", 3, 0, td.bit_len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == 0);
            wassert(actual(vi.imax) == (1 << td.bit_len) - 2);
            wassert(actual(vi.dmin) == 0);
            wassert(actual(vi.dmax) == ((1 << td.bit_len) - 2) * 0.001);
            wassert(actual(vi.type) == Vartype::Decimal);
        }
    });

    add_method("set_bufr_ref", []() {
        struct Testdata
        {
            int bit_ref;
            int bit_len;
            unsigned len;
            int imax;
        };
        std::vector<Testdata> testdata{
            {-1,          1,  1,  -1         },
            {1,           1,  1,  1          },
            {-10,         3,  2,  -4         },
            {1000,        3,  4,  1006       },
            {-0x7ffffffe, 3,  10, -0x7ffffff8},
            {-0x7ffffffe, 31, 10, 0          },
        };
        for (const auto& td : testdata)
        {
            WREPORT_TEST_INFO(info);
            info << "bit_ref: " << td.bit_ref << ", bit_len: " << td.bit_len;

            _Varinfo vi;
            vi.set_bufr(WR_VAR(0, 0, 0), "Desc", "M", 0, td.bit_ref,
                        td.bit_len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.bit_ref);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.bit_ref);
            wassert(actual(vi.dmax) == td.imax);
            wassert(actual(vi.type) == Vartype::Integer);

            vi.set_bufr(WR_VAR(0, 0, 0), "Desc", "M", -3, td.bit_ref,
                        td.bit_len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.bit_ref);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.bit_ref * 1000.0);
            wassert(actual(vi.dmax) == td.imax * 1000.0);
            wassert(actual(vi.type) == Vartype::Decimal);

            vi.set_bufr(WR_VAR(0, 0, 0), "Desc", "M", 3, td.bit_ref,
                        td.bit_len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.bit_ref);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.bit_ref * 0.001);
            wassert(actual(vi.dmax) == td.imax * 0.001);
            wassert(actual(vi.type) == Vartype::Decimal);
        }
    });

    add_method("set_crex", []() {
        struct Testdata
        {
            unsigned len;
            int imin;
            int imax;
        };
        std::vector<Testdata> testdata{
            {1,  -9,         8        },
            {3,  -999,       998      },
            {9,  -999999999, 999999998},
            {10, INT_MIN,    INT_MAX  },
            {31, INT_MIN,    INT_MAX  },
        };
        for (const auto& td : testdata)
        {
            WREPORT_TEST_INFO(info);
            info << "len: " << td.len;

            _Varinfo vi;
            vi.set_crex(WR_VAR(0, 0, 0), "Desc", "M", 0, td.len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.imin);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.imin);
            wassert(actual(vi.dmax) == td.imax);
            wassert(actual(vi.type) == Vartype::Integer);

            vi.set_crex(WR_VAR(0, 0, 0), "Desc", "M", -3, td.len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.imin);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.imin * 1000.0);
            wassert(actual(vi.dmax) == td.imax * 1000.0);
            wassert(actual(vi.type) == Vartype::Decimal);

            vi.set_crex(WR_VAR(0, 0, 0), "Desc", "M", 3, td.len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.imin);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin).almost_equal(td.imin * 0.001, 5));
            wassert(actual(vi.dmax).almost_equal(td.imax * 0.001, 5));
            wassert(actual(vi.type) == Vartype::Decimal);
        }
    });

    add_method("compute_bufr_length_integer", []() {
        _Varinfo info;

        auto e = wassert_throws(
            error_consistency,
            info.set_bufr(WR_VAR(0, 1, 2), "test", "M", 0, 1, 31));
        wassert(actual(e.what()) ==
                "001002 scaled value does not fit in a signed 32bit integer "
                "(31 bits with a base value of 1)");

        e = wassert_throws(
            error_consistency,
            info.set_bufr(WR_VAR(0, 1, 2), "test", "M", 0, 0x7ffffffe, 31));
        wassert(actual(e.what()) ==
                "001002 scaled value does not fit in a signed 32bit integer "
                "(31 bits with a base value of 2147483646)");
    });

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
        info.set_bufr(WR_VAR(0, 6, 2),               // Var
                      "LONGITUDE (COARSE ACCURACY)", // Desc
                      "DEGREE",                      // Unit
                      2, -18000, 16);                // Scale, bit_ref, bit_len
        wassert(actual(info.len) == 5u);
        wassert(actual(info.imin) == -18000);
        wassert(actual(info.imax) == -18000 + (1 << 16) - 2);
        wassert(actual(info.dmin) == -180.00);
        wassert(actual(info.dmax) == 475.34);
        wassert(actual(info.type) == Vartype::Decimal);
        // ensure_equals(info->decode_int(16755), -12.45);
        wassert(actual(info.decode_binary(16755)) == -12.45);

        info.set_crex(WR_VAR(0, 6, 2),               // Var
                      "LONGITUDE (COARSE ACCURACY)", // Desc
                      "DEGREE",                      // Unit
                      2, 5);                         // Scale, Length
        wassert(actual(info.imin) == -99999);
        wassert(actual(info.imax) == 99998);
        wassert(actual(info.dmin) == -999.99);
        wassert(actual(info.dmax) == 999.98);
        wassert(actual(info.type) == Vartype::Decimal);
    });
    add_method("issue42", []() {
        // Test binary varinfos
        _Varinfo info;

        info.set_bufr(WR_VAR(0, 13, 11), "TEST", "KG M-2", 1, -1, 14);
        wassert(actual(info.len) == 5u);
        wassert(actual(info.imin) == -1);
        wassert(actual(info.imax) == 16381);
        wassert(actual(info.dmin) == -0.1);
        wassert(actual(info.dmax) == 1638.1);

        wassert(actual(info.decode_decimal(1)) == 0.1);
        wassert(actual(info.decode_decimal(-1)) == -0.1);
        wassert(actual(info.decode_binary(0)) == -0.1);
        wassert(actual(info.decode_binary(0x3fff)) == 1638.2);

        info.set_crex(WR_VAR(0, 13, 11), "TEST", "KG M-2", 1, 5);
        wassert(actual(info.imin) == -99999);
        wassert(actual(info.imax) == 99998);
        wassert(actual(info.dmin) == -9999.9);
        wassert(actual(info.dmax) == 9999.8);
    });
    add_method("issue59", []() {
        _Varinfo info;
        const char* desc = "EARTH'S LOCAL RADIUS OF CURVATURE";

        info.set_bufr(WR_VAR(0, 10, 35), desc, "M", 1, 62000000, 22);
        wassert(actual(info.imin) == 62000000);
        wassert(actual(info.imax) == 66194302 /* 62000000 + (1 << 22) - 2 */);
        wassert(actual(info.dmin) == 6200000.0);
        wassert(actual(info.dmax) == 6619430.2);

        info.set_crex(WR_VAR(0, 10, 35), desc, "M", 1, 8);
        wassert(actual(info.imin) == -99999999);
        wassert(actual(info.imax) == 99999998);
        wassert(actual(info.dmin) == -9999999.9);
        wassert(actual(info.dmax) == 9999999.8);
    });
}

} // namespace
