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

#include "options.h"
#include <cstring>

using namespace wreport;

void Options::init_varcodes(const char* str)
{
    varcodes.clear();
    while (str && *str && strlen(str) >= 6)
    {
        varcodes.push_back(varcode_parse(str));
        str = strchr(str, ',');
        if (str && *str)
            ++str;
    }
}
