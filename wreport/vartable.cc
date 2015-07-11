/*
 * Future optimizations for dba_vartable can make use of string tables to store
 * varinfo descriptions and units instead of long fixed-length records.
 * However, the string table cannot grow dynamically or it will invalidate the
 * existing string pointers.
 */

#include "vartable.h"
#include "tableinfo.h"
#include "error.h"
#include "internals/tabledir.h"
#include <memory>
#include <map>
#include <cstring>
#include <cmath>
#include <climits>
#include "config.h"


// #define TRACE_LOADER

#ifdef TRACE_LOADER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {

Vartable::~Vartable() {}


namespace {

unsigned digits_per_bits(unsigned bit_count)
{
    // for i in range(33): print("case {:2}: return {:2}; // {}".format(i, len(str(2**i)), 2**i))
    switch (bit_count)
    {
        case  0: error_consistency::throwf("binary values 0 bits long are not supported");
        case  1: return  1; // 2
        case  2: return  1; // 4
        case  3: return  1; // 8
        case  4: return  2; // 16
        case  5: return  2; // 32
        case  6: return  2; // 64
        case  7: return  3; // 128
        case  8: return  3; // 256
        case  9: return  3; // 512
        case 10: return  4; // 1024
        case 11: return  4; // 2048
        case 12: return  4; // 4096
        case 13: return  4; // 8192
        case 14: return  5; // 16384
        case 15: return  5; // 32768
        case 16: return  5; // 65536
        case 17: return  6; // 131072
        case 18: return  6; // 262144
        case 19: return  6; // 524288
        case 20: return  7; // 1048576
        case 21: return  7; // 2097152
        case 22: return  7; // 4194304
        case 23: return  7; // 8388608
        case 24: return  8; // 16777216
        case 25: return  8; // 33554432
        case 26: return  8; // 67108864
        case 27: return  9; // 134217728
        case 28: return  9; // 268435456
        case 29: return  9; // 536870912
        case 30: return 10; // 1073741824
        case 31: return 10; // 2147483648
        case 32: return 10; // 4294967296
        default:
            error_consistency::throwf("binary values of more than 32 bits are not supported");
    }
}

struct fd_closer
{
    FILE* fd;
    fd_closer(FILE* fd) : fd(fd) {}
    ~fd_closer() { fclose(fd); }
};

static long getnumber(char* str)
{
    while (*str && isspace(*str))
        ++str;
    if (!*str) return 0;
    if (*str == '-')
    {
        ++str;
        // Eat spaces after the - (oh my this makes me sad)
        while (*str && isspace(*str))
            ++str;
        return -strtol(str, 0, 10);
    } else
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

    VartableEntry(const VartableEntry& other, int new_scale, unsigned new_bit_len)
        : varinfo(other.varinfo), alterations(other.alterations)
    {
#if 0
        fprintf(stderr, "Before alteration(w:%d,s:%d): bl %d len %d scale %d\n",
                WR_ALT_WIDTH(change), WR_ALT_SCALE(change),
                i->bit_len, i->len, i->scale);
#endif

        // Apply the alterations
        varinfo.bit_len = new_bit_len;
        switch (varinfo.type)
        {
            case Vartype::Integer:
            case Vartype::Decimal:
                varinfo.len = digits_per_bits(varinfo.bit_len);
                break;
            case Vartype::String:
                varinfo.len = varinfo.bit_len / 8;
                break;
            case Vartype::Binary:
                varinfo.len = ceil(varinfo.bit_len / 8);
                break;
        }

        varinfo.scale = new_scale;
        switch (varinfo.type)
        {
            case Vartype::String:
            case Vartype::Binary:
                break;
            case Vartype::Integer:
                if (varinfo.scale) varinfo.type = Vartype::Decimal;
                break;
            case Vartype::Decimal:
                if (!varinfo.scale) varinfo.type = Vartype::Integer;
                break;
        }

#if 0
        fprintf(stderr, "After alteration(w:%d,s:%d): bl %d len %d scale %d\n",
                WR_ALT_WIDTH(change), WR_ALT_SCALE(change),
                i->bit_len, i->len, i->scale);
#endif

        // Postprocess the data, filling in minval and maxval
        varinfo.compute_range();
    }

    /**
     * Search for this alteration in the alteration chain.
     *
     * Returns nullptr if it was not found
     */
    const VartableEntry* get_alteration(int new_scale, unsigned new_bit_len) const
    {
        if (varinfo.scale == new_scale && varinfo.bit_len == new_bit_len)
            return this;
        if (alterations == nullptr)
            return nullptr;
        return alterations->get_alteration(new_scale, new_bit_len);
    }
};

/// Base Vartable implementation
struct VartableBase : public Vartable
{
    /// Pathname to the file from which this vartable has been loaded
    std::string m_pathname;

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


