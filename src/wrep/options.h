/*
 * Copyright © ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This file is part of wreport. It is subject to the license terms in the
 * LICENSE file found in the top level directory of this distribution.
 *
 * No part of wreport, including this file, may be copied, modified,
 * propagated, or distributed except according to the terms contained in the
 * LICENSE file.
 */

#ifndef WREP_OPTIONS_H
#define WREP_OPTIONS_H

#include "defs.h"
#include <functional>
#include <memory>
#include <vector>
#include <wreport/varinfo.h>

namespace wreport {
struct Bulletin;
}

enum Action {
    DUMP,
    DUMP_STRUCTURE,
    DUMP_DDS,
    TRACE,
    PRINT_VARS,
    INFO,
    UNPARSABLE,
    TABLES,
    FEATURES,
    LIST_TABLES,
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
    Options() : crex(false), verbose(false), action(DUMP) {}

    void init_varcodes(const char* str);
};

#endif
