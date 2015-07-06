#include "tables.h"
#include "error.h"
#include "internals/tabledir.h"

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

void Tables::load_bufr(int centre, int subcentre, int master_table, int local_table)
{
    auto tabledir = tabledir::Tabledir::get();
    auto t = tabledir.find_bufr(centre, subcentre, master_table, local_table);
    btable = t->btable;
    dtable = t->dtable;
}

void Tables::load_crex(int master_table_number, int edition, int table)
{
    auto tabledir = tabledir::Tabledir::get();
    auto t = tabledir.find_crex(master_table_number, edition, table);
    btable = t->btable;
    dtable = t->dtable;
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

Varinfo Tables::get_chardata(Varcode code, const std::string& chardata) const
{
    auto res = chardata_table.find(chardata);
    if (res != chardata_table.end())
    {
        if (res->second.code != code)
            error_consistency::throwf("Character data '%s' has been requested with varcode %01d%02d%03d but it already exists as %01d%02d%03d",
                    chardata.c_str(), WR_VAR_FXY(code), WR_VAR_FXY(res->second.code));
        return &(res->second);
    }

    auto new_entry = chardata_table.emplace(make_pair(chardata, _Varinfo()));
    _Varinfo& vi = new_entry.first->second;
    vi.set_string(code, "CHARACTER DATA", chardata.size());
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