    VartableBase(const std::string& pathname)
        : m_pathname(pathname)
    {
    }

    std::string pathname() const override
    {
        return m_pathname;
    }

    _Varinfo* obtain(const std::string& pathname, unsigned line_no, Varcode code)
    {
        // Ensure that we are creating an ordered table
        if (!entries.empty() && entries.back().varinfo.code >= code)
            throw error_parse(m_pathname.c_str(), line_no, "input file is not sorted");

        // Append a new entry;
        entries.emplace_back(VartableEntry());
        _Varinfo* entry = &entries.back().varinfo;
        entry->code = code;
        return entry;
    }

    const VartableEntry* query_entry(Varcode code) const
    {
        int begin, end;

        // Binary search
        begin = -1, end = entries.size();
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
            error_notfound::throwf(
                    "variable %d%02d%03d not found in table %s",
                    WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code), m_pathname.c_str());
        else
            return &(e->varinfo);
    }

    bool contains(Varcode code) const override
    {
        return query_entry(code) != nullptr;
    }

    Varinfo query_altered(Varcode code, int new_scale, unsigned new_bit_len) const override
    {
        // Get the normal variable
        const VartableEntry* start = query_entry(code);
        if (!start)
            error_notfound::throwf(
                    "variable %d%02d%03d not found in table %s",
                    WR_VAR_FXY(code), m_pathname.c_str());

        // Look for an existing alteration
        const VartableEntry* alt = start->get_alteration(new_scale, new_bit_len);
        if (alt) return &(alt->varinfo);

        switch (start->varinfo.type)
        {
            case Vartype::Integer:
            case Vartype::Decimal:
                if (new_scale < -16 || new_scale > 16)
                    error_consistency::throwf("cannot alter variable %d%02d%03d with a new scale of %d", WR_VAR_FXY(code), new_scale);
                if (new_bit_len > 32)
                    error_consistency::throwf("cannot alter variable %d%02d%03d with a new bit_len of %u", WR_VAR_FXY(code), new_bit_len);
                break;
            case Vartype::String:
            case Vartype::Binary:
                break;
        }


        // Not found: we need to create it, duplicating the original varinfo
        unique_ptr<VartableEntry> newvi(new VartableEntry(*start, new_scale, new_bit_len));

        // Add the new alteration as the first alteration in the list after the
        // original value
        start->alterations = newvi.release();

        return &(start->alterations->varinfo);
    }
};

struct BufrVartable : public VartableBase
{
    /// Create and load a BUFR B table
    BufrVartable(const std::string& pathname) : VartableBase(pathname)
    {
        FILE* in = fopen(pathname.c_str(), "rt");
        if (!in) error_system::throwf("cannot open BUFR table file %s", pathname.c_str());
        fd_closer closer(in); // Close in on exit

        char line[200];
        int line_no = 0;
        while (fgets(line, 200, in) != NULL)
        {
            line_no++;

            if (strlen(line) < 119)
                throw error_parse(pathname.c_str(), line_no, "bufr table line too short");
            // fprintf(stderr, "Line: %s\n", line);

            /* FMT='(1x,A,1x,A64,47x,A24,I3,8x,I3)' */

            // Append a new entry;
            _Varinfo* entry = obtain(pathname, line_no, WR_STRING_TO_VAR(line + 2));

            // Read the description
            memcpy(entry->desc, line+8, 64);
            // Convert the description from space-padded to zero-padded
            for (int i = 63; i >= 0 && isspace(entry->desc[i]); --i)
                entry->desc[i] = 0;

            // Read the unit
            memcpy(entry->unit, line+73, 24);
            // Convert the unit from space-padded to zero-padded
            for (int i = 23; i >= 0 && isspace(entry->unit[i]); --i)
                entry->unit[i] = 0;

            entry->scale = getnumber(line+98);
            entry->bit_ref = getnumber(line+102);
            entry->bit_len = getnumber(line+115);

            // Set the is_string flag based on the unit
            if (strcmp(entry->unit, "CCITTIA5") == 0)
            {
                entry->type = Vartype::String;
                entry->len = entry->bit_len / 8;
            }
            else
            {
                if (entry->scale)
                    entry->type = Vartype::Decimal;
                else
                    entry->type = Vartype::Integer;

                // Compute the decimal length as the maximum number of digits
                // needed to encode 2**bit_len
                if (entry->bit_len == 1)
                    entry->len = 1;
                else
                    entry->len = ceil(entry->bit_len * log10(2.0));
            }

            // Postprocess the data, filling in minval and maxval
            entry->compute_range();

            /*
            fprintf(stderr, "Debug: B%05d len %d scale %d type %s desc %s\n",
                    bcode, entry->len, entry->scale, entry->type, entry->desc);
            */
        }
    }
};

