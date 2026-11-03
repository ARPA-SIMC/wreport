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

#ifndef WREP_DEFS_H
#define WREP_DEFS_H

#include <functional>
#include <string>
#include <wreport/bulletin.h>

/**
 * Interface for processing raw messages as they get read
 */
struct RawMessageHandler
{
    virtual ~RawMessageHandler() {}

    /// Process a buffer as a BUFR message
    virtual void handle_raw_bufr(const std::string& data, const char* fname,
                                 long offset) = 0;

    /// Process a buffer as a CREX message
    virtual void handle_raw_crex(const std::string& data, const char* fname,
                                 long offset) = 0;

    /// Signal the end of processing
    virtual void done() {}
};

#endif
