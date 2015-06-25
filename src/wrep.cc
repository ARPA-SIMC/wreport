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

#include <wreport/error.h>
#include <wreport/notes.h>
#include "options.h"
#include <string>
#include <iostream>
#include <cstdio>
#include "config.h"

#ifdef HAS_GETOPT_LONG
#include <getopt.h>
#endif

using namespace wreport;
using namespace std;

// These are split in separate files so they can be loaded as example code by
// the documentation
#include "info.cc"
#include "input.cc"
#include "output.cc"
#include "iterate.cc"
#include "unparsable.cc"

void do_usage(FILE* out)
{
    fputs("Usage: wrep [options] file1 [file2 [file3 ..]]\n", out);
}

void do_help(FILE* out)
{
    do_usage(out);
    fputs(
        "Simple weather bulletin handling functions\n"
        "Options:\n"
        "  -v,--verbose        verbose operation\n"
        "  -c,--crex           read CREX instead of BUFR\n"
        "  -h,--help           print this help message\n"
        "  -i,--info           print configuration information\n"
        "  -d,--dump           (default) dump message contents\n"
        "  -s,--structure      dump message contents and structure\n"
        "  -D,--dds            dump message Data Descriptor Section\n"
        "  -p,--print=VARCODES for each input bulletin, print the given\n"
        "                      comma-separated list of varcodes (e.g.\n"
        "                      \"B01019,B05001,B06001\")\n"
        "  -U,--unparsable     output a copy of the messages that cannot be parsed\n"
        "  -T,--tables         print the version of tables used by each bulletin\n"
#ifndef HAS_GETOPT_LONG
        "NOTE: long options are not supported on this system\n"
#endif
    , out);
}

int main(int argc, char* argv[])
{
#ifdef HAS_GETOPT_LONG
    static struct option long_options[] =
    {
        /* These options set a flag. */
        {"crex",       no_argument,       NULL, 'c'},
        {"dump",       no_argument,       NULL, 'd'},
        {"structure",  no_argument,       NULL, 's'},
        {"dds",        no_argument,       NULL, 'D'},
        {"print",      required_argument, NULL, 'p'},
        {"info",       no_argument,       NULL, 'i'},
        {"verbose",    no_argument,       NULL, 'v'},
        {"unparsable", no_argument,       NULL, 'U'},
        {"tables",     no_argument,       NULL, 'T'},
        {"help",       no_argument,       NULL, 'h'},
        {0, 0, 0, 0}
    };
#endif

    // Parse command line options
    Options options;
    while (1)
    {
        // getopt_long stores the option index here
        int option_index = 0;

#ifdef HAS_GETOPT_LONG
        int c = getopt_long(argc, argv, "cvUdsDihp:",
                long_options, &option_index);
#else
        int c = getopt(argc, argv, "cvUdsDihp:");
#endif

        // Detect the end of the options
        if (c == -1)
            break;

        switch (c)
        {
            case 'c': options.crex = true; break;
            case 'v': options.verbose = true; break;
            case 'd': options.action = DUMP; break;
            case 's': options.action = DUMP_STRUCTURE; break;
            case 'D': options.action = DUMP_DDS; break;
            case 'p':
                options.action = PRINT_VARS;
                options.init_varcodes(optarg);
                break;
            case 'i': options.action = INFO; break;
            case 'U': options.action = UNPARSABLE; break;
            case 'T': options.action = TABLES; break;
            case 'h': options.action = HELP; break;
            default:
                fprintf(stderr, "unknown option character %c (%d)\n", c, c);
                do_help(stderr);
                return 1;
        }
    }

    // Print out processing remarks if verbose
    if (options.verbose)
        notes::set_target(cerr);

    // Choose the right handler for the action requested by the user
    unique_ptr<RawHandler> handler;
    switch (options.action)
    {
        case HELP:
            do_help(stdout);
            return 0;
        case INFO:
            do_info();
            return 0;
        case DUMP: handler.reset(new PrintContents(stdout)); break;
        case DUMP_STRUCTURE: handler.reset(new PrintStructure(stdout)); break;
        case DUMP_DDS: handler.reset(new PrintDDS(stdout)); break;
        case PRINT_VARS: handler.reset(new PrintVars(options.varcodes)); break;
        case UNPARSABLE: handler.reset(new CopyUnparsable(stdout, stderr)); break;
        case TABLES: handler.reset(new PrintTables(stdout)); break;
    }

    // Ensure we have some file to process
    if (optind >= argc)
    {
        do_usage(stderr);
        return 1;
    }

    // Pick the reader we want
    bulletin_reader reader = read_bufr_raw;
    if (options.crex) reader = read_crex_raw;

    try {
        while (optind < argc)
        {
            if (options.verbose) fprintf(stderr, "Reading from %s\n", argv[optind]);
            reader(options, argv[optind++], *handler);
        }

        handler->done();
    } catch (std::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    return 0;
}
