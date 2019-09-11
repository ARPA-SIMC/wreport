#ifndef WREPORT_TABLEINFO_H
#define WREPORT_TABLEINFO_H

#include <cstdint>
#include <cstdio>
#include <wreport/fwd.h>

namespace wreport {

/**
 * Identifying information for one distinct instance of BUFR tables.
 */
class BufrTableID
{
public:
    uint16_t originating_centre = 0xffff;
    uint16_t originating_subcentre = 0xffff;
    uint8_t master_table_number = 0xff;
    uint8_t master_table_version_number = 0xff;
    uint8_t master_table_version_number_local = 0xff;

    BufrTableID() {}
    BufrTableID(
            uint16_t originating_centre, uint16_t originating_subcentre,
            uint8_t master_table_number, uint8_t master_table_version_number, uint8_t master_table_version_number_local)
        : originating_centre(originating_centre), originating_subcentre(originating_subcentre),
          master_table_number(master_table_number), master_table_version_number(master_table_version_number), master_table_version_number_local(master_table_version_number_local) {}

    bool operator<(const BufrTableID& o) const;

    bool is_acceptable_replacement(const BufrTableID& id) const;
    bool is_acceptable_replacement(const CrexTableID& id) const;
    int closest_match(const BufrTableID& first, const BufrTableID& second) const;
    int closest_match(const CrexTableID& first, const CrexTableID& second) const;
    int closest_match(const BufrTableID& first, const CrexTableID& second) const;

    void print(FILE* out) const;
};

/**
 * Identifying information for one distinct instance of CREX tables.
 */
class CrexTableID
{
public:
    uint8_t edition_number = 0xff;
    uint16_t originating_centre = 0xffff;
    uint16_t originating_subcentre = 0xffff;
    uint8_t master_table_number = 0xff;
    uint8_t master_table_version_number = 0xff;
    uint8_t master_table_version_number_bufr = 0xff;
    uint8_t master_table_version_number_local = 0xff;

    CrexTableID() {}
    CrexTableID(
            uint8_t edition_number,
            uint16_t originating_centre, uint16_t originating_subcentre,
            uint8_t master_table_number,
            uint8_t master_table_version_number,
            uint8_t master_table_version_number_bufr,
            uint8_t master_table_version_number_local
            )
        : edition_number(edition_number),
          originating_centre(originating_centre), originating_subcentre(originating_subcentre),
          master_table_number(master_table_number),
          master_table_version_number(master_table_version_number),
          master_table_version_number_bufr(master_table_version_number_bufr),
          master_table_version_number_local(master_table_version_number_local) {}

    bool operator<(const CrexTableID& o) const;

    bool is_acceptable_replacement(const BufrTableID& id) const;
    bool is_acceptable_replacement(const CrexTableID& id) const;
    int closest_match(const BufrTableID& first, const BufrTableID& second) const;
    int closest_match(const CrexTableID& first, const CrexTableID& second) const;
    int closest_match(const BufrTableID& first, const CrexTableID& second) const;

    void print(FILE* out) const;
};

}
#endif

