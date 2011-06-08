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
    for (unsigned i = 0; i < size(); ++i)
    {
        Varcode cur = (*this)[i];
        switch (WR_VAR_F(cur))
        {
            case 0: e.b_variable(cur); break;
            case 1:
                if (WR_VAR_Y(cur) == 0)
                {
                    // Delayed replication
                    e.r_replication_begin(cur, (*this)[i+1]);
                    ++i;
                } else
                    e.r_replication_begin(cur, 0);
                sub(i + 1, WR_VAR_X(cur)).explore(e, dtable);
                e.r_replication_end(cur);
                i += WR_VAR_X(cur);
                break;
            case 2: e.c_modifier(cur); break;
            case 3:
                e.d_group_begin(cur);
                dtable.query(cur).explore(e, dtable);
                e.d_group_end(cur);
                break;
            default:
                error_consistency::throwf("cannot handle opcode %01d%02d%03d",
                    WR_VAR_F(cur), WR_VAR_X(cur), WR_VAR_Y(cur));
        }
    }
}

namespace opcode {

Explorer::~Explorer() {}
void Explorer::b_variable(Varcode code) {}
void Explorer::c_modifier(Varcode code) {}
void Explorer::r_replication_begin(Varcode code, Varcode delayed_code) {}
void Explorer::r_replication_end(Varcode code) {}
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

void Printer::r_replication_begin(Varcode code, Varcode delayed_code)
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
}

void Printer::r_replication_end(Varcode code)
{
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

}

}

/* vim:set ts=4 sw=4: */
