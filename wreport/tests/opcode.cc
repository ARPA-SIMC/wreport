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
#include <wreport/opcode.h>
#include <wreport/dtable.h>

using namespace wreport;
using namespace std;

namespace tut {

struct opcode_shar
{
    opcode_shar()
    {
    }

    ~opcode_shar()
    {
    }
};
TESTGRP(opcode);

// Test simple access
template<> template<>
void to::test<1>()
{
    vector<Varcode> ch0_vec;
    ch0_vec.push_back('A');
    ch0_vec.push_back('n');
    ch0_vec.push_back('t');

    Opcodes ch0(ch0_vec);
    ensure_equals(ch0.head(), 'A');
    ensure_equals(ch0.next().head(), 'n');
    ensure_equals(ch0.next().next().head(), 't');
    ensure_equals(ch0.next().next().next().head(), 0);

    ensure_equals(ch0[1], 'n');
    ensure_equals(ch0[10], 0);
}

namespace {

struct ExploreCounter : public opcode::Explorer
{
    unsigned count_b;
    unsigned count_r_plain;
    unsigned count_r_delayed;
    unsigned count_c;
    unsigned count_d;

    ExploreCounter()
        : count_b(0), count_r_plain(0), count_r_delayed(0), count_c(0), count_d(0) {}

    void b_variable(Varcode code) { ++count_b; }
    void c_modifier(Varcode code) { ++count_c; }
    void r_replication_begin(Varcode code, Varcode delayed_code)
    {
        if (delayed_code)
            ++count_r_delayed;
        else
            ++count_r_plain;
    }
    void d_group_begin(Varcode code) { ++count_d; }
};

}

// Test explorer
template<> template<>
void to::test<2>()
{
    const DTable* table = DTable::get("D0000000000000014000");
    Opcodes ops = table->query(WR_VAR(3, 0, 10));
    ensure_equals(ops.size(), 4);

    ExploreCounter c;
    ops.explore(c, *table);

    ensure_equals(c.count_b, 4u);
    ensure_equals(c.count_c, 0u);
    ensure_equals(c.count_r_plain, 0u);
    ensure_equals(c.count_r_delayed, 1u);
    ensure_equals(c.count_d, 1u);
}

}

// vim:set ts=4 sw=4:
