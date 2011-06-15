/*
 * bulletin/dds-printer - Print a DDS using the interpreter
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

#ifndef WREPORT_BULLETIN_DDS_PRINTER_H
#define WREPORT_BULLETIN_DDS_PRINTER_H

#include <wreport/bulletin.h>
#include <wreport/bulletin/internals.h>
#include <vector>
#include <cstdio>

namespace wreport {
namespace bulletin {

struct DDSPrinter : public ConstBaseVisitor
{
    std::vector<Varcode> stack;
    FILE* out;

    DDSPrinter(const Bulletin& b, FILE* out);
    virtual ~DDSPrinter();

    void print_context(Varinfo info, unsigned var_pos);
    void print_context(Varcode code, unsigned var_pos);

    virtual void do_start_subset(unsigned subset_no, const Subset& current_subset);
    virtual void do_attr(Varinfo info, unsigned var_pos, Varcode attr_code);
    virtual void do_var(Varinfo info);
    virtual const Var& do_semantic_var(Varinfo info);
    virtual const Var* do_bitmap(Varcode code, Varcode delayed_code, const Opcodes& ops);
    virtual void do_char_data(Varcode code);
    void do_associated_field(unsigned bit_count, unsigned significance);

    virtual void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops);
    virtual void d_group_begin(Varcode code);
    virtual void d_group_end(Varcode code);
};

}
}

#endif
