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
#include <vector>
#include <cstdio>

namespace wreport {
namespace bulletin {

struct DDSPrinter : public ConstBaseDDSExecutor
{
    std::vector<Varcode> stack;
    FILE* out;

    DDSPrinter(const Bulletin& b, FILE* out);
    virtual ~DDSPrinter();

    void print_context(Varinfo info, unsigned var_pos);
    void print_context(Varcode code, unsigned var_pos);

    virtual void start_subset(unsigned subset_no, const Subset& current_subset);
    virtual void push_dcode(Varcode code);
    virtual void pop_dcode();
    virtual void encode_attr(Varinfo info, unsigned var_pos, Varcode attr_code);
    virtual void encode_var(Varinfo info);
    virtual Var encode_semantic_var(Varinfo info);
    virtual unsigned encode_bitmap_repetition_count(Varinfo info, const Var& bitmap);
    virtual void encode_bitmap(const Var& bitmap);
    virtual void encode_char_data(Varcode code);
};

}
}

#endif
