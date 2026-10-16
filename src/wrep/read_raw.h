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
#ifndef WREP_READ_RAW_H
#define WREP_READ_RAW_H

#include "defs.h"
#include "options.h"
#include <wreport/bulletin.h>

// Open a BUFR file and read each separate message as a binary string
void read_bufr_raw(const char* fname, RawMessageHandler& handler);

// Open a CREX file and read each separate message as a binary string
void read_crex_raw(const char* fname, RawMessageHandler& handler);

#endif
