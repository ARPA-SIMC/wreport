#ifndef WREPORT_TABLES_H
#define WREPORT_TABLES_H

#include <wreport/varinfo.h>
#include <forward_list>
#include <map>
#include <string>

namespace wreport {
struct BufrTableID;
struct CrexTableID;
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
    /// Storage for temporary Varinfos for arbitrary character data
    mutable std::map<unsigned, _Varinfo> chardata_table;
    /// Storage for temporary Varinfos for C06 unknown local descriptors
    mutable std::map<unsigned, _Varinfo> unknown_table;

    Tables();
    Tables(const Tables&) = delete;
    Tables(Tables&&);
    ~Tables();

    Tables& operator=(const Tables&) = delete;
    Tables& operator=(Tables&&);

    /// Check if the B and D tables have been loaded
    bool loaded() const;

    /// Clear btable, datable and all locally generated Varinfos
    void clear();

    /// Load BUFR B and D tables
    void load_bufr(const BufrTableID& id);

    /// Load CREX B and D tables
    void load_crex(const CrexTableID& id);

    // Create a varinfo to store the bitmap
    Varinfo get_bitmap(Varcode code, const std::string& bitmap) const;

    // Create a varinfo to store character data
    Varinfo get_chardata(Varcode code, unsigned len) const;

    // Create a varinfo to store a C06 unknown local descriptor
    Varinfo get_unknown(Varcode code, unsigned bit_len) const;
};

}
#endif
