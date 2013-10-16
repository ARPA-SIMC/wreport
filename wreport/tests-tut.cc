/*
 * Copyright (C) 2013  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include <wreport/tests.h>
#include <wreport/var.h>
#include <wreport/vartable.h>

using namespace wreport;
using namespace std;
using namespace wibble::tests;

namespace tut {

struct tests_shar
{
};
TESTGRP(tests);

// Test variable comparisons
template<> template<>
void to::test<1>()
{
    const Vartable* table = Vartable::get("B0000000000000014000");
    Var tempundef(table->query(WR_VAR(0, 12, 101)));
    Var temp12(table->query(WR_VAR(0, 12, 101)), 12.5);
    Var temp13(table->query(WR_VAR(0, 12, 101)), 13.5);
    wassert(actual(tempundef) == tempundef);
    wassert(actual(tempundef) != temp12);
    wassert(actual(tempundef) != temp13);
    wassert(actual(temp12) != tempundef);
    wassert(actual(temp12) == temp12);
    wassert(actual(temp12) != temp13);
    wassert(actual(temp13) != tempundef);
    wassert(actual(temp13) != temp12);
    wassert(actual(temp13) == temp13);

    Var tempundefa1(tempundef);
    Var tempundefa2(tempundef);
    tempundefa1.seta(auto_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 75)));
    tempundefa2.seta(auto_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 50)));

    wassert(actual(tempundef) != tempundefa1);
    wassert(actual(tempundef) != tempundefa2);
    wassert(actual(tempundefa1) == tempundefa1);
    wassert(actual(tempundefa1) != tempundefa2);
    wassert(actual(tempundefa2) != tempundefa1);
    wassert(actual(tempundefa2) == tempundefa2);

    Var temp12a1(temp12);
    Var temp12a2(temp12);
    temp12a1.seta(auto_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 75)));
    temp12a2.seta(auto_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 50)));

    wassert(actual(temp12) != temp12a1);
    wassert(actual(temp12) != temp12a2);
    wassert(actual(temp12a1) == temp12a1);
    wassert(actual(temp12a1) != temp12a2);
    wassert(actual(temp12a2) != temp12a1);
    wassert(actual(temp12a2) == temp12a2);

    Var tempa1(temp12);
    Var tempa2(temp12);
    tempa1.seta(auto_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 1)));
    tempa2.seta(auto_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  9)), 1)));
    wassert(actual(tempa1) == tempa1);
    wassert(actual(tempa1) != tempa2);
    wassert(actual(tempa2) != tempa1);
    wassert(actual(tempa2) == tempa2);
}

}

/* vim:set ts=4 sw=4: */
