#include "tables.h"
#include "dtable.h"
#include "error.h"
#include "internals/tabledir.h"
#include "internals/varinfo.h"
#include "vartable.h"

using namespace std;

namespace wreport {

Tables::Tables() : btable(0), dtable(0) {}

Tables::Tables(Tables&& o)
    : btable(o.btable), dtable(o.dtable),
      bitmap_table(std::move(o.bitmap_table)),
      chardata_table(std::move(o.chardata_table)),
      unknown_table(std::move(o.unknown_table))
{
}

Tables::~Tables() {}

Tables& Tables::operator=(Tables&& o)
{
    if (this == &o)
        return *this;
    btable         = o.btable;
    dtable         = o.dtable;
    bitmap_table   = std::move(o.bitmap_table);
    chardata_table = std::move(o.chardata_table);
    unknown_table  = std::move(o.unknown_table);
    return *this;
}

bool Tables::loaded() const { return btable && dtable; }

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
    auto t         = tabledir.find_bufr(id);
    if (!t)
        error_notfound::throwf(
            "BUFR table for center %hu:%hu table %hhu:%hhu:%hhu not found",
            id.originating_centre, id.originating_subcentre,
            id.master_table_number, id.master_table_version_number,
            id.master_table_version_number_local);
    btable = Vartable::load_bufr(t->btable_pathname);
    dtable = DTable::load_bufr(t->dtable_pathname);
}

void Tables::load_crex(const CrexTableID& id)
{
    auto& tabledir = tabledir::Tabledirs::get();
    auto t         = tabledir.find_crex(id);
    if (!t)
        error_notfound::throwf(
            "CREX table for center %hu:%hu table %hhu:%hhu:%hhu:%hhu not found",
            id.originating_centre, id.originating_subcentre,
            id.master_table_number, id.master_table_version_number,
            id.master_table_version_number_local,
            id.master_table_version_number_bufr);
    btable = Vartable::load_crex(t->btable_pathname);
    dtable = DTable::load_crex(t->dtable_pathname);
}

Varinfo Tables::get_bitmap(Varcode code, const std::string& bitmap) const
{
    auto res = bitmap_table.find(bitmap);
    if (res != bitmap_table.end())
    {
        if (res->second.code != code)
            error_consistency::throwf(
                "Bitmap '%s' has been requested with varcode %01d%02d%03d but "
                "it already exists as %01d%02d%03d",
                bitmap.c_str(), WR_VAR_FXY(code), WR_VAR_FXY(res->second.code));
        return &(res->second);
    }

    auto new_entry = bitmap_table.emplace(make_pair(bitmap, _Varinfo()));
    _Varinfo& vi   = new_entry.first->second;
    varinfo::set_string(vi, code, "DATA PRESENT BITMAP", bitmap.size());
    return &vi;
}

Varinfo Tables::get_chardata(Varcode code, unsigned len) const
{
    auto res = chardata_table.find(code);
    if (res != chardata_table.end())
    {
        if (res->second.code != code)
            error_consistency::throwf(
                "Character data with length %u has been requested with varcode "
                "%01d%02d%03d but it already exists as %01d%02d%03d",
                len, WR_VAR_FXY(code), WR_VAR_FXY(res->second.code));
        return &(res->second);
    }

    auto new_entry = chardata_table.emplace(make_pair(code, _Varinfo()));
    _Varinfo& vi   = new_entry.first->second;
    varinfo::set_string(vi, code, "CHARACTER DATA", len);
    return &vi;
}

Varinfo Tables::get_unknown(Varcode code, unsigned bit_len) const
{
    auto res = unknown_table.find(bit_len);
    if (res != unknown_table.end())
    {
        if (res->second.code != code)
            error_consistency::throwf(
                "Unknown binary data %u bits long has been requested with "
                "varcode %01d%02d%03d but it already exists as %01d%02d%03d",
                bit_len, WR_VAR_FXY(code), WR_VAR_FXY(res->second.code));
        return &(res->second);
    }

    auto new_entry = unknown_table.emplace(make_pair(bit_len, _Varinfo()));
    _Varinfo& vi   = new_entry.first->second;
    varinfo::set_binary(vi, code, "UNKNOWN LOCAL DESCRIPTOR", bit_len);
    return &vi;
}

