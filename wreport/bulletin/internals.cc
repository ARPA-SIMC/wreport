/*
 * wreport/bulletin/internals - Bulletin implementation helpers
 *
 * Copyright (C) 2005--2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "internals.h"
#include "var.h"
#include "subset.h"
#include "bulletin.h"
#include "notes.h"
#include <cmath>

// #define TRACE_INTERPRETER

#ifdef TRACE_INTERPRETER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {
namespace bulletin {



Parser::Parser(const Tables& tables, const Opcodes& opcodes, unsigned subset_no, const Subset& current_subset)
    : DDSInterpreter(tables, opcodes), current_subset(current_subset)
{
    TRACE("parser: start on subset %u\n", subset_no);
}

Parser::~Parser() {}

BaseParser::BaseParser(Bulletin& bulletin, unsigned subset_no)
    : Parser(bulletin.tables, bulletin.datadesc, subset_no, bulletin.subset(subset_no)), bulletin(bulletin), current_subset_no(0)
{
    current_subset_no = subset_no;
    current_var = 0;
}

Var& BaseParser::get_var()
{
    Var& res = get_var(current_var);
    ++current_var;
    return res;
}

Var& BaseParser::get_var(unsigned var_pos) const
{
    unsigned max_var = current_subset.size();
    if (var_pos >= max_var)
        error_consistency::throwf("requested variable #%u out of a maximum of %u in subset %u",
                var_pos, max_var, current_subset_no);
    return bulletin.subsets[current_subset_no][var_pos];
}

void BaseParser::define_bitmap(Varcode rep_code, Varcode delayed_code, const Opcodes& ops)
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    bitmaps.define(var, current_subset);
}

}
}
