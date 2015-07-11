#include "tabledir.h"
#include "error.h"
#include "vartable.h"
#include "dtable.h"
#include "notes.h"
#include "fs.h"
#include "config.h"
#include <map>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cerrno>

// Uncomment to trace approximate table matching
// #define TRACE_MATCH

using namespace std;

namespace wreport {
namespace tabledir {

Table::Table(const std::string& dirname, const std::string& filename)
    : btable(0), dtable(0)
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

void BufrTable::load_if_needed()
{
    if (!btable)
        btable = Vartable::load_bufr(btable_pathname);
    if (!dtable)
        dtable = DTable::load_bufr(dtable_pathname);
}

void CrexTable::load_if_needed()
{
    if (!btable)
        btable = Vartable::load_crex(btable_pathname);
    if (!dtable)
        dtable = DTable::load_crex(dtable_pathname);
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
    fs::Directory reader(pathname);
    if (!reader.exists()) return;
    struct stat st;
    reader.stat(st);
    if (mtime >= st.st_mtime) return;
    for (auto& e: reader)
    {
        switch (strlen(e.d_name))
        {
            case 11: // B000203.txt
            {
                int mt, ed, mtv;
                if (sscanf(e.d_name, "B%02d%02d%02d", &mt, &ed, &mtv) == 3)
                    crex_tables.push_back(CrexTable(CrexTableID(ed, 0xffff, 0xffff, mt, mtv, 0xff, 0xff), pathname, e.d_name));
                break;
            }
            case 20: // B000000000001100.txt
            {
                int ce, sc, mt, lt;
                if (sscanf(e.d_name, "B00000%03d%03d%02d%02d", &sc, &ce, &mt, &lt) == 4)
                    bufr_tables.push_back(BufrTable(BufrTableID(ce, sc, 0, mt, lt), pathname, e.d_name));
                break;
            }
            case 24: // B0000000000085014000.txt
            {
                int ce, sc, mt, lt, dummy;
                if (sscanf(e.d_name, "B00%03d%04d%04d%03d%03d", &dummy, &sc, &ce, &mt, &lt) == 5)
                    bufr_tables.push_back(BufrTable(BufrTableID(ce, sc, 0, mt, lt), pathname, e.d_name));
                break;
            }
        }
    }
    mtime = st.st_mtime;
}


BufrQuery::BufrQuery(const BufrTableID& id) : id(id), result(0)
{
}

void BufrQuery::search(Dir& dir)
{
    for (std::vector<BufrTable>::iterator i = dir.bufr_tables.begin(); i != dir.bufr_tables.end(); ++i)
        if (is_better(*i))
            result = &*i;
}

#ifdef TRACE_MATCH
static inline void trace_state_bufr(const BufrTableID& id, BufrTable* result, const BufrTable& t)
{
    fprintf(stderr, "want %03hu:%02hu %02hhu:%02hhu:%02hhu, have %03hu:%02hu %02hhu:%02hhu:%02hhu, try %03hu:%02hu %02hhu:%02hhu:%02hhu: ",
        id.originating_centre, id.originating_subcentre, id.master_table, id.master_table_version_number, id.master_table_version_number_local,
        result ? result->id.originating_centre : -1, result ? result->id.originating_subcentre : -1,
        result ? result->id.master_table : -1, result ? result->id.master_table_version_number : -1,
        result ? result->id.master_table_version_number_local : -1,
        t.id.originating_centre, t.id.originating_subcentre, t.id.master_table, t.id.master_table_version_number, t.id.master_table_version_number_local);
}
#define trace_state() trace_state_bufr(id, result, t)
#define accept(...) { trace_state(); fputs("ok: ", stderr); fprintf(stderr, __VA_ARGS__); putc('\n', stderr); return true; }
#define reject(...) { trace_state(); fputs("no: ", stderr); fprintf(stderr, __VA_ARGS__); putc('\n', stderr); return false; }
#else
#define accept(...) { return true; }
#define reject(...) { return false; }
#endif

bool BufrQuery::is_better(const BufrTable& t)
{
    if (t.id.master_table != id.master_table)
        reject("master table is different");

    if (t.id.master_table_version_number < id.master_table_version_number)
        reject("master table too old");

    // Any other table will do if we have no result so far
    if (!result)
        accept("better than nothing");

    if (result->id.master_table_version_number - id.master_table_version_number > t.id.master_table_version_number - id.master_table_version_number)
        accept("closer to the master_table we want");

    if (t.id.master_table_version_number != result->id.master_table_version_number)
        reject("not better than the master table that we have");

    if (result->id.originating_centre == id.originating_centre)
    {
        // If we already have the centre we want, only accept candidates
        // from that centre
        if (t.id.originating_centre != id.originating_centre)
            reject("not the same centre");

        // If both result and t have the same centre, keep going looking
        // for the rest of the details
    }
    else if (result->id.originating_centre == 0)
    {
        // If we approximated to WMO and we find the exact centre we want,
        // use it
        if (t.id.originating_centre == id.originating_centre)
            accept("exact match on the centre");

        // We have an approximate match, there is no point in looking for
        // the rest of the details
        reject("not an improvement on the centre");
    }
    else
    {
        if (t.id.originating_centre == id.originating_centre || t.id.originating_centre == 0)
            accept("better than a random centre");

        reject("not better than the current centre");
    }

    // Look for the closest match for the local table
    if (result->id.master_table_version_number_local < id.master_table_version_number_local)
    {
        if (t.id.master_table_version_number_local > result->id.master_table_version_number_local)
            accept("the current master table local version is lower than what we want: any higher one is better");
        if (t.id.master_table_version_number_local < result->id.master_table_version_number_local)
            reject("the master table local version is even lower than what we have");
    } else if (result->id.master_table_version_number_local >= id.master_table_version_number_local) {
        if (t.id.master_table_version_number_local < id.master_table_version_number_local)
            reject("not better than the current master table local version");
        if (result->id.master_table_version_number_local - id.master_table_version_number_local > t.id.master_table_version_number_local - id.master_table_version_number_local)
            accept("closer to the master table local version we want");
    }

    // If we don't have the same local_table as the current result, no
    // point in looking further
    if (result->id.master_table_version_number_local != t.id.master_table_version_number_local)
        reject("not better than the current local table");

    // Finally, try to match the exact subcentre
    if (t.id.originating_subcentre == id.originating_subcentre)
        accept("exact match on subcentre");

    reject("details are not better than what we have");
}


CrexQuery::CrexQuery(const CrexTableID& id) : id(id), result(0)
{
}

void CrexQuery::search(Dir& dir)
{
    for (std::vector<CrexTable>::iterator i = dir.crex_tables.begin(); i != dir.crex_tables.end(); ++i)
        if (is_better(*i))
            result = &*i;
}

#ifdef TRACE_MATCH
#undef trace_state
static inline void trace_state_crex(const CrexTableID& id, CrexTable* result, const CrexTable& t)
{
    fprintf(stderr, "want %02hu:%02hu %02hhu:%02hhu:%02hhu:%02hhu, have %02hu:%02hu %02hhu:%02hhu:%02hhu:%02hhu, try %02hu:%02hu %02hhu:%02hhu:%02hhu:%02hhu: ",
        id.originating_centre, id.originating_subcentre, id.master_table,
        id.master_table_version_number, id.master_table_version_number_local, id.master_table_version_number_bufr,
        result ? result->id.originating_centre : -1, result ? result->id.originating_subcentre : -1,
        result ? result->id.master_table : -1, result ? result->id.master_table_version_number : -1,
        result ? result->id.master_table_version_number_local : -1, result ? result->id.master_table_version_number_bufr : -1,
        t.id.originating_centre, t.id.originating_subcentre, t.id.master_table,
        t.id.master_table_version_number,
        t.id.master_table_version_number_local, t.id.master_table_version_number_bufr);
}
#define trace_state() trace_state_crex(id, result, t)
#endif

bool CrexQuery::is_better(const CrexTable& t)
{
    // Master table number must be the same
    if (t.id.edition_number != id.edition_number)
        reject("wrong edition number");

    // Master table number must be the same
    if (t.id.master_table != id.master_table)
        reject("wrong master table number");

    // Edition must be greater or equal to what we want
    if (t.id.master_table_version_number < id.master_table_version_number)
        reject("smaller than the master table version we want");

    // If we have no result so far, any random one will do
    if (!result)
        accept("better than nothing");

    if (t.id.master_table_version_number - id.master_table_version_number < result->id.master_table_version_number - id.master_table_version_number)
        accept("better than the master table version that we have");

    if (t.id.master_table_version_number != result->id.master_table_version_number)
        reject("not better than the master table version that we have");

    // Look for the closest match for the table
    if (result->id.master_table_version_number_local < id.master_table_version_number_local)
    {
        if (t.id.master_table_version_number_local > result->id.master_table_version_number_local)
            accept("the current master table local version is lower than what we want: any higher one is better");
        if (t.id.master_table_version_number_local < result->id.master_table_version_number_local)
            reject("the master table local version is even lower than what we have");
    } else if (result->id.master_table_version_number_local >= id.master_table_version_number_local) {
        if (t.id.master_table_version_number_local < id.master_table_version_number_local)
            reject("not better than the current master table local version");
        if (result->id.master_table_version_number_local - id.master_table_version_number_local > t.id.master_table_version_number_local - id.master_table_version_number_local)
            accept("closer to the master table local version we want");
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
    map<BufrTableID, const BufrTable*> bufr_cache;
    map<CrexTableID, const CrexTable*> crex_cache;

    Index(const vector<string>& dirs)
    {
        // Index the directories
        for (vector<string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i)
            this->dirs.push_back(Dir(*i));
    }

    const tabledir::BufrTable* find_bufr(const BufrTableID& id)
    {
        // First look it up in cache
        map<BufrTableID, const BufrTable*>::const_iterator i = bufr_cache.find(id);
        if (i != bufr_cache.end())
            return i->second;

        // If it is the first time this combination is requested, look for the best match
        BufrQuery query(id);
        for (vector<Dir>::iterator d = dirs.begin(); d != dirs.end(); ++d)
            query.search(*d);

        if (query.result)
        {
            query.result->load_if_needed();
            bufr_cache[id] = query.result;
            notes::logf("Matched table %s for ce %hu sc %hu mt %hhu mtv %hhu mtlv %hhu",
                    query.result->btable_id.c_str(),
                    id.originating_centre, id.originating_subcentre,
                    id.master_table, id.master_table_version_number, id.master_table_version_number_local);
        }

        return query.result;
    }

    const tabledir::CrexTable* find_crex(const CrexTableID& id)
    {
        // First look it up in cache
        map<CrexTableID, const CrexTable*>::const_iterator i = crex_cache.find(id);
        if (i != crex_cache.end())
            return i->second;

        // If it is the first time this combination is requested, look for the best match
        CrexQuery query(id);
        for (vector<Dir>::iterator d = dirs.begin(); d != dirs.end(); ++d)
            query.search(*d);

        if (query.result)
        {
            query.result->load_if_needed();
            crex_cache[id] = query.result;
            notes::logf("Matched table %s for mt %hhu mtv %hhu mtlv %hhu",
                    query.result->btable_id.c_str(),
                    id.master_table, id.master_table_version_number,
                    id.master_table_version_number_local);
        }

        return query.result;
    }

    const tabledir::BufrTable* find_bufr(const std::string& basename)
    {
        for (auto& d: dirs)
            for (auto& t: d.bufr_tables)
                if (t.btable_id == basename)
                {
                    t.load_if_needed();
                    return &t;
                }
        return nullptr;
    }

    const tabledir::CrexTable* find_crex(const std::string& basename)
    {
        for (auto& d: dirs)
            for (auto& t: d.crex_tables)
                if (t.btable_id == basename)
                {
                    t.load_if_needed();
                    return &t;
                }
        return nullptr;
    }
};


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

const tabledir::BufrTable* Tabledir::find_bufr(const BufrTableID& id)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_bufr(id);
}

const tabledir::CrexTable* Tabledir::find_crex(const CrexTableID& id)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_crex(id);
}

const tabledir::BufrTable* Tabledir::find_bufr(const std::string& basename)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_bufr(basename);
}

const tabledir::CrexTable* Tabledir::find_crex(const std::string& basename)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_crex(basename);
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
}
