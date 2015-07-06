#ifndef WREPORT_TABLES_H
#define WREPORT_TABLES_H

#include <wreport/varinfo.h>
#include <forward_list>
#include <map>
#include <string>

namespace wreport {
struct Vartable;
struct DTable;

/**
 * Collection of BUFR/CREX tables used to work on a bulletin
 */
struct Tables
{
    /// Vartable used to lookup B table codes
    const Vartable* btable;
    /// DTable used to lookup D table codes
    const DTable* dtable;
    /// Storage for temporary Varinfos for bitmaps
    mutable std::map<std::string, _Varinfo> bitmap_table;
    /// Storage for temporary Varinfos for bitmaps and character data
    std::forward_list<_Varinfo> local_vartable;

    Tables();
    Tables(const Tables&) = delete;
    Tables(Tables&&);
    ~Tables();

    Tables& operator=(const Tables&) = delete;
    Tables& operator=(Tables&&);

    bool loaded() const;

    void clear();
    void load_bufr(int centre, int subcentre, int master_table, int local_table);
    void load_crex(int master_table_number, int edition, int table);

    // Create a single use varinfo to store the bitmap
    _Varinfo* new_entry();

    // Create a varinfo to store the bitmap
    Varinfo get_bitmap(Varcode code, const std::string& size) const;

    // Create a varinfo to store character data
    Varinfo get_chardata_entry(Varcode code, unsigned size);

    // Create a varinfo to store a C06 unknown local descriptor
    Varinfo get_unknown(Varcode code, unsigned bit_len);
};

}
#endif
