/*
 * Future optimizations for dba_vartable can make use of string tables to store
 * varinfo descriptions and units instead of long fixed-length records.
 * However, the string table cannot grow dynamically or it will invalidate the
 * existing string pointers.
 */

#include "vartable.h"
#include "config.h"
#include "error.h"
#include "internals/tabledir.h"
#include "notes.h"
#include "tableinfo.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstring>
#include <map>
#include <memory>

using namespace std;

namespace wreport {

Vartable::~Vartable() {}

namespace {

struct fd_closer
{
    FILE* fd;
    explicit fd_closer(FILE* fd) : fd(fd) {}
    ~fd_closer() { fclose(fd); }
};

static long getnumber(const char* str)
{
    while (*str && isspace(*str))
        ++str;
    if (!*str)
        return 0;
    if (*str == '-')
    {
        ++str;
        // Eat spaces after the - (oh my this makes me sad)
        while (*str && isspace(*str))
            ++str;
        return -strtol(str, 0, 10);
    }
    else
        return strtol(str, 0, 10);
}

struct VartableEntry
{
    /**
     * Master Varinfo structure for this entry.
     *
     * A point to this will be given out and shared by all the code that needs
     * to refer to informations about this variable.
     */
    _Varinfo varinfo;

    /**
     * Altered versions of this Varinfo.
     *
     * BUFR messages can trasmit variables encoded with variations of standard
     * BUFR/CREX B table entries, by overriding reference codes or bit lengths.
     *
     * Altered versions of a Varinfo are stored in this chain. The first
     * element of the chain is always the original Varinfo defined in the B
     * table.
     */
    mutable VartableEntry* alterations = nullptr;

    VartableEntry() = default;

    /**
     * Build an altered entry created for BUFR table C modifiers
     */
    VartableEntry(const VartableEntry& other, int new_scale,
                  unsigned new_bit_len, int new_bit_ref)
        : varinfo(other.varinfo), alterations(other.alterations)
    {
#if 0
        fprintf(stderr, "Before alteration(w:%d,s:%d): bl %d len %d scale %d\n",
                WR_ALT_WIDTH(change), WR_ALT_SCALE(change),
                i->bit_len, i->len, i->scale);
#endif

        // Apply the alterations
        varinfo.set_bufr(varinfo.code, varinfo.desc, varinfo.unit, new_scale,
                         new_bit_ref, new_bit_len);

#if 0
        fprintf(stderr, "After alteration(w:%d,s:%d): bl %d len %d scale %d\n",
                WR_ALT_WIDTH(change), WR_ALT_SCALE(change),
                i->bit_len, i->len, i->scale);
#endif
    }

    /**
     * Search for this alteration in the alteration chain.
     *
     * Returns nullptr if it was not found
     */
    const VartableEntry* get_alteration(int new_scale, unsigned new_bit_len,
                                        int new_bit_ref) const
    {
        if (varinfo.scale == new_scale && varinfo.bit_len == new_bit_len &&
            varinfo.bit_ref == new_bit_ref)
            return this;
        if (alterations == nullptr)
            return nullptr;
        return alterations->get_alteration(new_scale, new_bit_len, new_bit_ref);
    }
};

/// Base Vartable implementation
struct VartableBase : public Vartable
{
    /// Pathname to the file from which this vartable has been loaded
    std::filesystem::path m_pathname;

    /**
     * Entries in this Vartable.
     *
     * The entries are sorted by varcode, so that we can look them up by binary
     * search.
     *
     * Since we are handing out pointers to _Varinfo structures inside the
     * vector, those pointers will be invalidated if a vector reallocation gets
     * triggered. This means that once the table has been loaded, it size cannot
     * be changed anymore.
     */
    std::vector<VartableEntry> entries;

    explicit VartableBase(const std::filesystem::path& pathname)
        : m_pathname(pathname)
    {
    }

    std::string pathname() const override { return m_pathname; }

    std::filesystem::path path() const override { return m_pathname; }

    _Varinfo* obtain(unsigned line_no, Varcode code)
    {
        // Ensure that we are creating an ordered table
        if (!entries.empty() && entries.back().varinfo.code >= code)
            throw error_parse(m_pathname.c_str(), line_no,
                              "input file is not sorted");

        // Append a new entry;
        entries.emplace_back(VartableEntry());
        _Varinfo* entry = &entries.back().varinfo;
        entry->code     = code;
        return entry;
    }

    const VartableEntry* query_entry(Varcode code) const
    {
        int begin, end;

        // Binary search
        begin = -1, end = static_cast<int>(entries.size());
        while (end - begin > 1)
        {
            int cur = (end + begin) / 2;
            if (entries[cur].varinfo.code > code)
                end = cur;
            else
                begin = cur;
        }
        if (begin == -1 || entries[begin].varinfo.code != code)
            return nullptr;
        else
            return &entries[begin];
    }

