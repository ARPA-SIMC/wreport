#include "tabledir.h"
#include "error.h"
#include "wreport/vartable.h"
#include "wreport/dtable.h"
#include "wreport/notes.h"
#include "wreport/utils/sys.h"
#include "config.h"
#include <map>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cerrno>

using namespace std;

namespace wreport {
namespace tabledir {

Table::Table(const std::filesystem::path& dirname, const std::string& filename)
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

    btable_pathname = dirname / filename;
    dtable_pathname = dirname / ("D"s + filename.substr(1));
}

void Table::print_id(FILE* out) const
{
    fprintf(out, "(raw data)");
}

void BufrTable::print_id(FILE* out) const
{
    id.print(out);
}

void CrexTable::print_id(FILE* out) const
{
    id.print(out);
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
    if (!std::filesystem::exists(pathname))
        return;

    sys::Path dir(pathname);
    struct stat st;
    dir.fstat(st);
    if (mtime >= st.st_mtime)
        return;

    for (const auto& e: dir)
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
                            tables.push_back(new CrexTable(CrexTableID(ed, 0, 0, mt, mtv, 0, 0), pathname, e.d_name));
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

namespace {

struct Query
{
    BufrTable* bufr_best = nullptr;
    CrexTable* crex_best = nullptr;

    void consider_table(Table* t)
    {
        if (BufrTable* cur = dynamic_cast<BufrTable*>(t))
        {
            if (!is_acceptable(cur->id)) return;
            if (!bufr_best)
                bufr_best = cur;
            else
                bufr_best = choose_best(*bufr_best, *cur);
        }
        else if (CrexTable* cur = dynamic_cast<CrexTable*>(t))
        {
            if (!is_acceptable(cur->id)) return;
            if (!crex_best)
                crex_best = cur;
            else
                crex_best = choose_best(*crex_best, *cur);
        }
        // Ignore other kinds of tables
    }

    void search(const Dir& dir)
    {
        for (const auto& t : dir.tables)
            consider_table(t);
    }

    void explain_search(Dir& dir, FILE* out)
    {
        for (const auto& t : dir.tables)
        {
            fprintf(out, "%s: considering ", dir.pathname.c_str());
            t->print_id(out);
            consider_table(t);
            fprintf(out, ": best bufr: ");
            if (bufr_best)
                bufr_best->print_id(out);
            else
                fprintf(out, "none");
            fprintf(out, ", best crex: ");
            if (crex_best)
                crex_best->print_id(out);
            else
                fprintf(out, "none");
            fprintf(out, "\n");
        }
    }

    Table* result() const
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

    void explain_result(FILE* out) const
    {
        Table* res = result();
        fprintf(out, "Result chosen: ");
        if (res)
            res->print_id(out);
        else
            fprintf(out, "none");
        fprintf(out, "\n");
    }

    virtual ~Query() {}
    virtual bool is_acceptable(const BufrTableID& id) const = 0;
    virtual bool is_acceptable(const CrexTableID& id) const = 0;
    virtual BufrTable* choose_best(BufrTable& first, BufrTable& second) const = 0;
    virtual CrexTable* choose_best(CrexTable& first, CrexTable& second) const = 0;
    virtual Table* choose_best(BufrTable& first, CrexTable& second) const = 0;
};

/// Query for a BUFR table
struct BufrQuery : public Query
{
    BufrTableID id;

    explicit BufrQuery(const BufrTableID& id) : id(id) {}

    bool is_acceptable(const BufrTableID& id) const override
    {
        return this->id.is_acceptable_replacement(id);
    }

    bool is_acceptable(const CrexTableID& id) const override
    {
        return this->id.is_acceptable_replacement(id);
    }

    BufrTable* choose_best(BufrTable& first, BufrTable& second) const override
    {
        int cmp = id.closest_match(first.id, second.id);
        return cmp <= 0 ? &first : &second;
    }

    CrexTable* choose_best(CrexTable&, CrexTable&) const override
    {
        return nullptr;
    }

    Table* choose_best(BufrTable& first, CrexTable&) const override
    {
        return &first;
    }
};

/// Query for a CREX table
struct CrexQuery : public Query
{
    CrexTableID id;

    explicit CrexQuery(const CrexTableID& id) : id(id) {}

    bool is_acceptable(const BufrTableID& id) const override
    {
        return this->id.is_acceptable_replacement(id);
    }

    bool is_acceptable(const CrexTableID& id) const override
    {
        return this->id.is_acceptable_replacement(id);
    }

    BufrTable* choose_best(BufrTable& first, BufrTable& second) const override
    {
        int cmp = id.closest_match(first.id, second.id);
        return cmp <= 0 ? &first : &second;
    }

    CrexTable* choose_best(CrexTable& first, CrexTable& second) const override
    {
        int cmp = id.closest_match(first.id, second.id);
        return cmp <= 0 ? &first : &second;
    }

    Table* choose_best(BufrTable& first, CrexTable& second) const override
    {
        int cmp = id.closest_match(first.id, second.id);
        if (cmp <= 0)
            return &first;
        else
            return &second;
    }
};

}

struct Index
{
    vector<Dir> dirs;
    map<BufrTableID, const Table*> bufr_cache;
    map<CrexTableID, const Table*> crex_cache;

    explicit Index(const vector<string>& dirs)
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
            notes::logf("Matched table %s for ce %hu sc %hu mt %hhu mtv %hhu mtlv %hhu\n",
                    result->btable_id.c_str(),
                    id.originating_centre, id.originating_subcentre,
                    id.master_table_number, id.master_table_version_number, id.master_table_version_number_local);
            return result;
        }
        return nullptr;
    }

