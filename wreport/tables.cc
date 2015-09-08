#include "tables.h"
#include "error.h"
#include "internals/tabledir.h"
#include "vartable.h"
#include "dtable.h"

using namespace std;

namespace wreport {

Tables::Tables()
    : btable(0), dtable(0)
{
}

Tables::Tables(Tables&& o)
    : btable(o.btable), dtable(o.dtable),
      bitmap_table(move(o.bitmap_table)),
      chardata_table(move(o.chardata_table)),
      unknown_table(move(o.unknown_table))
{
}

Tables::~Tables()
{
}

Tables& Tables::operator=(Tables&& o)
{
    if (this == &o) return *this;
    btable = o.btable;
    dtable = o.dtable;
    bitmap_table = move(o.bitmap_table);
    chardata_table = move(o.chardata_table);
    unknown_table = move(o.unknown_table);
    return *this;
}

bool Tables::loaded() const
{
    return btable && dtable;
}

void Tables::clear()
{
    btable = 0;
    dtable = 0;
    bitmap_table.clear();
    chardata_table.clear();
    unknown_table.clear();
}

void Tables::load_bufr(const BufrTableID& id)
{
    auto& tabledir = tabledir::Tabledirs::get();
    auto t = tabledir.find_bufr(id);
    if (!t)
        error_notfound::throwf("BUFR table for center %hu:%hu table %hhu:%hhu:%hhu not found",
                id.originating_centre, id.originating_subcentre,
                id.master_table_number, id.master_table_version_number,
                id.master_table_version_number_local);
    btable = Vartable::load_bufr(t->btable_pathname);
    dtable = DTable::load_bufr(t->dtable_pathname);
}

void Tables::load_crex(const CrexTableID& id)
{
    auto& tabledir = tabledir::Tabledirs::get();
    auto t = tabledir.find_crex(id);
    if (!t)
        error_notfound::throwf("CREX table for center %hu:%hu table %hhu:%hhu:%hhu:%hhu not found",
                id.originating_centre, id.originating_subcentre,
                id.master_table_number, id.master_table_version_number,
                id.master_table_version_number_local, id.master_table_version_number_bufr);
    btable = Vartable::load_crex(t->btable_pathname);
    dtable = DTable::load_crex(t->dtable_pathname);
}

Varinfo Tables::get_bitmap(Varcode code, const std::string& bitmap) const
{
    auto res = bitmap_table.find(bitmap);
    if (res != bitmap_table.end())
    {
        if (res->second.code != code)
            error_consistency::throwf("Bitmap '%s' has been requested with varcode %01d%02d%03d but it already exists as %01d%02d%03d",
                    bitmap.c_str(), WR_VAR_FXY(code), WR_VAR_FXY(res->second.code));
        return &(res->second);
    }

    auto new_entry = bitmap_table.emplace(make_pair(bitmap, _Varinfo()));
    _Varinfo& vi = new_entry.first->second;
    vi.set_string(code, "DATA PRESENT BITMAP", bitmap.size());
    return &vi;
}

Varinfo Tables::get_chardata(Varcode code, unsigned len) const
{
    auto res = chardata_table.find(code);
    if (res != chardata_table.end())
    {
        if (res->second.code != code)
            error_consistency::throwf("Character data with length %u has been requested with varcode %01d%02d%03d but it already exists as %01d%02d%03d",
                    len, WR_VAR_FXY(code), WR_VAR_FXY(res->second.code));
        return &(res->second);
    }

    auto new_entry = chardata_table.emplace(make_pair(code, _Varinfo()));
    _Varinfo& vi = new_entry.first->second;
    vi.set_string(code, "CHARACTER DATA", len);
    return &vi;
}

Varinfo Tables::get_unknown(Varcode code, unsigned bit_len) const
{
    auto res = unknown_table.find(bit_len);
    if (res != unknown_table.end())
    {
        if (res->second.code != code)
            error_consistency::throwf("Unknown binary data %u bits long has been requested with varcode %01d%02d%03d but it already exists as %01d%02d%03d",
                    bit_len, WR_VAR_FXY(code), WR_VAR_FXY(res->second.code));
        return &(res->second);
    }

    auto new_entry = unknown_table.emplace(make_pair(bit_len, _Varinfo()));
    _Varinfo& vi = new_entry.first->second;
    vi.set_binary(code, "UNKNOWN LOCAL DESCRIPTOR", bit_len);
    return &vi;
}

}