Var Tables::copy_variable(const Tables& table, const Var& var)
{
    auto code = var.code();
    switch (WR_VAR_F(code))
    {
        case 0: {
            if (table.btable->contains(code))
            {
                /// The variable uses our same tables: return a copy
                if (btable->contains_varinfo(var.info()))
                    return var;

                Varinfo local = btable->query(code);

                // The varinfos are equivalent: copy but use our table
                if (var.info()->equivalent(*local))
                    return Var(local, var);

                // TODO: lookup in unknown_table?
                // TODO: query an altered version?

                error_unimplemented::throwf(
                    "copying %01d%02d%03d with non equivalent varinfo",
                    WR_VAR_FXY(code));
            }
            else
            {
                // Try using unknown_table
                auto theirs = table.unknown_table.find(var.info()->bit_len);
                if (theirs == table.unknown_table.end())
                {
                    error_consistency::throwf(
                        "variable %01d%02d%03d is not defined in its "
                        "B table nor in its unknown entry table",
                        WR_VAR_FXY(code));
                }
                auto inserted = unknown_table.emplace(*theirs);
                if (not inserted.second)
                {
                    // We had one already defined locally
                    if (not var.info()->equivalent(inserted.first->second))
                        error_consistency::throwf(
                            "variable %01d%02d%03d defines an "
                            "unkonw entry which is alredy defined, "
                            "in a different way, in the destination "
                            "bulletin",
                            WR_VAR_FXY(code));
                }
                return Var(&(inserted.first->second), var);
            }
            break;
        }
        case 2:
            switch (WR_VAR_X(code))
            {
                case 5: {
                    // Raw character data
                    auto theirs = table.chardata_table.find(code);
                    if (theirs == table.chardata_table.end())
                    {
                        error_consistency::throwf(
                            "variable %01d%02d%03d is not defined in its "
                            "character data tables",
                            WR_VAR_FXY(code));
                    }
                    auto inserted = chardata_table.emplace(*theirs);
                    if (not inserted.second)
                    {
                        // We had one already defined locally
                        if (not var.info()->equivalent(inserted.first->second))
                            error_consistency::throwf(
                                "variable %01d%02d%03d defines a "
                                "character data entry which is alredy defined, "
                                "in a different way, in the destination "
                                "bulletin",
                                WR_VAR_FXY(code));
                    }
                    return Var(&(inserted.first->second), var);
                    break;
                }
                case 22:
                case 23: {
                    // Bitmaps
                    for (const auto& bitmap : table.bitmap_table)
                    {
                        if (var.info() == &(bitmap.second))
                        {
                            // Define the same bitmap locally
                            auto inserted = bitmap_table.emplace(bitmap);
                            if (not inserted.second)
                            {
                                // We had one already defined locally
                                if (not var.info()->equivalent(
                                        inserted.first->second))
                                    error_consistency::throwf(
                                        "variable %01d%02d%03d defines a "
                                        "bitmap which is alredy defined, in a "
                                        "different way, in the destination "
                                        "bulletin",
                                        WR_VAR_FXY(code));
                            }
                            return Var(&(inserted.first->second), var);
                        }
                    }
                    error_consistency::throwf(
                        "variable %01d%02d%03d defines a bitmap which is not "
                        "found in its bulletin tables",
                        WR_VAR_FXY(code));
                }
                break;
                default:
                    error_unimplemented::throwf("copying %01d%02d%03d",
                                                WR_VAR_FXY(code));
            }
            break;
        case 1:
        case 3:
        default:
            error_consistency::throwf(
                "Cannot copy variable with unsupported code %01d%02d%03d",
                WR_VAR_FXY(code));
    }
    // A B (variant?)
    // C05: raw character data
    // a varcode in unknown_table indexed by bit len?
}

} // namespace wreport
