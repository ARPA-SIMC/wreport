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
    dtable_id[0] = 'D';

    btable_pathname = dirname + "/" + filename;
    dtable_pathname = dirname + "/D" + filename.substr(1);
}


Dir::Dir(const std::string& pathname)
    : pathname(pathname), mtime(0)
{
    refresh();
}

Dir::~Dir()
{
    for (auto t: tables)
        delete t;
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
        size_t name_len = strlen(e.d_name);

        // Look for a .txt extension
        if (name_len < 5) continue;
        if (strcmp(e.d_name + name_len - 4, ".txt") != 0) continue;

        switch (e.d_name[0])
        {
            case 'B':
                switch (strlen(e.d_name))
                {
                    case 11: // B000203.txt
                    {
                        int mt, ed, mtv;
                        if (sscanf(e.d_name, "B%02d%02d%02d", &mt, &ed, &mtv) == 3)
                            tables.push_back(new CrexTable(CrexTableID(ed, 0xffff, 0xffff, mt, mtv, 0xff, mtv), pathname, e.d_name));
                        break;
                    }
                    case 20: // B000000000001100.txt
                    {
                        int ce, sc, mt, lt;
                        if (sscanf(e.d_name, "B00000%03d%03d%02d%02d", &sc, &ce, &mt, &lt) == 4)
                            tables.push_back(new BufrTable(BufrTableID(ce, sc, 0, mt, lt), pathname, e.d_name));
                        break;
                    }
                    case 24: // B0000000000085014000.txt
                    {
                        int ce, sc, mt, lt, dummy;
                        if (sscanf(e.d_name, "B00%03d%04d%04d%03d%03d", &dummy, &sc, &ce, &mt, &lt) == 5)
                            tables.push_back(new BufrTable(BufrTableID(ce, sc, 0, mt, lt), pathname, e.d_name));
                        break;
                    }
                }
                break;
            case 'D':
                // Skip D tables
                break;
            default:
                // Add all the rest as raw tables, that will be skipped by BUFR
                // and CREX searches but that are still reachable by basename
                tables.push_back(new Table(pathname, e.d_name));
                break;
        }
    }
    mtime = st.st_mtime;
}


void Query::search(Dir& dir)
{
    for (const auto& t : dir.tables)
    {
        if (BufrTable* cur = dynamic_cast<BufrTable*>(t))
        {
            if (!is_acceptable(cur->id)) continue;
            if (!bufr_best)
                bufr_best = cur;
            else
                bufr_best = choose_best(*bufr_best, *cur);
        }
        else if (CrexTable* cur = dynamic_cast<CrexTable*>(t))
        {
            if (!is_acceptable(cur->id)) continue;
            if (!crex_best)
                crex_best = cur;
            else
                crex_best = choose_best(*crex_best, *cur);
        }
        // Ignore other kinds of tables
    }
}

Table* Query::result() const
{
    if (!bufr_best)
        if (!crex_best)
            return nullptr;
        else
            return crex_best;
    else
        if (!crex_best)
            return bufr_best;
        else
            return choose_best(*bufr_best, *crex_best);
}


BufrQuery::BufrQuery(const BufrTableID& id) : id(id)
{
}

bool BufrQuery::is_acceptable(const BufrTableID& id) const
{
    return this->id.is_acceptable_replacement(id);
}

bool BufrQuery::is_acceptable(const CrexTableID& id) const
{
    return this->id.is_acceptable_replacement(id);
}

BufrTable* BufrQuery::choose_best(BufrTable& first, BufrTable& second) const
{
    int cmp = id.closest_match(first.id, second.id);
    return cmp <= 0 ? &first : &second;
}

CrexTable* BufrQuery::choose_best(CrexTable& first, CrexTable& second) const
{
    return nullptr;
}

Table* BufrQuery::choose_best(BufrTable& first, CrexTable& second) const
{
    return &first;
}


CrexQuery::CrexQuery(const CrexTableID& id) : id(id)
{
}

bool CrexQuery::is_acceptable(const BufrTableID& id) const
{
    return this->id.is_acceptable_replacement(id);
}

bool CrexQuery::is_acceptable(const CrexTableID& id) const
{
    return this->id.is_acceptable_replacement(id);
}

BufrTable* CrexQuery::choose_best(BufrTable& first, BufrTable& second) const
{
    int cmp = id.closest_match(first.id, second.id);
    return cmp <= 0 ? &first : &second;
}

CrexTable* CrexQuery::choose_best(CrexTable& first, CrexTable& second) const
{
    int cmp = id.closest_match(first.id, second.id);
    return cmp <= 0 ? &first : &second;
}

Table* CrexQuery::choose_best(BufrTable& first, CrexTable& second) const
{
    int cmp = id.closest_match(first.id, second.id);
    if (cmp <= 0)
        return &first;
    else
        return &second;
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
    map<BufrTableID, const Table*> bufr_cache;
    map<CrexTableID, const Table*> crex_cache;

    Index(const vector<string>& dirs)
    {
        // Index the directories
        for (vector<string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i)
            this->dirs.push_back(Dir(*i));
    }

    const tabledir::Table* find_bufr(const BufrTableID& id)
    {
        // First look it up in cache
        const auto i = bufr_cache.find(id);
        if (i != bufr_cache.end())
            return i->second;

        // If it is the first time this combination is requested, look for the best match
        BufrQuery query(id);
        for (vector<Dir>::iterator d = dirs.begin(); d != dirs.end(); ++d)
            query.search(*d);

        if (auto result = query.result())
        {
            bufr_cache[id] = result;
            notes::logf("Matched table %s for ce %hu sc %hu mt %hhu mtv %hhu mtlv %hhu",
                    result->btable_id.c_str(),
                    id.originating_centre, id.originating_subcentre,
                    id.master_table, id.master_table_version_number, id.master_table_version_number_local);
            return result;
        }
        return nullptr;
    }

    const tabledir::Table* find_crex(const CrexTableID& id)
    {
        // First look it up in cache
        const auto i = crex_cache.find(id);
        if (i != crex_cache.end())
            return i->second;

        // If it is the first time this combination is requested, look for the best match
        CrexQuery query(id);
        for (vector<Dir>::iterator d = dirs.begin(); d != dirs.end(); ++d)
            query.search(*d);

        if (auto result = query.result())
        {
            crex_cache[id] = result;
            notes::logf("Matched table %s for mt %hhu mtv %hhu mtlv %hhu",
                    result->btable_id.c_str(),
                    id.master_table, id.master_table_version_number,
                    id.master_table_version_number_local);
            return result;
        }
        return nullptr;
    }

    const tabledir::Table* find(const std::string& basename)
    {
        for (auto& d: dirs)
            for (auto& t: d.tables)
                if (t->btable_id == basename)
                    return t;
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

const tabledir::Table* Tabledir::find_bufr(const BufrTableID& id)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_bufr(id);
}

const tabledir::Table* Tabledir::find_crex(const CrexTableID& id)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_crex(id);
}

const tabledir::Table* Tabledir::find(const std::string& basename)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find(basename);
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
