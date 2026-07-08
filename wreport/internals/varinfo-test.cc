#include "varinfo.h"
#include "wreport/tests.h"
#include "wreport/varinfo.h"
#include <climits>

using namespace wreport;
using namespace wreport::tests;

namespace {
class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("internals_varinfo");

void Tests::register_tests()
{
    add_method("compute_type", []() {
        _Varinfo info;
        varinfo::set_bufr(info, WR_VAR(2, 20, 0), "test", "CCITTIA5", 24);
        wassert(actual(info.type) == Vartype::String);

        varinfo::set_bufr(info, WR_VAR(0, 15, 194), "test", "?", 17, 0, 10);
        wassert(actual(info.type) == Vartype::Decimal);

        varinfo::set_bufr(info, WR_VAR(0, 15, 194), "test", "?", 17);
        wassert(actual(info.type) == Vartype::Integer);
    });

    add_method("set_string", []() {
        _Varinfo info;
        varinfo::set_string(info, WR_VAR(2, 20, 0), "test", 10);
        wassert(actual(info.code) == WR_VAR(2, 20, 0));
        wassert(actual(std::string(info.desc)) == "test");
        wassert(actual(std::string(info.unit)) == "CCITTIA5");
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
        varinfo::set_binary(info, WR_VAR(0, 0, 0), "TEST", 6);
        wassert(actual(info.len) == 1u);
        wassert(actual(info.bit_len) == 6u);
        varinfo::set_binary(info, WR_VAR(0, 0, 0), "TEST", 12);
        wassert(actual(info.len) == 2u);
        varinfo::set_binary(info, WR_VAR(0, 0, 0), "TEST", 16);
        wassert(actual(info.len) == 2u);
        varinfo::set_binary(info, WR_VAR(0, 0, 0), "TEST", 17);
        wassert(actual(info.len) == 3u);

        wassert(actual(info.type) == Vartype::Binary);
        wassert(actual(info.imin) == 0);
        wassert(actual(info.imax) == 0);
        wassert(actual(info.dmin) == 0.0);
        wassert(actual(info.dmax) == 0.0);
    });

    add_method("set_bufr_string", []() {
        _Varinfo info;
        varinfo::set_bufr(info, WR_VAR(0, 0, 0), "test", "CCITTIA5", 24);
        wassert(actual(info.len) == 3u);
        wassert(actual(info.bit_len) == 24u);
        varinfo::set_bufr(info, WR_VAR(0, 0, 0), "test", "CCITTIA5", 8);
        wassert(actual(info.len) == 1u);
        wassert(actual(info.bit_len) == 8u);
        varinfo::set_bufr(info, WR_VAR(0, 0, 0), "test", "CCITTIA5", 4);
        wassert(actual(info.len) == 1u);
        wassert(actual(info.bit_len) == 4u);
        varinfo::set_bufr(info, WR_VAR(0, 0, 0), "test", "CCITTIA5", 25);
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
        varinfo::set_crex(info, WR_VAR(0, 0, 0), "test", "CHARACTER", 24);
        wassert(actual(info.len) == 24u);
        varinfo::set_crex(info, WR_VAR(0, 0, 0), "test", "CHARACTER", 8);
        wassert(actual(info.len) == 8u);
        varinfo::set_crex(info, WR_VAR(0, 0, 0), "test", "CHARACTER", 4);
        wassert(actual(info.len) == 4u);
        varinfo::set_crex(info, WR_VAR(0, 0, 0), "test", "CHARACTER", 25);
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
            varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "Desc", "M", td.bit_len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == 0);
            wassert(actual(vi.imax) == (1 << td.bit_len) - 2);
            wassert(actual(vi.dmin) == 0);
            wassert(actual(vi.dmax) == (1 << td.bit_len) - 2);
            wassert(actual(vi.type) == Vartype::Integer);

            varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "Desc", "M", td.bit_len, 0,
                              -3);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == 0);
            wassert(actual(vi.imax) == (1 << td.bit_len) - 2);
            wassert(actual(vi.dmin) == 0);
            wassert(actual(vi.dmax) == ((1 << td.bit_len) - 2) * 1000.0);
            wassert(actual(vi.type) == Vartype::Decimal);

            varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "Desc", "M", td.bit_len, 0,
                              3);
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
            varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "Desc", "M", td.bit_len,
                              td.bit_ref);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.bit_ref);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.bit_ref);
            wassert(actual(vi.dmax) == td.imax);
            wassert(actual(vi.type) == Vartype::Integer);

            varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "Desc", "M", td.bit_len,
                              td.bit_ref, -3);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.bit_ref);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.bit_ref * 1000.0);
            wassert(actual(vi.dmax) == td.imax * 1000.0);
            wassert(actual(vi.type) == Vartype::Decimal);

            varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "Desc", "M", td.bit_len,
                              td.bit_ref, 3);
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
            varinfo::set_crex(vi, WR_VAR(0, 0, 0), "Desc", "M", td.len);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.imin);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.imin);
            wassert(actual(vi.dmax) == td.imax);
            wassert(actual(vi.type) == Vartype::Integer);

            varinfo::set_crex(vi, WR_VAR(0, 0, 0), "Desc", "M", td.len, -3);
            wassert(actual(vi.len) == td.len);
            wassert(actual(vi.imin) == td.imin);
            wassert(actual(vi.imax) == td.imax);
            wassert(actual(vi.dmin) == td.imin * 1000.0);
            wassert(actual(vi.dmax) == td.imax * 1000.0);
            wassert(actual(vi.type) == Vartype::Decimal);

            varinfo::set_crex(vi, WR_VAR(0, 0, 0), "Desc", "M", td.len, 3);
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
            varinfo::set_bufr(info, WR_VAR(0, 1, 2), "test", "M", 31, 1));
        wassert(actual(e.what()) ==
                "001002 scaled value does not fit in a signed 32bit integer "
                "(31 bits with a base value of 1)");

        e = wassert_throws(error_consistency,
                           varinfo::set_bufr(info, WR_VAR(0, 1, 2), "test", "M",
                                             31, 0x7ffffffe));
        wassert(actual(e.what()) ==
                "001002 scaled value does not fit in a signed 32bit integer "
                "(31 bits with a base value of 2147483646)");
    });
}

} // namespace
