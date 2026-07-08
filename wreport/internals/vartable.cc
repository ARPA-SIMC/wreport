#include "vartable.h"
#include "tableinfo.h"
#include "varinfo.h"
#include "wreport/error.h"
#include "wreport/notes.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstring>
#include <functional>
#include <memory>

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

} // namespace

namespace wreport::vartable {

Entry::Entry(const Entry& other, int new_scale, unsigned new_bit_len,
             int new_bit_ref)
    : varinfo(other.varinfo), alterations(other.alterations)
{
#if 0
        fprintf(stderr, "Before alteration(w:%d,s:%d): bl %d len %d scale %d\n",
                WR_ALT_WIDTH(change), WR_ALT_SCALE(change),
                i->bit_len, i->len, i->scale);
#endif

    // Apply the alterations
    varinfo::set_bufr(varinfo, varinfo.code, varinfo.desc, varinfo.unit,
                      new_bit_len, new_bit_ref, new_scale);

#if 0
        fprintf(stderr, "After alteration(w:%d,s:%d): bl %d len %d scale %d\n",
                WR_ALT_WIDTH(change), WR_ALT_SCALE(change),
                i->bit_len, i->len, i->scale);
#endif
}

const Entry* Entry::get_alteration(int new_scale, unsigned new_bit_len,
                                   int new_bit_ref) const
{
    if (varinfo.scale == new_scale && varinfo.bit_len == new_bit_len &&
        varinfo.bit_ref == new_bit_ref)
        return this;
    if (alterations == nullptr)
        return nullptr;
    return alterations->get_alteration(new_scale, new_bit_len, new_bit_ref);
}

Base::Base(const std::filesystem::path& pathname) : m_pathname(pathname) {}

_Varinfo* Base::obtain(unsigned line_no, Varcode code)
{
    // Ensure that we are creating an ordered table
    if (!entries.empty() && entries.back().varinfo.code >= code)
        throw error_parse(m_pathname.c_str(), line_no,
                          "input file is not sorted");

    // Append a new entry;
    entries.emplace_back(Entry());
    _Varinfo* entry = &entries.back().varinfo;
    entry->code     = code;
    return entry;
}

const Entry* Base::query_entry(Varcode code) const
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

Varinfo Base::query(Varcode code) const
{
    auto e = query_entry(code);
    if (!e)
        error_notfound::throwf("variable %d%02d%03d not found in table %s",
                               WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code),
                               m_pathname.c_str());
    else
        return &(e->varinfo);
}

bool Base::contains(Varcode code) const { return query_entry(code) != nullptr; }

Varinfo Base::query_altered(Varcode code, int new_scale, unsigned new_bit_len,
                            int new_bit_ref) const
{
    // Get the normal variable
    const Entry* start = query_entry(code);
    if (!start)
        error_notfound::throwf("variable %d%02d%03d not found in table %s",
                               WR_VAR_FXY(code), m_pathname.c_str());

    // Look for an existing alteration
    const Entry* alt =
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
    std::unique_ptr<Entry> newvi(
        new Entry(*start, new_scale, new_bit_len, new_bit_ref));

    // Add the new alteration as the first alteration in the list after the
    // original value
    start->alterations = newvi.release();

    return &(start->alterations->varinfo);
}

bool Base::iterate(std::function<bool(Varinfo)> dest) const
{
    for (const auto& entry : entries)
        for (const Entry* e = &entry; e; e = e->alterations)
            if (!dest(&(e->varinfo)))
                return false;
    return true;
}

Bufr::Bufr(const std::filesystem::path& pathname) : Base(pathname)
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

        varinfo::set_bufr(*entry, entry->code, desc_buf, unit_buf,
                          // bit_len
                          static_cast<unsigned>(getnumber(line + 115)),
                          // bit_ref
                          static_cast<int>(getnumber(line + 102)),
                          // scale
                          static_cast<int>(getnumber(line + 98)));
    }
}

Crex::Crex(const std::filesystem::path& pathname) : Base(pathname)
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

        varinfo::set_crex(*entry, code, desc_buf, unit_buf,
                          // Length
                          static_cast<unsigned>(getnumber(line + 149)),
                          // Scale
                          static_cast<int>(getnumber(line + 143)));
        ++found;
    }
    if (!found)
        error_consistency::throwf(
            "%s: table does not contain any CREX information",
            pathname.c_str());
}

} // namespace wreport::vartable
