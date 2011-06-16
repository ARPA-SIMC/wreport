/*
 * wreport/notes - Collect notes about unusual processing
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

#ifndef WREPORT_NOTES_H
#define WREPORT_NOTES_H

#include <iosfwd>

#ifndef WREPORT_PRINTF_ATTRS
#define WREPORT_PRINTF_ATTRS(a, b) __attribute__ ((format(printf, a, b)))
#endif

namespace wreport {

/**
 * Collect notes about unusual things that happen during processing.
 *
 * By default notes are discarded, unless set_target() is called or a
 * notes::Collect object is instantiated to direct notes where needed.
 */
namespace notes {

/// Set the target stream where the notes are sent
void set_target(std::ostream& out);

std::ostream* get_target();

/// Return true if there is any target to which notes are sent
bool logs() throw ();

/// Output stream to send notes to
std::ostream& log() throw ();

/// printf-style logging
void logf(const char* fmt, ...) WREPORT_PRINTF_ATTRS(1, 2);

/**
 * RAII way to temporarily set a notes target.
 *
 * Notes are sent to the given output stream for as long as the object is in
 * scope.
 */
struct Collect
{
    std::ostream* old;

    Collect(std::ostream& out)
    {
        old = get_target();
        set_target(out);
    }
    ~Collect()
    {
        set_target(*old);
    }
};

}

}

/* vim:set ts=4 sw=4: */
#endif
