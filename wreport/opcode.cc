/*
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "config.h"

#include "opcode.h"
#include "vartable.h"
#include "dtable.h"
#include "error.h"
#include <stdio.h>

using namespace std;

namespace wreport {

void Opcodes::print(FILE* out) const
{
    if (begin == end)
        fprintf(out, "(empty)");
    else
        for (unsigned i = begin; i < end; ++i)
            fprintf(out, "%d%02d%03d ", WR_VAR_F(vals[i]), WR_VAR_X(vals[i]), WR_VAR_Y(vals[i]));
}

void Opcodes::explore(opcode::Explorer& e, const DTable& dtable) const
{
    e.dtable = &dtable;
    explore(e);
}

void Opcodes::explore(opcode::Explorer& e) const
{
    for (unsigned i = 0; i < size(); ++i)
    {
        Varcode cur = (*this)[i];
        switch (WR_VAR_F(cur))
        {
            case 0: e.b_variable(cur); break;
            case 1: {
                Varcode rep_code = 0;
                if (WR_VAR_Y(cur) == 0)
                {
                    // Delayed replication
                    rep_code = (*this)[i+1];
                    ++i;
                }
                Opcodes ops = sub(i + 1, WR_VAR_X(cur));
                e.r_replication(cur, rep_code, ops);
                i += WR_VAR_X(cur);
                break;
            }
            case 2:
                // Generic notification
                e.c_modifier(cur);
                // Specific notification
                switch (WR_VAR_X(cur))
                {
                    case 1:
                        e.c_change_data_width(cur, WR_VAR_Y(cur) ? WR_VAR_Y(cur) - 128 : 0);
                        break;
                    case 2:
                        e.c_change_data_scale(cur, WR_VAR_Y(cur) ? WR_VAR_Y(cur) - 128 : 0);
                        break;
                    case 4:
                        e.c_associated_field(cur, (*this)[i + 1], WR_VAR_Y(cur));
                        ++i;
                        break;
                    case 5:
                        e.c_char_data(cur);
                        break;
                    case 6:
                        e.c_local_descriptor(cur, (*this)[i + 1], WR_VAR_Y(cur));
                        ++i;
                        break;
                    case 8:
                        e.c_char_data_override(cur, WR_VAR_Y(cur));
                        break;
                    case 22:
                        e.c_quality_information_bitmap(cur);
                        break;
                    case 23:
                        // Substituted values
                        switch (WR_VAR_Y(cur))
                        {
                            case 0:
                                e.c_substituted_value_bitmap(cur);
                                break;
                            case 255:
                                e.c_substituted_value(cur);
                                break;
                            default:
                                error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                                        WR_VAR_F(cur),
                                        WR_VAR_X(cur),
                                        WR_VAR_Y(cur));
                        }
                        break;
                        /*
                    case 24:
                        // First order statistical values
                        if (WR_VAR_Y(code) == 0)
                        {
                            used += do_r_data(ops.sub(1), var_pos);
                        } else
                            error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                                        WR_VAR_F(code),
                                        WR_VAR_X(code),
                                        WR_VAR_Y(code));
                        break;
                    default:
                        error_unimplemented::throwf("C modifier %d%02d%03d is not yet supported",
                                    WR_VAR_F(cur),
                                    WR_VAR_X(cur),
                                    WR_VAR_Y(cur));
                        */
                }
                break;
            case 3:
                e.d_group_begin(cur);
                e.dtable->query(cur).explore(e);
                e.d_group_end(cur);
                break;
            default:
                error_consistency::throwf("cannot handle opcode %01d%02d%03d",
                    WR_VAR_F(cur), WR_VAR_X(cur), WR_VAR_Y(cur));
        }
    }
}

namespace opcode {

Explorer::Explorer() : dtable(0) {}
Explorer::~Explorer() {}
void Explorer::b_variable(Varcode code) {}
void Explorer::c_modifier(Varcode code) {}
void Explorer::c_change_data_width(Varcode code, int change) {}
void Explorer::c_change_data_scale(Varcode code, int change) {}
void Explorer::c_associated_field(Varcode code, Varcode sig_code, unsigned nbits) {}
void Explorer::c_char_data(Varcode code) {}
void Explorer::c_char_data_override(Varcode code, unsigned new_length) {}
void Explorer::c_quality_information_bitmap(Varcode code) {}
void Explorer::c_substituted_value_bitmap(Varcode code) {}
void Explorer::c_substituted_value(Varcode code) {}
void Explorer::c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits) {}
void Explorer::r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops) {}
void Explorer::d_group_begin(Varcode code) {}
void Explorer::d_group_end(Varcode code) {}

Printer::Printer()
    : out(stdout), btable(0), indent(0), indent_step(2)
{
}

void Printer::print_lead(Varcode code)
{
    fprintf(out, "%*s%d%02d%03d",
            indent, "", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));
}

void Printer::b_variable(Varcode code)
{
    print_lead(code);
    if (btable)
    {
        if (btable->contains(code))
        {
            Varinfo info = btable->query(code);
            fprintf(out, " %s[%s]", info->desc, info->unit);
        } else
            fprintf(out, " (missing in B table %s)", btable->id().c_str());
    }
    putc('\n', out);
}

void Printer::c_modifier(Varcode code)
{
    print_lead(code);
    fputs(" (C modifier)\n", out);
}

void Printer::r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
{
    print_lead(code);
    unsigned group = WR_VAR_X(code);
    unsigned count = WR_VAR_Y(code);
    fprintf(out, " replicate %u descriptors", group);
    if (count)
        fprintf(out, " %u times\n", count);
    else
        fputs(" (delayed) times\n", out);
    indent += indent_step;
    ops.explore(*this);
    indent -= indent_step;
}

void Printer::d_group_begin(Varcode code)
{
    print_lead(code);
    fputs(" (group)\n", out);

    indent += indent_step;
}

void Printer::d_group_end(Varcode code)
{
    indent -= indent_step;
}

void Printer::c_change_data_width(Varcode code, int change)
{
    print_lead(code);
    fprintf(out, " change data width to %d\n", change);
}
void Printer::c_change_data_scale(Varcode code, int change)
{
    print_lead(code);
    fprintf(out, " change data scale to %d\n", change);
}
void Printer::c_associated_field(Varcode code, Varcode sig_code, unsigned nbits)
{
    print_lead(code);
    fprintf(out, " %d bits of associated field, significance code %d%02d%03d\n",
           nbits, WR_VAR_F(sig_code), WR_VAR_X(sig_code), WR_VAR_Y(sig_code));
}
void Printer::c_char_data(Varcode code)
{
    print_lead(code);
    fputs(" character data\n", out);
}
void Printer::c_char_data_override(Varcode code, unsigned new_length)
{
    print_lead(code);
    fprintf(out, " override character data length to %d\n", new_length);
}
void Printer::c_quality_information_bitmap(Varcode code)
{
    print_lead(code);
    fputs(" quality information with bitmap\n", out);
}
void Printer::c_substituted_value_bitmap(Varcode code)
{
    print_lead(code);
    fputs(" substituted values bitmap\n", out);
}
void Printer::c_substituted_value(Varcode code)
{
    print_lead(code);
    fputs(" one substituted value\n", out);
}
void Printer::c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits)
{
    print_lead(code);
    fprintf(out, " local descriptor %d%02d%03d %d bits long",
            WR_VAR_F(desc_code), WR_VAR_X(desc_code), WR_VAR_Y(desc_code), nbits);
}

}

}

/* vim:set ts=4 sw=4: */
