/*
 * wrep - command line tool to work with weather bulletins
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

#include "config.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <wreport/error.h>
#include <wreport/notes.h>

#ifdef HAS_GETOPT_LONG
#include <getopt.h>
#endif

using namespace wreport;
using namespace std;

// These are split in separate files so they can be loaded as example code by
// the documentation
#include "makebuoy.cc"

void do_usage(FILE* out) { fputs("Usage: examples [options]\n", out); }

void do_help(FILE* out)
{
    do_usage(out);
    fputs("Run wreport examples\n"
          "Options:\n"
          "  -v,--verbose        verbose operation\n"
          "  -h,--help           print this help message\n"
          "  -B,--makebuoy       generate a buoy BUFR message\n"
#ifndef HAS_GETOPT_LONG
          "NOTE: long options are not supported on this system\n"
#endif
          ,
          out);
}

int main(int argc, char* argv[])
{
#ifdef HAS_GETOPT_LONG
    static struct option long_options[] = {
  /* These options set a flag. */
        {"makebuoy", no_argument, NULL, 'B'},
        {"verbose",  no_argument, NULL, 'v'},
        {"help",     no_argument, NULL, 'h'},
        {0,          0,           0,    0  }
    };
#endif

    // Parse command line options
    bool verbose                   = false;
    enum { HELP, MAKEBUOY } action = HELP;
    while (1)
    {
        // getopt_long stores the option index here
        int option_index = 0;

#ifdef HAS_GETOPT_LONG
        int c = getopt_long(argc, argv, "vh", long_options, &option_index);
#else
        int c = getopt(argc, argv, "vh");
#endif

        // Detect the end of the options
        if (c == -1)
            break;

        switch (c)
        {
            case 'v': verbose = true; break;
            case 'h': action = HELP; break;
            case 'B': action = MAKEBUOY; break;
            default:
                fprintf(stderr, "unknown option character %c (%d)\n", c, c);
                do_help(stderr);
                return 1;
        }
    }

    // Print out processing remarks if verbose
    if (verbose)
        notes::set_target(cerr);

    // Run the handler for the action requested by the user
    try
    {
        switch (action)
        {
            case HELP:     do_help(stdout); return 0;
            case MAKEBUOY: do_makebuoy(); return 0;
        }
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    return 0;
}
