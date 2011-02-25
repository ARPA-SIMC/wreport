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

#include "wreport/bulletin/dds-validator.h"

using namespace std;

namespace wreport {
namespace bulletin {

DDSValidator::DDSValidator(const Bulletin& b) : b(b), current_subset(0) {}

const Var& DDSValidator::get_var(unsigned var_pos) const
{
    unsigned max_var = b.subset(current_subset).size();
    if (var_pos >= max_var)
        error_consistency::throwf("requested variable #%u out of a maximum of %u in subset %u",
                var_pos, max_var, current_subset);
    return b.subset(current_subset)[var_pos];
}

void DDSValidator::check_fits(Varinfo info, const Var& var)
{
    if (var.code() != info->var)
        error_consistency::throwf("input variable %d%02d%03d differs from expected variable %d%02d%03d",
                WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()),
                WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var));

    if (var.value() == NULL)
        ;
    else if (info->is_string())
        ;
    else {
        unsigned encoded = info->encode_int(var.enqd());
        if (encoded >= (1u<<info->bit_len))
            error_consistency::throwf("value %f does not fit in %d bits", var.enqd(), info->bit_len);
    }
}

void DDSValidator::start_subset(unsigned subset_no)
{
    if (subset_no >= b.subsets.size())
        error_consistency::throwf("requested subset #%u out of a maximum of %zd", subset_no, b.subsets.size());
    current_subset = subset_no;
}

void DDSValidator::encode_attr(Varinfo info, unsigned var_pos, Varcode attr_code)
{
    const Var& var = get_var(var_pos);
    if (const Var* a = var.enqa(attr_code))
        check_fits(info, *a);
}

void DDSValidator::encode_var(Varinfo info, unsigned var_pos)
{
    const Var& var = get_var(var_pos);
    check_fits(info, var);
}

unsigned DDSValidator::encode_repetition_count(Varinfo info, unsigned var_pos)
{
    const Var& var = get_var(var_pos);
    check_fits(info, var);
    return var.enqi();
}

unsigned DDSValidator::encode_bitmap_repetition_count(Varinfo info, const Var& bitmap)
{
    return bitmap.info()->len;
}

void DDSValidator::encode_bitmap(const Var& bitmap)
{
}

const Var* DDSValidator::get_bitmap(unsigned var_pos)
{
    const Var& var = get_var(var_pos);
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                var_pos, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    return &var;
}

}
}

/* vim:set ts=4 sw=4: */
