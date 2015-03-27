/*
 * wreport/tabledir-internals - tabledir implementation internals.
 *
 * This header is NOT part of the wreport API, and it must only be included in
 * wreport .cc files. It exists so that unit testing can access and test
 * the internal implementation.
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

#ifndef WREPORT_TABLEDIR_INTERNALS_H
#define WREPORT_TABLEDIR_INTERNALS_H

#include <string>
#include <ctime>
#include <dirent.h>

namespace wreport {
struct Vartable;
struct DTable;

namespace tabledir {

struct Table
{
    const Vartable* btable;
    std::string btable_id;
    std::string btable_pathname;
    const DTable* dtable;
    std::string dtable_id;
    std::string dtable_pathname;

    Table(const std::string& dirname, const std::string& filename);

    /// Load btable and dtable if they have not been loaded yet
    void load_if_needed();
};

/// Information about a version of a BUFR table
struct BufrTable : Table
{
    int centre;
    int subcentre;
    int master_table;
    int local_table;

    BufrTable(int centre, int subcentre, int master_table, int local_table,
              const std::string& dirname, const std::string& filename)
        : Table(dirname, filename),
          centre(centre), subcentre(subcentre),
          master_table(master_table), local_table(local_table) {}
};

/// Information about a version of a CREX table
struct CrexTable : Table
{
    int master_table_number;
    int edition;
    int table;

    CrexTable(int master_table_number, int edition, int table,
              const std::string& dirname, const std::string& filename)
        : Table(dirname, filename), master_table_number(master_table_number),
          edition(edition), table(table) {}
};


/// Access a table directory
struct DirReader
{
    const std::string& pathname;
    int fd;
    DIR* dir;
    struct dirent* cur_entry;
    time_t mtime;

    DirReader(const std::string& pathname);
    ~DirReader();

    /// Check if the directory exists
    bool exists() const { return fd != -1; }

    /// Start reading files
    void start_reading();

    /**
     * Move cur_entry to the next file.
     *
     * This needs to be called for the first file as well: the constructor will
     * not call readdir to point cur_entry to the first file.
     *
     * Returns false when the end of the directory is reached.
     */
    bool next_file();
};

/// Indexed version of a table directory
struct Dir
{
    std::string pathname;
    time_t mtime;
    std::vector<BufrTable> bufr_tables;
    std::vector<CrexTable> crex_tables;

    Dir(const std::string& pathname);
    ~Dir();

    /// Reread the directory contents if it has changed
    void refresh();

    /// Add a BUFR table entry
    void add_bufr_entry(int centre, int subcentre, int master_table, int local_table, const DirReader& reader);

    /// Add a CREX table entry
    void add_crex_entry(int master_table_number, int edition, int table, const DirReader& reader);
};

/// Query for a BUFR table
struct BufrQuery
{
    int centre;
    int subcentre;
    int master_table;
    int local_table;
    BufrTable* result;

    BufrQuery(int centre, int subcentre, int master_table, int local_table);
    void search(Dir& dir);
    bool is_better(const BufrTable& t);
};

/// Query for a CREX table
struct CrexQuery
{
    int master_table_number;
    int edition;
    int table;
    CrexTable* result;

    CrexQuery(int master_table_number, int edition, int table);
    void search(Dir& dir);
    bool is_better(const CrexTable& t);
};

}
}

#endif
