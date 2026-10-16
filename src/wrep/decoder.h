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

#ifndef WREP_DECODER_H
#define WREP_DECODER_H

#include "defs.h"

struct Decoder : public RawMessageHandler
{
    /// Process a decoded BUFR
    virtual void handle_bufr(wreport::Bulletin& bulletin) { handle(bulletin); }

    /// Process a decoded CREX
    virtual void handle_crex(wreport::Bulletin& bulletin) { handle(bulletin); }

    /// Process a decoded bulletin regardless of encoding format
    virtual void handle(wreport::Bulletin&) = 0;
};

// Interface for classes that process bulletins, parsing only message headers
struct DecodeHeader : public Decoder
{
    /// Decode and handle the decoded bulletin
    void handle_raw_bufr(const std::string& raw_data, const char* fname,
                         long offset) override;

    /// Decode and handle the decoded bulletin
    void handle_raw_crex(const std::string& raw_data, const char* fname,
                         long offset) override;
};

// Interface for classes that process bulletins, parsing full messages
struct DecodeFull : public Decoder
{
    /// Decode and handle the decoded bulletin
    void handle_raw_bufr(const std::string& raw_data, const char* fname,
                         long offset) override;

    /// Decode and handle the decoded bulletin
    void handle_raw_crex(const std::string& raw_data, const char* fname,
                         long offset) override;
};

#endif
