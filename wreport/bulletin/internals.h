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

#ifndef WREPORT_BULLETIN_INTERNALS_H
#define WREPORT_BULLETIN_INTERNALS_H

#include <vector>

namespace wreport {
struct Var;
struct Subset;

namespace bulletin {

/**
 * Associate a Data Present Bitmap to decoded variables in a subset
 */
struct Bitmap
{
    const Var* bitmap;
    std::vector<unsigned> refs;
    std::vector<unsigned>::const_reverse_iterator iter;
    unsigned old_anchor;

    Bitmap();
    ~Bitmap();

    void reset();

    /**
     * Initialise the bitmap handler
     *
     * @param bitmap
     *   The bitmap
     * @param subset
     *   The subset to which the bitmap refers
     * @param anchor
     *   The index to the first element after the end of the bitmap (usually
     *   the C operator that defines or uses the bitmap)
     */
    void init(const Var& bitmap, const Subset& subset, unsigned anchor);

    bool eob() const;
    unsigned next();
};

}
}

#endif
