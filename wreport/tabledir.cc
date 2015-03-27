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
#include "tabledir.h"
#include "tabledir-internals.h"
#include "error.h"
#include "config.h"
#include <map>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Uncomment to trace approximate table matching
// #define TRACE_MATCH

using namespace std;

namespace wreport {
namespace tabledir {

Table::Table(const std::string& dirname, const std::string& filename)
{
    // Build IDs to look up pre-built tables
    size_t extpos = filename.rfind('.');
    if (extpos == string::npos)
    {
        btable_id = filename;
        dtable_id = filename;
    } else {
        btable_id = filename.substr(0, extpos);
        dtable_id = btable_id;
    }
    btable_id[0] = 'B';
    dtable_id[0] = 'D';

    btable_pathname = dirname + "/B" + filename.substr(1);
    dtable_pathname = dirname + "/D" + filename.substr(1);
}


DirReader::DirReader(const std::string& pathname)
    : pathname(pathname), fd(-1), dir(NULL), cur_entry(NULL), mtime(0)
{
    fd = open(pathname.c_str(), O_DIRECTORY | O_PATH);
    if (fd == -1)
    {
       if (errno != ENOENT)
           error_system::throwf("cannot open directory %s", pathname.c_str());
       else
           return;
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
        error_system::throwf("stat failed on directory %s", pathname.c_str());
    mtime = st.st_mtime;
}

DirReader::~DirReader()
{
    if (cur_entry != NULL)
        free(cur_entry);
    if (dir != NULL)
        closedir(dir);
    if (fd != -1)
        close(fd);
}

void DirReader::start_reading()
{
    int fd1 = openat(fd, ".", O_DIRECTORY);
    if (fd1 == -1)
        error_system::throwf("cannot open directory %s", pathname.c_str());

    dir = fdopendir(fd1);
    if (dir == NULL)
        error_system::throwf("opendir failed on directory %s", pathname.c_str());

    long name_max = fpathconf(fd, _PC_NAME_MAX);
    if (name_max == -1) // Limit not defined, or error: take a guess
        name_max = 255;
    size_t len = offsetof(dirent, d_name) + name_max + 1;
    cur_entry = (struct dirent*)malloc(len);
    if (cur_entry == NULL)
        throw error_alloc("cannot allocate space for a dirent structure");
}

bool DirReader::next_file()
{
    struct dirent* result;
    if (readdir_r(dir, cur_entry, &result) != 0)
        error_system::throwf("readdir_r failed on directory %s", pathname.c_str());

    if (result == NULL)
        return false;

    return true;
}

Dir::Dir(const std::string& pathname)
    : pathname(pathname), mtime(0)
{
    refresh();
}

Dir::~Dir()
{
}

// Reread the directory contents if it has changed
void Dir::refresh()
{
    DirReader reader(pathname);
    if (!reader.exists()) return;
    if (mtime >= reader.mtime) return;
    reader.start_reading();
    while (reader.next_file())
    {
        switch (strlen(reader.cur_entry->d_name))
        {
            case 11: // B000203.txt
            {
                int mtn, ed, ta;
                if (sscanf(reader.cur_entry->d_name, "B%02d%02d%02d", &mtn, &ed, &ta) == 3)
                    add_crex_entry(mtn, ed, ta, reader);
                break;
            }
            case 20: // B000000000001100.txt
            {
                int ce, sc, mt, lt;
                if (sscanf(reader.cur_entry->d_name, "B00000%03d%03d%02d%02d", &sc, &ce, &mt, &lt) == 4)
                    add_bufr_entry(ce, sc, mt, lt, reader);
                break;
            }
            case 24: // B0000000000085014000.txt
            {
                int ce, sc, mt, lt, dummy;
                if (sscanf(reader.cur_entry->d_name, "B00%03d%04d%04d%03d%03d", &dummy, &sc, &ce, &mt, &lt) == 5)
                    add_bufr_entry(ce, sc, mt, lt, reader);
                break;
            }
        }
    }
    mtime = reader.mtime;
}

void Dir::add_bufr_entry(int centre, int subcentre, int master_table, int local_table, const DirReader& reader)
{
    bufr_tables.push_back(BufrTable(centre, subcentre, master_table, local_table, reader.pathname, reader.cur_entry->d_name));
}

void Dir::add_crex_entry(int master_table_number, int edition, int table, const DirReader& reader)
{
    crex_tables.push_back(CrexTable(master_table_number, edition, table, reader.pathname, reader.cur_entry->d_name));
}


BufrQuery::BufrQuery(int centre, int subcentre, int master_table, int local_table)
    : centre(centre), subcentre(subcentre), master_table(master_table), local_table(local_table), result(0)
{
}

void BufrQuery::search(const Dir& dir)
{
    for (std::vector<BufrTable>::const_iterator i = dir.bufr_tables.begin(); i != dir.bufr_tables.end(); ++i)
        if (is_better(*i))
            result = &*i;
}

#ifdef TRACE_MATCH
#define trace_state() \
    fprintf(stderr, "want %2d %2d %2d %2d, have %2d %2d %2d %2d, try %2d %2d %2d %2d: ", \
        centre, subcentre, master_table, local_table, \
        result ? result->centre : -1, result ? result->subcentre : -1, result ? result->master_table : -1, result ? result->local_table : -1, \
        t.centre, t.subcentre, t.master_table, t.local_table)
#define accept(...) { trace_state(); fputs("ok: ", stderr); fprintf(stderr, __VA_ARGS__); putc('\n', stderr); return true; }
#define reject(...) { trace_state(); fputs("no: ", stderr); fprintf(stderr, __VA_ARGS__); putc('\n', stderr); return false; }
#else
#define accept(...) { return true; }
#define reject(...) { return false; }
#endif

bool BufrQuery::is_better(const BufrTable& t)
{
    if (t.master_table < master_table)
        reject("master table too old");

    // Any other table will do if we have no result so far
    if (!result)
        accept("better than nothing");

    if (result->master_table - master_table > t.master_table - master_table)
        accept("closer to the master_table we want");

    if (t.master_table != result->master_table)
        reject("not better than the master table that we have");

    if (result->centre == centre)
    {
        // If we already have the centre we want, only accept candidates
        // from that centre
        if (t.centre != centre)
            reject("not the same centre");

        // If both result and t have the same centre, keep going looking
        // for the rest of the details
    }
    else if (result->centre == 0)
    {
        // If we approximated to WMO and we find the exact centre we want,
        // use it
        if (t.centre == centre)
            accept("exact match on the centre");

        // We have an approximate match, there is no point in looking for
        // the rest of the details
        reject("not an improvement on the centre");
    }
    else
    {
        if (t.centre == centre || t.centre == 0)
            accept("better than a random centre");

        reject("not better than the current centre");
    }

    // Look for the closest match for the local table
    if (result->local_table < local_table)
    {
        if (t.local_table > result->local_table)
            accept("the current local table is lower than what we want: any higher one is better");
        if (t.local_table < result->local_table)
            reject("the local table is even lower than what we have");
    } else if (result->local_table >= local_table) {
        if (t.local_table < local_table)
            reject("not better than the current local_table");
        if (result->local_table - local_table > t.local_table - local_table)
            accept("closer to the local_table we want");
    }

    // If we don't have the same local_table as the current result, no
    // point in looking further
    if (result->local_table != t.local_table)
        reject("not better than the current local table");

    // Finally, try to match the exact subcentre
    if (t.subcentre == subcentre)
        accept("exact match on subcentre");

    reject("details are not better than what we have");
}


CrexQuery::CrexQuery(int master_table_number, int edition, int table)
    : master_table_number(master_table_number), edition(edition), table(table), result(0)
{
}

void CrexQuery::search(const Dir& dir)
{
    for (std::vector<CrexTable>::const_iterator i = dir.crex_tables.begin(); i != dir.crex_tables.end(); ++i)
        if (is_better(*i))
            result = &*i;
}

#undef trace_state
#define trace_state() \
    fprintf(stderr, "want %2d %2d %2d, have %2d %2d %2d, try %2d %2d %2d: ", \
        master_table_number, edition, table, \
        result ? result->master_table_number : -1, result ? result->edition : -1, result ? result->table : -1, \
        t.master_table_number, t.edition, t.table)

bool CrexQuery::is_better(const CrexTable& t)
{
    // Master table number must be the same
    if (t.master_table_number != master_table_number)
        reject("wrong master_table_number");

    // Edition must be greater or equal to what we want
    if (t.edition < edition)
        reject("smaller than the edition we want");

    // If we have no result so far, any random one will do
    if (!result)
        accept("better than nothing");

    if (t.edition - edition < result->edition - edition)
        accept("better than the edition we have");

    if (t.edition != result->edition)
        reject("not better than the edition that we have");

    // Look for the closest match for the table
    if (result->table < table)
    {
        if (t.table > result->table)
            accept("the current table is lower than what we want: any higher one is better");
        if (t.table < result->table)
            reject("the table is even lower than what we have");
    } else if (result->table >= table) {
        if (t.table < table)
            reject("not better than the current table");
        if (result->table - table > t.table - table)
            accept("closer to the table we want");
    }

    reject("details are not better than what we have");
}

// Pack a BUFR request in a single unsigned
static inline unsigned pack_bufr(int centre, int subcentre, int master_table, int local_table)
{
    return (centre << 24) | (subcentre << 16) | (master_table << 8) | local_table;
}
// Pack a CREX request in a single unsigned
static inline unsigned pack_crex(int master_table_number, int edition, int table)
{
    return (master_table_number << 16) | (edition << 8) | table;
}

struct Index
{
    vector<Dir> dirs;
    map<unsigned, const BufrTable*> bufr_cache;
    map<unsigned, const CrexTable*> crex_cache;

    Index(const vector<string>& dirs)
    {
        // Index the directories
        for (vector<string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i)
            this->dirs.push_back(Dir(*i));
    }

    const tabledir::BufrTable* find_bufr(int centre, int subcentre, int master_table, int local_table)
    {
        // First look it up in cache
        unsigned cache_key = pack_bufr(centre, subcentre, master_table, local_table);
        map<unsigned, const BufrTable*>::const_iterator i = bufr_cache.find(cache_key);
        if (i != bufr_cache.end())
            return i->second;

        // If it is the first time this combination is requested, look for the best match
        BufrQuery query(centre, subcentre, master_table, local_table);
        for (vector<Dir>::const_iterator d = dirs.begin(); d != dirs.end(); ++d)
            query.search(*d);

        if (query.result)
            bufr_cache[cache_key] = query.result;

        return query.result;
    }

    const tabledir::CrexTable* find_crex(int master_table_number, int edition, int table)
    {
        // First look it up in cache
        unsigned cache_key = pack_crex(master_table_number, edition, table);
        map<unsigned, const CrexTable*>::const_iterator i = crex_cache.find(cache_key);
        if (i != crex_cache.end())
            return i->second;

        // If it is the first time this combination is requested, look for the best match
        CrexQuery query(master_table_number, edition, table);
        for (vector<Dir>::const_iterator d = dirs.begin(); d != dirs.end(); ++d)
            query.search(*d);

        if (query.result)
            crex_cache[cache_key] = query.result;

        return query.result;
    }
};

}


Tabledir::Tabledir()
    : index(0)
{
}

Tabledir::~Tabledir()
{
    delete index;
}

void Tabledir::add_default_directories()
{
    if (char* env = getenv("WREPORT_EXTRA_TABLES"))
        add_directory(env);
    if (char* env = getenv("WREPORT_TABLES"))
        add_directory(env);
    add_directory(TABLE_DIR);
}

void Tabledir::add_directory(const std::string& dir)
{
    // Strip trailing /
    string clean_dir(dir);
    while (!clean_dir.empty() && clean_dir[clean_dir.size() - 1] == '/')
        clean_dir.resize(clean_dir.size() - 1);
    if (clean_dir.empty())
        clean_dir = "/";

    // Do not add a duplicate directory
    for (vector<string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i)
        if (*i == clean_dir)
            return;
    dirs.push_back(clean_dir);

    // Force a rebuild of the index
    delete index;
    index = 0;
}

const tabledir::BufrTable* Tabledir::find_bufr(int centre, int subcentre, int master_table, int local_table)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_bufr(centre, subcentre, master_table, local_table);
}

const tabledir::CrexTable* Tabledir::find_crex(int master_table_number, int edition, int table)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_crex(master_table_number, edition, table);
}

Tabledir& Tabledir::get()
{
    static Tabledir* default_tabledir = 0;
    if (!default_tabledir)
    {
        default_tabledir = new Tabledir();
        default_tabledir->add_default_directories();
    }
    return *default_tabledir;
}


}
