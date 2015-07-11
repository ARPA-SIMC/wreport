/*
 * options - wrep runtime configuration
 *
 * Copyright (C) 2011--2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#include <memory>

namespace wreport {
struct Bulletin;
}

enum Action {
    DUMP,
    DUMP_STRUCTURE,
    DUMP_DDS,
    PRINT_VARS,
    INFO,
    UNPARSABLE,
    TABLES,
    FEATURES,
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

struct RawHandler
{
    virtual ~RawHandler() {}
    virtual void handle_raw_bufr(const std::string& data, const char* fname, long offset) = 0;
    virtual void handle_raw_crex(const std::string& data, const char* fname, long offset) = 0;
    virtual void done() {}
};

// Interface for classes that process bulletins, parsing only message headers
struct BulletinHeadHandler : public RawHandler
{
    virtual ~BulletinHeadHandler() {}

    /// Decode and handle the decoded bulletin
    virtual void handle_raw_bufr(const std::string& raw_data, const char* fname, long offset);

    /// Decode and handle the decoded bulletin
    virtual void handle_raw_crex(const std::string& raw_data, const char* fname, long offset);

    virtual void handle(wreport::Bulletin&) = 0;
};

// Interface for classes that process bulletins, parsing full messages
struct BulletinFullHandler : public RawHandler
{
    virtual ~BulletinFullHandler() {}

    /// Decode and handle the decoded bulletin
    virtual void handle_raw_bufr(const std::string& raw_data, const char* fname, long offset);

    /// Decode and handle the decoded bulletin
    virtual void handle_raw_crex(const std::string& raw_data, const char* fname, long offset);

    virtual void handle(wreport::Bulletin&) = 0;
};

// Signature for functions that read bulletins from a file
typedef void (*bulletin_reader)(const Options&, const char*, RawHandler& handler);

#endif