    Varinfo query(Varcode code) const override
    {
        auto e = query_entry(code);
        if (!e)
            error_notfound::throwf("variable %d%02d%03d not found in table %s",
                                   WR_VAR_F(code), WR_VAR_X(code),
                                   WR_VAR_Y(code), m_pathname.c_str());
        else
            return &(e->varinfo);
    }

    bool contains(Varcode code) const override
    {
        return query_entry(code) != nullptr;
    }

    Varinfo query_altered(Varcode code, int new_scale, unsigned new_bit_len,
                          int new_bit_ref) const override
    {
        // Get the normal variable
        const VartableEntry* start = query_entry(code);
        if (!start)
            error_notfound::throwf("variable %d%02d%03d not found in table %s",
                                   WR_VAR_FXY(code), m_pathname.c_str());

        // Look for an existing alteration
        const VartableEntry* alt =
            start->get_alteration(new_scale, new_bit_len, new_bit_ref);
        if (alt)
            return &(alt->varinfo);

        switch (start->varinfo.type)
        {
            case Vartype::Integer:
            case Vartype::Decimal:
                if (new_scale < -16 || new_scale > 16)
                    error_consistency::throwf(
                        "cannot alter variable %d%02d%03d with a new scale of "
                        "%d",
                        WR_VAR_FXY(code), new_scale);
                if (new_bit_len > 32)
                    error_consistency::throwf(
                        "cannot alter variable %d%02d%03d with a new bit_len "
                        "of %u",
                        WR_VAR_FXY(code), new_bit_len);
                break;
            case Vartype::String:
            case Vartype::Binary: break;
        }

        // Not found: we need to create it, duplicating the original varinfo
        unique_ptr<VartableEntry> newvi(
            new VartableEntry(*start, new_scale, new_bit_len, new_bit_ref));

        // Add the new alteration as the first alteration in the list after the
        // original value
        start->alterations = newvi.release();

        return &(start->alterations->varinfo);
    }

