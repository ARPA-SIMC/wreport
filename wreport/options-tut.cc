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
#include <wreport/options.h>

using namespace wreport;
using namespace std;

namespace tut {

struct options_shar
{
    options_shar()
    {
    }

    ~options_shar()
    {
    }
};
TESTGRP(options);

template<> template<>
void to::test<1>()
{
    int a = 1;
    {
        options::LocalOverride<int> o(a, 2);
        ensure_equals(a, 2);
        ensure_equals(o.old_value, 1);
    }
    ensure_equals(a, 1);
}

}

// vim:set ts=4 sw=4:
