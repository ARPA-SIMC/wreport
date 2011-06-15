/*
 * wreport/bulletin/internals - Bulletin implementation helpers
 *
 * Copyright (C) 2005--2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

namespace wreport {
namespace bulletin {

Bitmap::Bitmap() : bitmap(0) {}
Bitmap::~Bitmap() {}

void Bitmap::reset()
{
    bitmap = 0;
    old_anchor = 0;
    refs.clear();
    iter = refs.rend();
}

void Bitmap::init(const Var& bitmap, const Subset& subset, unsigned anchor)
{
    this->bitmap = &bitmap;
    refs.clear();

    // From the specs it looks like bitmaps refer to all data that precedes
    // the C operator that defines or uses the bitmap, but from the data
    // samples that we have it look like when multiple bitmaps are present,
    // they always refer to the same set of variables. For this reason we
    // remember the first anchor point that we see and always refer the
    // other bitmaps that we see to it.
    if (old_anchor)
        anchor = old_anchor;
    else
        old_anchor = anchor;

    unsigned b_cur = bitmap.info()->len;
    unsigned s_cur = anchor;
    if (b_cur == 0) throw error_consistency("data present bitmap has length 0");
    if (s_cur == 0) throw error_consistency("data present bitmap is anchored at start of subset");

    while (true)
    {
        --b_cur;
        --s_cur;
        while (WR_VAR_F(subset[s_cur].code()) != 0)
        {
            if (s_cur == 0) throw error_consistency("bitmap refers to variables before the start of the subset");
            --s_cur;
        }

        if (bitmap.value()[b_cur] == '+')
            refs.push_back(s_cur);

        if (b_cur == 0)
            break;
        if (s_cur == 0)
            throw error_consistency("bitmap refers to variables before the start of the subset");
    }

    iter = refs.rbegin();
}

bool Bitmap::eob() const { return iter == refs.rend(); }
unsigned Bitmap::next() { unsigned res = *iter; ++iter; return res; }


}
}
