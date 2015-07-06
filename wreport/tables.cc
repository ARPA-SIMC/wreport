#include "tables.h"
#include "internals/tabledir.h"

using namespace std;

namespace wreport {

Tables::Tables()
    : btable(0), dtable(0)
{
}

Tables::Tables(Tables&& o)
    : btable(o.btable), dtable(o.dtable), local_vartable(move(o.local_vartable))
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
    local_vartable = move(o.local_vartable);
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
    local_vartable.clear();
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

_Varinfo* Tables::new_entry()
{
    local_vartable.emplace_front();
    return &local_vartable.front();
}

Varinfo Tables::get_bitmap_entry(Varcode code, unsigned size)
{
    auto res = new_entry();
    res->set_string(code, "DATA PRESENT BITMAP", size);
    return res;
}

Varinfo Tables::get_chardata_entry(Varcode code, unsigned size)
{
    auto res = new_entry();
    res->set_string(code, "CHARACTER DATA", size);
    return res;
}

Varinfo Tables::get_unknown(Varcode code, unsigned bit_len)
{
    auto res = new_entry();
    res->set_binary(code, "UNKNOWN LOCAL DESCRIPTOR", bit_len);
    return res;
}

}
