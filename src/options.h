/*
 * options - wrep runtime configuration
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

#ifndef WREP_OPTIONS_H
#define WREP_OPTIONS_H

#include <wreport/varinfo.h>
#include <vector>

namespace wreport {
struct Bulletin;
}

enum Action {
    DUMP,
    DUMP_STRUCTURE,
    DUMP_DDS,
    PRINT_VARS,
    INFO,
    HELP,
};

// wrep runtime configuration
struct Options
{
    // Read CREX instead of BUFR
    bool crex;
    // Verbose processing
    bool verbose;

    // Action requested
    enum Action action;

    // List of varcodes selected by the user
    std::vector<wreport::Varcode> varcodes;

    // Initialise with default values
    Options()
        : crex(false), verbose(false), action(DUMP)
    {
    }

    void init_varcodes(const char* str);
};

// Interface for classes that process bulletins
struct BulletinHandler
{
    virtual ~BulletinHandler() {}
    virtual void handle(const wreport::Bulletin&) = 0;
    virtual void done() {}
};

// Signature for functions that read bulletins from a file
typedef void (*bulletin_reader)(const Options&, const char*, BulletinHandler& handler, bool header_only);


#endif
