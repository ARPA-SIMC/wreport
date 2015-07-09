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

UncompressedEncoder::UncompressedEncoder(const Bulletin& bulletin, unsigned subset_no)
    : DDSInterpreter(bulletin.tables, bulletin.datadesc), current_subset(bulletin.subset(subset_no))
{
}

UncompressedEncoder::~UncompressedEncoder()
{
}

const Var& UncompressedEncoder::peek_var()
{
    return get_var(current_var);
}

const Var& UncompressedEncoder::get_var()
{
    return get_var(current_var++);
}

const Var& UncompressedEncoder::get_var(unsigned pos) const
{
    unsigned max_var = current_subset.size();
    if (pos >= max_var)
        error_consistency::throwf("cannot return variable #%u out of a maximum of %u", pos, max_var);
    return current_subset[pos];
}

void UncompressedEncoder::define_bitmap(unsigned bitmap_size)
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    bitmaps.define(var, current_subset);
}


UncompressedDecoder::UncompressedDecoder(Bulletin& bulletin, unsigned subset_no)
    : DDSInterpreter(bulletin.tables, bulletin.datadesc), output_subset(bulletin.obtain_subset(subset_no))
{
}

UncompressedDecoder::~UncompressedDecoder()
{
}


CompressedDecoder::CompressedDecoder(Bulletin& bulletin)
    : DDSInterpreter(bulletin.tables, bulletin.datadesc), output_bulletin(bulletin)
{
    TRACE("parser: start on compressed bulletin\n");
}

CompressedDecoder::~CompressedDecoder() {}

}
}
