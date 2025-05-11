/*
 * Future optimizations for dba_vartable can make use of string tables to store
 * varinfo descriptions and units instead of long fixed-length records.
 * However, the string table cannot grow dynamically or it will invalidate the
 * existing string pointers.
 */

#include "vartable.h"
#include "error.h"
#include "internals/tabledir.h"
#include "internals/vartable.h"
#include <map>

using namespace std;

namespace wreport {

Vartable::~Vartable() {}

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
    static std::map<std::filesystem::path, vartable::Bufr*>* tables = 0;
    if (!tables)
        tables = new std::map<std::filesystem::path, vartable::Bufr*>;

    // Return it from cache if we have it
    auto i = tables->find(pathname);
    if (i != tables->end())
        return i->second;

    // Else, instantiate it
    return (*tables)[pathname] = new vartable::Bufr(pathname);
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
    static std::map<std::filesystem::path, vartable::Crex*>* tables = 0;
    if (!tables)
        tables = new std::map<std::filesystem::path, vartable::Crex*>;

    // Return it from cache if we have it
    auto i = tables->find(pathname);
    if (i != tables->end())
        return i->second;

    // Else, instantiate it
    return (*tables)[pathname] = new vartable::Crex(pathname);
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
