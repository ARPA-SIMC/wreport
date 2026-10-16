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

#include "read_raw.h"

using namespace wreport;

void read_bufr_raw(const char* fname, RawMessageHandler& handler)
{
    // Open the input file
    FILE* in = fopen(fname, "rb");
    if (in == NULL)
        error_system::throwf("opening BUFR file %s", fname);

    // Use a generic try/catch block to ensure we always close the input file,
    // even in case of errors
    try
    {
        // String used to hold raw data read from the input file
        std::string raw_data;

        // (optional) offset of the start of the BUFR message read, which we
        // pass to the decoder to have nicer error messages
        off_t offset;

        // Read all BUFR data in the input file, one message at a time. Extra
        // data before and after each BUFR message is skipped.
        // fname and offset are optional and we pass them just to have nicer
        // error messages.
        while (BufrBulletin::read(in, raw_data, fname, &offset))
            handler.handle_raw_bufr(raw_data, fname, offset);

        // Cleanup
        fclose(in);
    }
    catch (...)
    {
        fclose(in);
        throw;
    }
}

void read_crex_raw(const char* fname, RawMessageHandler& handler)
{
    /*
     * Note that the code is the same as with reading BUFRs, except for the
     * text open mode, and for using CrexBulletin::read instead of
     * BufrBulletin::read
     */

    // Open the input file
    FILE* in = fopen(fname, "rt");
    if (in == NULL)
        error_system::throwf("opening CREX file %s", fname);

    // Use a generic try/catch block to ensure we always close the input file,
    // even in case of errors
    try
    {
        // String used to hold raw data read from the input file
        std::string raw_data;

        // (optional) offset of the start of the CREX message read, which we
        // pass to the decoder to have nicer error messages
        off_t offset;

        // Read all CREX data in the input file, one message at a time. Extra
        // data before and after each CREX message is skipped.
        // fname and offset are optional and we pass them just to have nicer
        // error messages.
        while (CrexBulletin::read(in, raw_data, fname, &offset))
            handler.handle_raw_crex(raw_data, fname, offset);

        // Cleanup
        fclose(in);
    }
    catch (...)
    {
        fclose(in);
        throw;
    }
}
