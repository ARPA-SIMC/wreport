/*
 * makebuoy - Example on how to create a buoy BUFR message
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
#include <cstring>

/// Create a simple buoy BUFR message
void do_makebuoy()
{
    // Create a blank BUFR bulletin
    unique_ptr<BufrBulletin> bulletin(BufrBulletin::create());


    // * Fill up metadata

    // BUFR edition number
    bulletin->edition = 4;

    // Master table number is 0 by default
    // bulletin->master_table_number = 0;

    // Data category information
    bulletin->type = 1;
    bulletin->subtype = 21;
    bulletin->localsubtype = 255;

    // Reference time
    bulletin->rep_year = 2011;
    bulletin->rep_month = 10;
    bulletin->rep_day = 3;
    bulletin->rep_hour = 17;
    bulletin->rep_minute = 0;
    bulletin->rep_second = 0;

    // Originating centre information
    bulletin->centre = 98; // ECMWF
    bulletin->subcentre = 0;

    // B table version used by the message
    bulletin->master_table = 14;
    bulletin->local_table = 0;

    // Compression is still not supported when encoding BUFR
    bulletin->compression = 0;

    // Update sequence number is 0 by default
    // bulletin->update_sequence_number = 0;

    // Optional section
    bulletin->optional_section_length = 4;
    bulletin->optional_section = new char[4];
    memcpy(bulletin->optional_section, "test", 4);


    // * Fill up data descriptor section

    // There is only one descriptor in this case, but one can push_back as many
    // as one likes
    bulletin->datadesc.push_back(WR_VAR(3, 8, 3)); // D08003


    // * Fill up the data section

    // Load encoding tables
    bulletin->load_tables();

    // Create the first (and only) subset
    Subset& s = bulletin->obtain_subset(0);

    // Add variables to the subset, as dictated by the data descriptor section
    s.store_variable_i(WR_VAR(0,  1,  5), 65602);
    s.store_variable_d(WR_VAR(0,  1, 12), 12.0);
    s.store_variable_d(WR_VAR(0,  1, 13), 0.2);
    s.store_variable_i(WR_VAR(0,  2,  1), 0);
    s.store_variable_i(WR_VAR(0,  4,  1), 2011);
    s.store_variable_i(WR_VAR(0,  4,  2), 10);
    s.store_variable_i(WR_VAR(0,  4,  3), 3);
    s.store_variable_i(WR_VAR(0,  4,  4), 17);
    s.store_variable_i(WR_VAR(0,  4,  5), 0);
    s.store_variable_d(WR_VAR(0,  5,  2), 59.03);
    s.store_variable_d(WR_VAR(0,  6,  2), -2.99);
    s.store_variable_d(WR_VAR(0, 10,  4), 99520.0);
    s.store_variable_d(WR_VAR(0, 10, 51), 99520.0);
    s.store_variable_d(WR_VAR(0, 10, 61), 310);
    s.store_variable_i(WR_VAR(0, 10, 63), 7);
    s.store_variable_undef(WR_VAR(0, 11, 11));
    s.store_variable_undef(WR_VAR(0, 11, 12));
    s.store_variable_d(WR_VAR(0, 12,  4), 278.5);
    s.store_variable_undef(WR_VAR(0, 12,  6));
    s.store_variable_undef(WR_VAR(0, 13,  3));
    s.store_variable_undef(WR_VAR(0, 20,  1));
    s.store_variable_undef(WR_VAR(0, 20,  3));
    s.store_variable_undef(WR_VAR(0, 20,  4));
    s.store_variable_undef(WR_VAR(0, 20,  5));
    s.store_variable_undef(WR_VAR(0, 20, 10));
    s.store_variable_undef(WR_VAR(0,  8,  2));
    s.store_variable_undef(WR_VAR(0, 20, 11));
    s.store_variable_undef(WR_VAR(0, 20, 13));
    s.store_variable_undef(WR_VAR(0, 20, 12));
    s.store_variable_undef(WR_VAR(0, 20, 12));
    s.store_variable_undef(WR_VAR(0, 20, 12));
    s.store_variable_d(WR_VAR(0, 22, 42), 280.8);


    // Encode the BUFR
    string encoded;
    bulletin->encode(encoded);

    // Output the BUFR
    if (fwrite(encoded.data(), encoded.size(), 1, stdout) != 1)
        perror("cannot write BUFR to standard output");
}
