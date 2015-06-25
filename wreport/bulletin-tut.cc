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
#include <wreport/bulletin.h>

using namespace wreport;
using namespace std;

namespace tut {

struct bulletin_shar
{
	bulletin_shar()
	{
	}

	~bulletin_shar()
	{
	}
};
TESTGRP(bulletin);

template<> template<>
void to::test<1>()
{
    unique_ptr<BufrBulletin> b(BufrBulletin::create());
    try {
        b->obtain_subset(0);
        ensure(false);
    } catch (error_consistency& e) {
        ensure_contains(e.what(), "B tables not loaded");
    }
	// b.subsets[0].store_variable_undef(WR_VAR(0, 1, 1));
#if 0
	// No more template parsing

	int c, sc, lc;

	parse_template("1.2.3", &c, &sc, &lc);
	ensure_equals(c, 1);
	ensure_equals(sc, 2);
	ensure_equals(lc, 3);

	parse_template("generic", &c, &sc, &lc);
	ensure_equals(c, 255);
	ensure_equals(sc, 255);
	ensure_equals(lc, 0);

	parse_template("synop", &c, &sc, &lc);
	ensure_equals(c, 0);
	ensure_equals(sc, 255);
	ensure_equals(lc, 1);

	parse_template("temp", &c, &sc, &lc);
	ensure_equals(c, 2);
	ensure_equals(sc, 255);
	ensure_equals(lc, 101);
#endif
}

}

// vim:set ts=4 sw=4:
