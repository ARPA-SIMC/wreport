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
#include <cstdio>
#include <iostream>
#include <string>
#include <wreport/bulletin.h>
#include <wreport/error.h>
#include <wreport/varinfo.h>

namespace {

/**
 * Dump the contents of a Subset.
 *
 * BUFR messages contain one or more subsets (CREX only one), each with a
 * sequence of data variables as described by the messsage's data descriptor
 * section.
 */
void dump_subset(const wreport::Subset& subset, unsigned subset_number)
{
    // Each subset contains one or more variables
    unsigned var_number = 0;
    for (const auto& var : subset)
    {
        ++var_number;
        // Format the variable code
        auto code_formatted = wreport::varcode_format(var.code());
        // Format the variable value
        auto formatted      = var.format("-");
        // Print the value and its metadata
        printf("%03u:%03u:%s: %10s %s (%s)\n", subset_number, var_number,
               code_formatted.c_str(), formatted.c_str(), var.info()->desc,
               var.info()->unit);
    }
}

/*
 * Dump the contents of a Bulletin.
 *
 * A Bulletin is the interface for decoded BUFR or CREX messages
 */
void dump_bulletin(const wreport::Bulletin& bulletin)
{
    // Each bulletin contains one or more subsets of data
    unsigned subset_number = 0;
    for (const auto& subset : bulletin.subsets)
    {
        ++subset_number;
        dump_subset(subset, subset_number);
    }
}

/**
 * Show how to dump the contents of a BUFR file
 */
void dump_bufr(const char* fname)
{
    // Open the input file
    FILE* in = fopen(fname, "rb");
    if (in == NULL)
        wreport::error_system::throwf("opening BUFR file %s", fname);

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
        // fname and offset are optional, and help with error messages
        while (wreport::BufrBulletin::read(in, raw_data, fname, &offset))
        {
            // Decode the raw data. fname and offset are still optional
            auto bulletin =
                wreport::BufrBulletin::decode(raw_data, fname, offset);

            dump_bulletin(*bulletin);
        }

        // Cleanup
        fclose(in);
    }
    catch (...)
    {
        fclose(in);
        throw;
    }
}

} // namespace

int main(int argc, char* argv[])
{
    // Dump all input files
    for (int i = 1; i < argc; ++i)
        dump_bufr(argv[i]);
    return 0;
}