    void explain_find_bufr(const BufrTableID& id, FILE* out)
    {
        // If it is the first time this combination is requested, look for the best match
        BufrQuery query(id);
        for (vector<Dir>::iterator d = dirs.begin(); d != dirs.end(); ++d)
            query.explain_search(*d, out);
        query.explain_result(out);
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
            notes::logf("Matched table %s for mt %hhu mtv %hhu mtlv %hhu\n",
                    result->btable_id.c_str(),
                    id.master_table_number, id.master_table_version_number,
                    id.master_table_version_number_local);
            return result;
        }
        return nullptr;
    }

    void explain_find_crex(const CrexTableID& id, FILE* out)
    {
        // If it is the first time this combination is requested, look for the best match
        CrexQuery query(id);
        for (vector<Dir>::iterator d = dirs.begin(); d != dirs.end(); ++d)
            query.explain_search(*d, out);
        query.explain_result(out);
    }

    const tabledir::Table* find(const std::string& basename)
    {
        for (const auto& d: dirs)
            for (auto& t: d.tables)
                if (t->btable_id == basename)
                    return t;
        return nullptr;
    }

    void print(FILE* out) const
    {
        for (auto& d: dirs)
            for (auto& t: d.tables)
            {
                fprintf(out, "%s/%s:", d.pathname.c_str(), t->btable_id.c_str());
                t->print_id(out);
                fprintf(out, "\n");
            }
    }
};


Tabledirs::Tabledirs()
    : index(0)
{
}

Tabledirs::~Tabledirs()
{
    delete index;
}

void Tabledirs::add_default_directories()
{
    if (char* env = getenv("WREPORT_EXTRA_TABLES"))
        add_directory(env);
    if (char* env = getenv("WREPORT_TABLES"))
        add_directory(env);
    add_directory(TABLE_DIR);
}

void Tabledirs::add_directory(const std::string& dir)
{
    // Strip trailing /
    std::string clean_dir(dir);
    while (!clean_dir.empty() && clean_dir.back() == '/')
        clean_dir.pop_back();
    if (clean_dir.empty())
        clean_dir = "/";

    // Do not add a duplicate directory
    for (const auto& i: dirs)
        if (i == clean_dir)
            return;
    dirs.push_back(clean_dir);

    // Force a rebuild of the index
    delete index;
    index = 0;
}

const tabledir::Table* Tabledirs::find_bufr(const BufrTableID& id)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_bufr(id);
}

const tabledir::Table* Tabledirs::find_crex(const CrexTableID& id)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find_crex(id);
}

const tabledir::Table* Tabledirs::find(const std::string& basename)
{
    if (!index) index = new tabledir::Index(dirs);
    return index->find(basename);
}

void Tabledirs::print(FILE* out)
{
    if (!index) index = new tabledir::Index(dirs);
    index->print(out);
}

void Tabledirs::explain_find_bufr(const BufrTableID& id, FILE* out)
{
    if (!index) index = new tabledir::Index(dirs);
    index->explain_find_bufr(id, out);
}

void Tabledirs::explain_find_crex(const CrexTableID& id, FILE* out)
{
    if (!index) index = new tabledir::Index(dirs);
    index->explain_find_crex(id, out);
}

Tabledirs& Tabledirs::get()
{
    static Tabledirs* default_tabledir = 0;
    if (!default_tabledir)
    {
        default_tabledir = new Tabledirs();
        default_tabledir->add_default_directories();
    }
    return *default_tabledir;
}


}
}
