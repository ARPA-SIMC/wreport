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

#include <wreport/bulletin.h>
#include <wreport/bulletin/internals.h>
#include <vector>

namespace wreport {
namespace bulletin {

/**
 * bulletin::Visitor that checks if the data that has been added to the
 * bulletin subsets can actualy be encoded matching the bulletin Data
 * Descriptor Section.
 */
class DDSValidator : public BaseParser
{
    bool is_crex;
    void check_fits(Varinfo info, const Var& var);

public:
    /**
     * Create a new DDS validator
     *
     * @param b
     *   Reference to the bulletin being visited
     */
    DDSValidator(Bulletin& b, unsigned subset_idx);

    void do_attr(Varinfo info, unsigned var_pos, Varcode attr_code) override;
    void define_variable(Varinfo info) override;
    const Var& define_semantic_variable(Varinfo info) override;
    void do_char_data(Varcode code) override;
};

}
}

#endif
