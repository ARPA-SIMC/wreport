/*
 * Copyright (C) 2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include <tests.h>
#include "conv.h"
#include "codetables.h"

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
    Test("units", [](Fixture& f) {
        ensure_equals(convert_units("K", "K", 273.15), 273.15);
        ensure_similar(convert_units("C", "K", 0.7), 273.85, 0.0001);
        //ensure(convert_units_allowed("C", "K"));
        //ensure(not convert_units_allowed("C", "M"));
        //ensure_equals(convert_units_get_mul("C", "K"), 1.0);
        ensure_equals(convert_units("RATIO", "%", 1.0), 100.0);
        ensure_equals(convert_units("%", "RATIO", 100.0), 1.0);

        ensure_equals(convert_units("ms/cm", "S/M", 1.0), 0.1);
        ensure_equals(convert_units("S/M", "ms/cm", 0.1), 1.0);

        ensure_similar(convert_units("ug/m**3", "KG/M**3", 45), 4.5e-08, 1e-10);
        ensure_similar(convert_units("KG/M**3", "ug/m**3", 0.000000051), 51.0, 1e-10);

        ensure_equals(convert_units("S", "MINUTE", 60.0), 1.0);
        ensure_equals(convert_units("MINUTE", "S", 1.0), 60.0);

        ensure_equals(convert_units("J/M**2", "MJ/M**2", 1), 0.000001);
        ensure_equals(convert_units("MJ/M**2", "J/M**2", 1), 1000000);

        ensure_equals(convert_units("octants", "DEGREE TRUE", 0), 0);
        ensure_equals(convert_units("octants", "DEGREE TRUE", 1), 45);
        ensure_equals(convert_units("DEGREE TRUE", "octants", 45), 1);

        ensure_equals(convert_units("FLAGTABLE", "FLAG TABLE 2002", 1), 1);
        ensure_equals(convert_units("CODETABLE", "CODE TABLE 2001", 1), 1);
    }),
    Test("vss", [](Fixture& f) {
        // Vertical sounding significance conversion functions
        ensure_equals(convert_BUFR08001_to_BUFR08042(BUFR08001::ALL_MISSING), BUFR08042::ALL_MISSING);
        ensure_equals(convert_BUFR08001_to_BUFR08042(BUFR08001::TROPO), BUFR08042::TROPO);
        ensure_equals(convert_BUFR08001_to_BUFR08042(BUFR08001::SIGTH), BUFR08042::SIGTEMP | BUFR08042::SIGHUM);

        ensure_equals(convert_BUFR08042_to_BUFR08001(BUFR08042::ALL_MISSING), BUFR08001::ALL_MISSING);
        ensure_equals(convert_BUFR08042_to_BUFR08001(BUFR08042::TROPO), BUFR08001::TROPO);
        ensure_equals(convert_BUFR08042_to_BUFR08001(BUFR08042::SIGTEMP), BUFR08001::SIGTH);
        ensure_equals(convert_BUFR08042_to_BUFR08001(BUFR08042::SIGHUM), BUFR08001::SIGTH);
    }),
    Test("octants", [](Fixture& f) {
        ensure_equals(convert_octants_to_degrees(0),   0.0);
        ensure_equals(convert_octants_to_degrees(1),  45.0);
        ensure_equals(convert_octants_to_degrees(2),  90.0);
        ensure_equals(convert_octants_to_degrees(3), 135.0);
        ensure_equals(convert_octants_to_degrees(4), 180.0);
        ensure_equals(convert_octants_to_degrees(5), 225.0);
        ensure_equals(convert_octants_to_degrees(6), 270.0);
        ensure_equals(convert_octants_to_degrees(7), 315.0);
        ensure_equals(convert_octants_to_degrees(8), 360.0);

        ensure_equals(convert_degrees_to_octants(  0.0),  0);
        ensure_equals(convert_degrees_to_octants( 10.0),  8);
        ensure_equals(convert_degrees_to_octants( 22.5),  8);
        ensure_equals(convert_degrees_to_octants( 47.0),  1);
        ensure_equals(convert_degrees_to_octants(360.0),  8);
    }),
};

test_group newtg("conv", tests);

}