    bool iterate(std::function<bool(Varinfo)> dest) const override
    {
        for (const auto& entry : entries)
            for (const VartableEntry* e = &entry; e; e = e->alterations)
                if (!dest(&(e->varinfo)))
                    return false;
        return true;
    }
};

static void normalise_unit(char* unit)
{
    if (strncmp(unit, "CODE TABLE", 10) == 0 ||
        strncmp(unit, "CODETABLE", 9) == 0)
        strcpy(unit, "CODE TABLE");
    else if (strncmp(unit, "FLAG TABLE", 10) == 0 ||
             strncmp(unit, "FLAGTABLE", 9) == 0)
        strcpy(unit, "FLAG TABLE");
}

static void read_space_padded(char* dest, const char* src, size_t len)
{
    memcpy(dest, src, len);
    dest[len] = 0;
    // Convert the description from space-padded to zero-padded
    for (int i = len - 1; i >= 0 && isspace(dest[i]); --i)
        dest[i] = 0;
}

struct BufrVartable : public VartableBase
{
    /// Create and load a BUFR B table
    explicit BufrVartable(const std::filesystem::path& pathname)
        : VartableBase(pathname)
    {
        FILE* in = fopen(pathname.c_str(), "rt");
        if (!in)
            error_system::throwf("cannot open BUFR table file %s",
                                 pathname.c_str());
        fd_closer closer(in); // Close in on exit

        char desc_buf[65];
        char unit_buf[25];
        char line[200];
        int line_no = 0;
        while (fgets(line, 200, in) != NULL)
        {
            size_t line_length = strlen(line);
            line_no++;

            if (line_length < 119)
                throw error_parse(pathname.c_str(), line_no,
                                  "bufr table line too short");
            // fprintf(stderr, "Line: %s\n", line);
            // FMT='(1x,A,1x,A64,47x,A24,I3,8x,I3)'

            // Append a new entry;
            _Varinfo* entry = obtain(line_no, WR_STRING_TO_VAR(line + 2));

            // Read the description
            read_space_padded(desc_buf, line + 8, 64);

            // Read the BUFR unit
            read_space_padded(unit_buf, line + 73, 24);
            normalise_unit(unit_buf);

            entry->set_bufr(entry->code, desc_buf, unit_buf,
                            // scale
                            static_cast<int>(getnumber(line + 98)),
                            // bit_ref
                            static_cast<int>(getnumber(line + 102)),
                            // bit_len
                            static_cast<unsigned>(getnumber(line + 115)));
        }
    }
};

struct CrexVartable : public VartableBase
{
    /// Create and load a CREX B table
    explicit CrexVartable(const std::filesystem::path& pathname)
        : VartableBase(pathname)
    {
        FILE* in = fopen(pathname.c_str(), "rt");
        if (!in)
            error_system::throwf("cannot open CREX table file %s",
                                 pathname.c_str());
        fd_closer closer(in); // Close in on exit

        char desc_buf[65];
        char unit_buf[25];
        char line[200];
        int line_no    = 0;
        unsigned found = 0;
        while (fgets(line, 200, in) != NULL)
        {
            line_no++;
            Varcode code = WR_STRING_TO_VAR(line + 2);

            if (strlen(line) < 157)
                continue;
            // fprintf(stderr, "Line: %s\n", line);
            // FMT='(1x,A,1x,A64,47x,A24,I3,8x,I3)'

            // Append a new entry;
            _Varinfo* entry = obtain(line_no, code);

            // Read the description
            read_space_padded(desc_buf, line + 8, 64);

            // Read the CREX unit
            read_space_padded(unit_buf, line + 119, 24);
            normalise_unit(unit_buf);

            entry->set_crex(code, desc_buf, unit_buf,
                            // Scale
                            static_cast<int>(getnumber(line + 143)),
                            // Length
                            static_cast<unsigned>(getnumber(line + 149)));
            ++found;
        }
        if (!found)
            error_consistency::throwf(
                "%s: table does not contain any CREX information",
                pathname.c_str());
    }
};

} // namespace

const Vartable* Vartable::load_bufr(const char* pathname)
{
    return load_bufr(std::filesystem::path(pathname));
}

const Vartable* Vartable::load_bufr(const std::string& pathname)
{
    return load_bufr(std::filesystem::path(pathname));
}

const Vartable* Vartable::load_bufr(const std::filesystem::path& pathname)
{
    static std::map<string, BufrVartable*>* tables = 0;
    if (!tables)
        tables = new std::map<string, BufrVartable*>;

    // Return it from cache if we have it
    auto i = tables->find(pathname);
    if (i != tables->end())
        return i->second;

    // Else, instantiate it
    return (*tables)[pathname] = new BufrVartable(pathname);
}

const Vartable* Vartable::load_crex(const char* pathname)
{
    return load_crex(std::filesystem::path(pathname));
}

const Vartable* Vartable::load_crex(const std::string& pathname)
{
    return load_crex(std::filesystem::path(pathname));
}

const Vartable* Vartable::load_crex(const std::filesystem::path& pathname)
{
    static std::map<string, CrexVartable*>* tables = 0;
    if (!tables)
        tables = new std::map<string, CrexVartable*>;

    // Return it from cache if we have it
    auto i = tables->find(pathname);
    if (i != tables->end())
        return i->second;

    // Else, instantiate it
    return (*tables)[pathname] = new CrexVartable(pathname);
}

const Vartable* Vartable::get_bufr(const BufrTableID& id)
{
    auto& tabledir = tabledir::Tabledirs::get();
    auto res       = tabledir.find_bufr(id);
    if (!res)
        error_notfound::throwf(
            "BUFR table for centre %hu:%hu and tables %hhu:%hhu:%hhu not found",
            id.originating_centre, id.originating_subcentre,
            id.master_table_number, id.master_table_version_number,
            id.master_table_version_number_local);
    return load_bufr(res->btable_pathname);
}

const Vartable* Vartable::get_crex(const CrexTableID& id)
{
    auto& tabledir = tabledir::Tabledirs::get();
    auto res       = tabledir.find_crex(id);
    if (!res)
        error_notfound::throwf("CREX table for centre %hu:%hu and tables "
                               "%hhu:%hhu:%hhu:%hhu not found",
                               id.originating_centre, id.originating_subcentre,
                               id.master_table_number,
                               id.master_table_version_number,
                               id.master_table_version_number_local,
                               id.master_table_version_number_bufr);
    return load_crex(res->btable_pathname);
}

const Vartable* Vartable::get_bufr(const std::string& basename)
{
    auto& tabledir = tabledir::Tabledirs::get();
    auto res       = tabledir.find(basename);
    if (!res)
        error_notfound::throwf("BUFR table %s not found", basename.c_str());
    return load_bufr(res->btable_pathname);
}

const Vartable* Vartable::get_crex(const std::string& basename)
{
    auto& tabledir = tabledir::Tabledirs::get();
    auto res       = tabledir.find(basename);
    if (!res)
        error_notfound::throwf("CREX table %s not found", basename.c_str());
    return load_crex(res->btable_pathname);
}

} // namespace wreport
