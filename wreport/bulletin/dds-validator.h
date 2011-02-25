/*
 * bulletin/dds-validator - Validate variables of a bulletin against its data
 *                          descriptor section
 *
 * Copyright (C) 2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#include <vector>

namespace wreport {
namespace bulletin {

struct DDSValidator : public ConstBaseDDSExecutor
{
    bool is_crex;

    DDSValidator(const Bulletin& b);

    const Var& get_var(unsigned var_pos) const;
    void check_fits(Varinfo info, const Var& var);

    virtual void encode_attr(Varinfo info, unsigned var_pos, Varcode attr_code);
    virtual void encode_var(Varinfo info, unsigned var_pos);
    virtual unsigned encode_repetition_count(Varinfo info, unsigned var_pos);
    virtual unsigned encode_bitmap_repetition_count(Varinfo info, const Var& bitmap);
    virtual void encode_bitmap(const Var& bitmap);
    virtual void encode_char_data(Varcode code, unsigned var_pos);
};

}
}

#endif
