/*
 * input - BUFR and CREX input examples
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

#include <wreport/bulletin.h>
#include "options.h"

using namespace wreport;

// Read all BUFR messages from a file
void read_bufr_raw(const Options& opts, const char* fname, RawHandler& handler)
{
    // Open the input file
    FILE* in = fopen(fname, "rb");
    if (in == NULL)
        error_system::throwf("opening file %s", fname);

    // Use a generic try/catch block to ensure we always close the input file,
    // even in case of errors
    try {
        // String used to hold raw data read from the input file
        string raw_data;

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
    } catch (...) {
        fclose(in);
        throw;
    }
}

/*
 * Read all CREX messages from a file
 *
 * Note that the code is basically the same as with reading BUFRs, with only
 * two changes:
 *  - it uses a CrexBulletin instead of a BufrBulletin
 *  - it uses CrexBulletin::read instead of BufrBulletin::read
 */
void read_crex_raw(const Options& opts, const char* fname, RawHandler& handler)
{
    // Open the input file
    FILE* in = fopen(fname, "rt");
    if (in == NULL)
        error_system::throwf("opening file %s", fname);

    // Use a generic try/catch block to ensure we always close the input file,
    // even in case of errors
    try {
        // Create a CREX bulletin
        unique_ptr<Bulletin> bulletin(CrexBulletin::create());

        // String used to hold raw data read from the input file
        string raw_data;

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
    } catch (...) {
        fclose(in);
        throw;
    }
}
