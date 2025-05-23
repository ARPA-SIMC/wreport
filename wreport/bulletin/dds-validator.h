/*
 * bulletin/dds-validator - Validate variables of a bulletin against its data
 *                          descriptor section
 *
 * Copyright (C) 2011--2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#ifndef WREPORT_BULLETIN_DDS_VALIDATOR_H
#define WREPORT_BULLETIN_DDS_VALIDATOR_H

#include <vector>
#include <wreport/bulletin.h>
#include <wreport/bulletin/internals.h>

namespace wreport {
namespace bulletin {

/**
 * Interpreter that checks if the data that has been added to the bulletin
 * subsets can actualy be encoded matching the bulletin Data Descriptor
 * Section.
 */
class DDSValidator : public UncompressedEncoder
{
    bool is_crex;
    void check_fits(Varinfo info, const Var& var);
    void check_attr(Varinfo info, unsigned var_pos);

public:
    /**
     * Create a new DDS validator
     *
     * @param b
     *   Reference to the bulletin being visited
     */
    DDSValidator(const Bulletin& b, unsigned subset_idx);

    // void define_bitmap(unsigned bitmap_size) override;
    void define_substituted_value(unsigned pos) override;
    void define_attribute(Varinfo info, unsigned pos) override;
    void define_raw_character_data(Varcode code) override;
    void define_c03_refval_override(Varcode code) override;
    void encode_var(Varinfo info, const Var& var) override;
};

} // namespace bulletin
} // namespace wreport

#endif
