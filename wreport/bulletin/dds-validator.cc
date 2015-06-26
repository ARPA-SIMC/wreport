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

#include "wreport/bulletin/dds-validator.h"

using namespace std;

namespace wreport {
namespace bulletin {

DDSValidator::DDSValidator(const Bulletin& b)
    : ConstBaseVisitor(b)
{
    is_crex = dynamic_cast<const CrexBulletin*>(&b) != NULL;
}

void DDSValidator::check_fits(Varinfo info, const Var& var)
{
    if (var.code() != info->code)
        error_consistency::throwf("input variable %d%02d%03d differs from expected variable %d%02d%03d",
                WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()),
                WR_VAR_F(info->code), WR_VAR_X(info->code), WR_VAR_Y(info->code));

    if (var.value() == NULL)
        ;
    else if (info->is_string())
        ;
    else {
        unsigned encoded;
        if (is_crex)
            encoded = info->encode_decimal(var.enqd());
        else
            encoded = info->encode_binary(var.enqd());
        if (!is_crex)
        {
            if (encoded >= (1u<<info->bit_len))
                error_consistency::throwf("value %f (%u) does not fit in %d bits", var.enqd(), encoded, info->bit_len);
        }
    }
}

void DDSValidator::do_attr(Varinfo info, unsigned var_pos, Varcode attr_code)
{
    const Var& var = get_var(var_pos);
    if (const Var* a = var.enqa(attr_code))
        check_fits(info, *a);
}

void DDSValidator::do_var(Varinfo info)
{
    const Var& var = get_var();

    if (associated_field.bit_count)
    {
        const Var& var = get_var(current_var);
        /*const Var* att =*/ associated_field.get_attribute(var);
    }

    check_fits(info, var);
}

const Var& DDSValidator::do_semantic_var(Varinfo info)
{
    const Var& var = get_var();
    check_fits(info, var);
    return var;
}

void DDSValidator::do_char_data(Varcode code)
{
    const Var& var = get_var();
    if (var.code() != code)
        error_consistency::throwf("input variable %d%02d%03d differs from expected variable %d%02d%03d",
                WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()),
                WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));
}

}
}

/* vim:set ts=4 sw=4: */