struct CrexVartable : public VartableBase
{
    /// Create and load a CREX B table
    CrexVartable(const std::string& pathname) : VartableBase(pathname)
    {
        FILE* in = fopen(pathname.c_str(), "rt");
        if (!in) error_system::throwf("cannot open CREX table file %s", pathname.c_str());
        fd_closer closer(in); // Close in on exit

        char line[200];
        int line_no = 0;
        while (fgets(line, 200, in) != NULL)
        {
            line_no++;
            Varcode code = WR_STRING_TO_VAR(line + 2);

            if (strlen(line) < 157)
            {
                // Rows for delayed replicators do not have crex entries, so we
                // skip them
                if (WR_VAR_X(code) != 31)
                    throw error_parse(pathname.c_str(), line_no, "crex table line too short");
                else
                    continue;
            }
            // fprintf(stderr, "Line: %s\n", line);

            /* FMT='(1x,A,1x,A64,47x,A24,I3,8x,I3)' */

            // Append a new entry;
            _Varinfo* entry = obtain(pathname, line_no, code);

            // Read the description
            memcpy(entry->desc, line+8, 64);
            // Convert the description from space-padded to zero-padded
            for (int i = 63; i >= 0 && isspace(entry->desc[i]); --i)
                entry->desc[i] = 0;

            // Read the CREX unit
            memcpy(entry->unit, line+119, 24);
            // Convert the unit from space-padded to zero-padded
            for (int i = 23; i >= 0 && isspace(entry->unit[i]); --i)
                entry->unit[i] = 0;

            entry->scale = getnumber(line+143);
            entry->len = getnumber(line+149);

            // Ignore the BUFR part: since it can have a different measurement
            // unit, we cannot really use that information. It will just mean
            // that values loaded using CREX tables cannot be encoded in binary
            entry->bit_ref = 0;
            entry->bit_len = 0;

            // Set the is_string flag based on the unit
            if (strcmp(entry->unit, "CHARACTER") == 0)
                entry->type = Vartype::String;
            else if (entry->scale)
                entry->type = Vartype::Decimal;
            else
                entry->type = Vartype::Integer;

            // Postprocess the data, filling in minval and maxval
            entry->compute_range();

            /*
            fprintf(stderr, "Debug: B%05d len %d scale %d type %s desc %s\n",
                    bcode, entry->len, entry->scale, entry->type, entry->desc);
            */
        }
    }
};

}


const Vartable* Vartable::load_bufr(const std::string& pathname)
{
    static std::map<string, BufrVartable*>* tables = 0;
    if (!tables) tables = new std::map<string, BufrVartable*>;

    // Return it from cache if we have it
    auto i = tables->find(pathname);
    if (i != tables->end())
        return i->second;

    // Else, instantiate it
    return (*tables)[pathname] = new BufrVartable(pathname);
}

const Vartable* Vartable::load_crex(const std::string& pathname)
{
    static std::map<string, CrexVartable*>* tables = 0;
    if (!tables) tables = new std::map<string, CrexVartable*>;

    // Return it from cache if we have it
    auto i = tables->find(pathname);
    if (i != tables->end())
        return i->second;

    // Else, instantiate it
    return (*tables)[pathname] = new CrexVartable(pathname);
}

const Vartable* Vartable::get_bufr(const BufrTableID& id)
{
    auto tabledir = tabledir::Tabledir::get();
    auto res = tabledir.find_bufr(id);
    return res->btable;
}

const Vartable* Vartable::get_crex(const CrexTableID& id)
{
    auto tabledir = tabledir::Tabledir::get();
    auto res = tabledir.find_crex(id);
    return res->btable;
}

const Vartable* Vartable::get_bufr(const std::string& basename)
{
    auto tabledir = tabledir::Tabledir::get();
    auto res = tabledir.find(basename);
    return res->btable;
}

const Vartable* Vartable::get_crex(const std::string& basename)
{
    auto tabledir = tabledir::Tabledir::get();
    auto res = tabledir.find(basename);
    return res->btable;
}


}
