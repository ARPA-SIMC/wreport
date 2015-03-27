/*
 * wreport/tabledir - Access a BUFR/CREX table collection
 *
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

#ifndef WREPORT_TABLEDIR_H
#define WREPORT_TABLEDIR_H

#include <string>
#include <vector>

namespace wreport {

namespace tabledir {
struct Index;

struct BufrTable
{
    int centre;
    int subcentre;
    int master_table;
    int local_table;
    std::string btable_pathname;
    std::string dtable_pathname;

    BufrTable() {}
    BufrTable(int centre, int subcentre, int master_table, int local_table,
              const std::string& btable_pathname,
              const std::string& dtable_pathname)
        : centre(centre), subcentre(subcentre), master_table(master_table),
          local_table(local_table), btable_pathname(btable_pathname),
          dtable_pathname(dtable_pathname) {}
};

struct CrexTable
{
    int master_table_number;
    int edition;
    int table;
    std::string btable_pathname;
    std::string dtable_pathname;

    CrexTable() {}
    CrexTable(int master_table_number, int edition, int table,
              const std::string& btable_pathname,
              const std::string& dtable_pathname)
        : master_table_number(master_table_number), edition(edition),
          table(table), btable_pathname(btable_pathname),
          dtable_pathname(dtable_pathname) {}
};

}

class Tabledir
{
protected:
    std::vector<std::string> dirs;
    tabledir::Index* index;

public:
    Tabledir();
    ~Tabledir();

    /**
     * Add the default directories according to compile-time and environment
     * variables.
     */
    void add_default_directories();

    /// Add a table directory to this collection
    void add_directory(const std::string& dir);

    /// Find a BUFR table
    const tabledir::BufrTable* find_bufr(int centre, int subcentre, int master_table, int local_table);

    /// Find a CREX table
    const tabledir::CrexTable* find_crex(int master_table_number, int edition, int table);

    /// Get the default tabledir instance
    static Tabledir& get();
};

}

#endif
