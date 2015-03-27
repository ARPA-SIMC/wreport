/*
 * Copyright (C) 2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#include "tabledir.h"
#include "tabledir-internals.h"
#include <set>

using namespace wibble::tests;
using namespace wreport;
using namespace std;

namespace tut {

struct tabledir_shar
{
    tabledir_shar()
    {
    }

    ~tabledir_shar()
    {
    }
};
TESTGRP(tabledir);

// Test DirReader
template<> template<>
void to::test<1>()
{
    using namespace wreport::tabledir;

    DirReader reader(".");
    wassert(actual(reader.exists()).istrue());
    wassert(actual(reader.mtime) > 0);

    set<string> fnames;
    reader.start_reading();
    while (reader.next_file())
        fnames.insert(reader.cur_entry->d_name);

    wassert(actual(fnames.size()) >= 2);
    wassert(actual(fnames.find(".") != fnames.end()).istrue());
    wassert(actual(fnames.find("..") != fnames.end()).istrue());
}

namespace {

struct BufrQueryTester
{
    tabledir::BufrQuery q;
    vector<tabledir::BufrTable> candidates;

    BufrQueryTester(int centre, int subcentre, int master_table, int local_table)
        : q(centre, subcentre, master_table, local_table)
    {
        // Reserve a large amount to avoid reallocations that invalidate q.result
        candidates.reserve(100);
    }

    bool try_entry(int centre, int subcentre, int master_table, int local_table)
    {
        using namespace wreport::tabledir;
        candidates.push_back(BufrTable(centre, subcentre, master_table, local_table, "a", "a"));
        bool res = q.is_better(candidates.back());
        if (res) q.result = &candidates.back();
        return res;
    }
};

struct CrexQueryTester
{
    tabledir::CrexQuery q;
    vector<tabledir::CrexTable> candidates;

    CrexQueryTester(int master_table_number, int edition, int table)
        : q(master_table_number, edition, table)
    {
        // Reserve a large amount to avoid reallocations that invalidate q.result
        candidates.reserve(100);
    }

    bool try_entry(int master_table_number, int edition, int table)
    {
        using namespace wreport::tabledir;
        candidates.push_back(CrexTable(master_table_number, edition, table, "a", "a"));
        bool res = q.is_better(candidates.back());
        if (res) q.result = &candidates.back();
        return res;
    }
};

}

// Test BufrQuery
template<> template<>
void to::test<2>()
{
    using namespace wreport::tabledir;

    BufrQueryTester qt(98, 1, 15, 3);
    wassert(actual(qt.try_entry( 0, 0, 14, 0)).isfalse());
    wassert(actual(qt.try_entry( 0, 0, 20, 0)).istrue());
    wassert(actual(qt.try_entry( 3, 1, 20, 3)).isfalse());
    wassert(actual(qt.try_entry( 3, 1, 19, 3)).istrue());
    wassert(actual(qt.try_entry( 3, 1, 20, 3)).isfalse());
    wassert(actual(qt.try_entry( 0, 0, 20, 0)).isfalse());
    wassert(actual(qt.try_entry( 0, 0, 19, 0)).istrue());
    wassert(actual(qt.try_entry( 0, 0, 16, 0)).istrue());
    wassert(actual(qt.try_entry( 0, 0, 15, 0)).istrue());
    wassert(actual(qt.try_entry( 0, 0, 15, 3)).isfalse());
    wassert(actual(qt.try_entry( 0, 1, 15, 0)).isfalse());
    wassert(actual(qt.try_entry( 0, 1, 15, 3)).isfalse());
    wassert(actual(qt.try_entry(98, 1, 16, 3)).isfalse());
    wassert(actual(qt.try_entry(98, 0, 15, 0)).istrue());
    wassert(actual(qt.try_entry(98, 1, 15, 0)).istrue());
    wassert(actual(qt.try_entry(98, 0, 15, 5)).istrue());
    wassert(actual(qt.try_entry(98, 0, 15, 7)).isfalse());
    wassert(actual(qt.try_entry(98, 0, 15, 3)).istrue());
    wassert(actual(qt.try_entry(98, 1, 15, 3)).istrue());
}

// Test CrexQuery
template<> template<>
void to::test<3>()
{
    using namespace wreport::tabledir;

    CrexQueryTester qt(0, 15, 3);
    wassert(actual(qt.try_entry(3, 15, 3)).isfalse());
    wassert(actual(qt.try_entry(0, 14, 0)).isfalse());
    wassert(actual(qt.try_entry(0, 20, 0)).istrue());
    wassert(actual(qt.try_entry(0, 21, 0)).isfalse());
    wassert(actual(qt.try_entry(0, 15, 0)).istrue());
    wassert(actual(qt.try_entry(0, 15, 0)).isfalse());
    wassert(actual(qt.try_entry(0, 15, 1)).istrue());
    wassert(actual(qt.try_entry(0, 15, 6)).istrue());
    wassert(actual(qt.try_entry(0, 15, 8)).isfalse());
    wassert(actual(qt.try_entry(0, 15, 5)).istrue());
    wassert(actual(qt.try_entry(0, 15, 3)).istrue());
}

// Test Tabledir
template<> template<>
void to::test<4>()
{
    // Get the default Tabledir
    Tabledir& td(Tabledir::get());

    const tabledir::BufrTable* bt;
    const tabledir::CrexTable* ct;

    bt = td.find_bufr(0, 0, 15, 0);
    wassert(actual(bt != 0).istrue());
    wassert(actual(bt->centre) == 0);
    wassert(actual(bt->subcentre) == 0);
    wassert(actual(bt->master_table) == 16);
    wassert(actual(bt->local_table) == 0);

    /// Find a CREX table
    ct = td.find_crex(0, 15, 0);
    wassert(actual(ct != 0).istrue());
    wassert(actual(ct->master_table_number) == 0);
    wassert(actual(ct->edition) == 16);
    wassert(actual(ct->table) == 0);
}

}
