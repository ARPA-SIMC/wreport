#include "config.h"
#include "options.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <wreport/bulletin.h>
#include <wreport/error.h>
#include <wreport/internals/tabledir.h>
#include <wreport/notes.h>

using namespace wreport;
using namespace std;

/**
 * Read a BUFR and iterate its contents, printing only errors if they happen.
 *
 * This can be used as a test program for AFL (see
 * http://lcamtuf.coredump.cx/afl/ )
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fputs("Usage: afl-test file\n", stderr);
        return 1;
    }

    try
    {
        // Open the input file
        const char* fname = argv[1];
        FILE* in          = fopen(fname, "rb");
        if (in == NULL)
            error_system::throwf("opening file %s", fname);

        // Use a generic try/catch block to ensure we always close the input
        // file, even in case of errors
        try
        {
            // String used to hold raw data read from the input file
            string raw_data;

            // (optional) offset of the start of the BUFR message read, which we
            // pass to the decoder to have nicer error messages
            off_t offset;

            // Read all BUFR data in the input file, one message at a time.
            // Extra data before and after each BUFR message is skipped. fname
            // and offset are optional and we pass them just to have nicer error
            // messages.
            while (BufrBulletin::read(in, raw_data, fname, &offset))
            {
                // Decode the raw data. fname and offset are optional and we
                // pass them just to have nicer error messages
                auto bulletin =
                    BufrBulletin::decode_header(raw_data, fname, offset);

                for (const auto& subset : bulletin->subsets)
                    for (const auto& var : subset)
                        var.enqs();
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
    catch (std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    return 0;
}
