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

#include "notes.h"

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#if 0
#include <config.h>

#include <stdio.h>	/* vasprintf */
#include <stdlib.h>	/* free */
#include <string.h>	/* strerror */
#include <stdarg.h> /* va_start, va_end */
#include <regex.h>	/* regerror */
#include <errno.h>
#include <assert.h>

#include <execinfo.h>

#include <sstream>
#endif

using namespace std;

namespace wreport {
namespace notes {

// streambuf that discards all data
struct null_streambuf : public std::streambuf
{
    int overflow(int c) { return c; }
};


__thread ostream* target = 0;
null_streambuf* null_sb = 0;
ostream* null_stream = 0;

void set_target(std::ostream& out)
{
    target = &out;
}

std::ostream* get_target()
{
    return target;
}

bool logs() throw ()
{
    return target != 0;
}

std::ostream& log() throw ()
{
    // If there is a target, use it
    if (target) return *target;

    // If there is no target, return an ostream that discards all data
    if (!null_sb)
    {
        null_sb = new null_streambuf;
        null_stream = new ostream(null_sb);
    }
    return *null_stream;
}

void logf(const char* fmt, ...)
{
    if (!target) return;

    char *c;
    va_list ap;
    va_start( ap, fmt );
    vasprintf( &c, fmt, ap );
    (*target) << c;
    free( c );
}

} // namespace notes
} // namespace wreport

/* vim:set ts=4 sw=4: */
