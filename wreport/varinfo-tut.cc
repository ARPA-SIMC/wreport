/*
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#include <test-utils-wreport.h>
#include <wreport/varinfo.h>
#include <cstring>

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    // Test varcode encoding functions
    Test("encode", [](Fixture& f) {
        ensure_equals(WR_VAR(0, 0, 0), 0);
        ensure_equals(WR_VAR(0, 0, 255), 0xff);
        ensure_equals(WR_VAR(0, 1, 0), 0x100);
        ensure_equals(WR_VAR(0, 63, 0), 0x3f00);
        ensure_equals(WR_VAR(0, 63, 255), 0x3fff);
        ensure_equals(WR_VAR(1, 0, 0), 0x4000);
        ensure_equals(WR_VAR(2, 0, 255), 0x80ff);
        ensure_equals(WR_VAR(3, 1, 0), 0xc100);
        ensure_equals(WR_VAR(1, 63, 0), 0x7f00);
        ensure_equals(WR_VAR(2, 63, 255), 0xbfff);
        ensure_equals(WR_VAR(3, 63, 255), 0xffff);

        ensure_equals(WR_STRING_TO_VAR("12345"), WR_VAR(0, 12, 345));
        ensure_equals(WR_STRING_TO_VAR("00345"), WR_VAR(0, 0, 345));
        ensure_equals(WR_STRING_TO_VAR("00000"), WR_VAR(0, 0, 0));
        ensure_equals(WR_STRING_TO_VAR("63255"), WR_VAR(0, 63, 255));

        ensure_equals(descriptor_code("B12345"), WR_VAR(0, 12, 345));
        ensure_equals(descriptor_code("R00345"), WR_VAR(1, 0, 345));
        ensure_equals(descriptor_code("C00000"), WR_VAR(2, 0, 0));
        ensure_equals(descriptor_code("D63255"), WR_VAR(3, 63, 255));
        ensure_equals(descriptor_code("012345"), WR_VAR(0, 12, 345));
        ensure_equals(descriptor_code("100345"), WR_VAR(1, 0, 345));
        ensure_equals(descriptor_code("200000"), WR_VAR(2, 0, 0));
        ensure_equals(descriptor_code("363255"), WR_VAR(3, 63, 255));

        ensure_equals(varcode_format(WR_VAR(0, 1, 2)), "B01002");
        ensure_equals(varcode_format(WR_VAR(1, 1, 2)), "R01002");
        ensure_equals(varcode_format(WR_VAR(2, 1, 2)), "C01002");
        ensure_equals(varcode_format(WR_VAR(3, 1, 2)), "D01002");
        ensure_equals(varcode_format(WR_VAR(4000, 1, 2)), "B01002");
    }),
    Test("set", [](Fixture& f) {
        // Test varinfo set
        _Varinfo info;
        info.set_string(WR_VAR(2, 20, 0), "test", 10);
        wassert(actual(info.var) == WR_VAR(2, 20, 0));
        wassert(actual(string(info.desc)) == "test");
        wassert(actual(string(info.unit)) == "CCITTIA5");
        wassert(actual(info.len) == 10);
        wassert(actual(info.bit_len) == 80);
    }),
    Test("bounds", [](Fixture& f) {
        // Test the calculation of bounds
        _Varinfo info;
        info.set_bufr(WR_VAR(0, 15, 194),        // Var
              "[SIM] O3 Concentration", // Desc
              "KG/M**3",            // Unit
              10, 5, 10, 0, 17);     // Scale, len, bit_scale, bit_ref, bit_len
        info.compute_range();
        wassert(actual(info.dmin) == 0);
        wassert(actual(info.dmax) == 9.9998e-06);
        wassert(actual(info.is_string()).isfalse());
    }),
    Test("encode_doubles", [](Fixture& f) {
        // Test encoding doubles to ints
        _Varinfo info;
        info.set_bufr(WR_VAR(0, 6, 2),      // Var
              "LONGITUDE (COARSE ACCURACY)",// Desc
              "DEGREE",         // Unit
              2, 5, 2, -18000, 16);     // Scale, len, bit_scale, bit_ref, bit_len
        info.bit_scale = 2;
        info.compute_range();
        wassert(actual(info.dmin) == -180);
        wassert(actual(info.dmax) == 475.34);
        wassert(actual(info.is_string()).isfalse());
        // ensure_equals(info->decode_int(16755), -12.45);
        wassert(actual(info.decode_binary(16755)) == -12.45);
    }),
    Test("range_crex", [](Fixture& f) {
        // Test range checking for CREX
        _Varinfo info;
        // 012003 DEW-POINT TEMPERATURE in B000103.txt
        info.set_crex(WR_VAR(0, 12, 3),
                 "DEW-POINT TEMPERATURE", "C",
                 1, 3);
        wassert(actual(info.dmin) == -99.9);
        wassert(actual(info.dmax) == 99.8);
    }),
    Test("set_binary", [](Fixture& f) {
        // Test binary varinfos
        _Varinfo info;
        info.set_binary(WR_VAR(0, 0, 0), "TEST", 6);
        wassert(actual(info.len) == 1);
        wassert(actual(info.bit_len) == 6);
        wassert(actual(info.imin) == 0);
        wassert(actual(info.imax) == 0);
        wassert(actual(info.dmin) == 0);
        wassert(actual(info.dmax) == 0);
    }),
};

test_group newtg("varinfo", tests);

}
