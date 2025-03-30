#include "config.h"
#include "options.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <wreport/error.h>
#include <wreport/internals/tabledir.h>
#include <wreport/notes.h>

#ifdef HAS_GETOPT_LONG
#include <getopt.h>
#endif

using namespace wreport;
using namespace std;

// These are split in separate files so they can be loaded as example code by
// the documentation
#include "info.cc"
#include "input.cc"
#include "iterate.cc"
#include "output.cc"
#include "unparsable.cc"

namespace {

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
        "  -t,--trace          print a verbose trace of the decoding process\n"
        "  -s,--structure      dump message contents and structure\n"
        "  -D,--dds            dump message Data Descriptor Section\n"
        "  -p,--print=VARCODES for each input bulletin, print the given\n"
        "                      comma-separated list of varcodes (e.g.\n"
        "                      \"B01019,B05001,B06001\")\n"
        "  -U,--unparsable     output a copy of the messages that cannot be "
        "parsed\n"
        "  -T,--tables         print the version of tables used by each "
        "bulletin\n"
        "  -F,--features       print the features used by each bulletin\n"
        "  -L,--list-tables    print a list of all tables found\n"
#ifndef HAS_GETOPT_LONG
        "NOTE: long options are not supported on this system\n"
#endif
        ,
        out);
}

} // namespace

int main(int argc, char* argv[])
{
#ifdef HAS_GETOPT_LONG
    static struct option long_options[] = {
        /* These options set a flag. */
        {"crex",        no_argument,       NULL, 'c'},
        {"dump",        no_argument,       NULL, 'd'},
        {"trace",       no_argument,       NULL, 't'},
        {"structure",   no_argument,       NULL, 's'},
        {"dds",         no_argument,       NULL, 'D'},
        {"print",       required_argument, NULL, 'p'},
        {"info",        no_argument,       NULL, 'i'},
        {"verbose",     no_argument,       NULL, 'v'},
        {"unparsable",  no_argument,       NULL, 'U'},
        {"tables",      no_argument,       NULL, 'T'},
        {"features",    no_argument,       NULL, 'F'},
        {"list-tables", no_argument,       NULL, 'L'},
        {"help",        no_argument,       NULL, 'h'},
        {0,             0,                 0,    0  }
    };
#endif

    // Parse command line options
    Options options;
    while (1)
    {
        // getopt_long stores the option index here
        int option_index = 0;

#ifdef HAS_GETOPT_LONG
        int c = getopt_long(argc, argv, "cdsDpivtUTFLh:", long_options,
                            &option_index);
#else
        int c = getopt(argc, argv, "cdsDpivtUTFLh:");
#endif

        // Detect the end of the options
        if (c == -1)
            break;

        switch (c)
        {
            case 'c': options.crex = true; break;
            case 'v': options.verbose = true; break;
            case 'd': options.action = DUMP; break;
            case 't': options.action = TRACE; break;
            case 's': options.action = DUMP_STRUCTURE; break;
            case 'D': options.action = DUMP_DDS; break;
            case 'p':
                options.action = PRINT_VARS;
                options.init_varcodes(optarg);
                break;
            case 'i': options.action = INFO; break;
            case 'U': options.action = UNPARSABLE; break;
            case 'T': options.action = TABLES; break;
            case 'F': options.action = FEATURES; break;
            case 'L': options.action = LIST_TABLES; break;
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
        case HELP:           do_help(stdout); return 0;
        case INFO:           do_info(); return 0;
        case LIST_TABLES:    tabledir::Tabledirs::get().print(stdout); return 0;
        case TRACE:          handler.reset(new PrintTrace(stdout)); break;
        case DUMP:           handler.reset(new PrintContents(stdout)); break;
        case DUMP_STRUCTURE: handler.reset(new PrintStructure(stdout)); break;
        case DUMP_DDS:       handler.reset(new PrintDDS(stdout)); break;
        case PRINT_VARS:     handler.reset(new PrintVars(options.varcodes)); break;
        case UNPARSABLE:
            handler.reset(new CopyUnparsable(stdout, stderr));
            break;
        case TABLES:   handler.reset(new PrintTables(stdout)); break;
        case FEATURES: handler.reset(new PrintFeatures(stdout)); break;
    }

    // Ensure we have some file to process
    if (optind >= argc)
    {
        do_usage(stderr);
        return 1;
    }

    // Pick the reader we want
    bulletin_reader reader = read_bufr_raw;
    if (options.crex)
        reader = read_crex_raw;

    try
    {
        while (optind < argc)
        {
            if (options.verbose)
                fprintf(stderr, "Reading from %s\n", argv[optind]);
            const char* fname = argv[optind++];
            try
            {
                reader(options, fname, *handler);
            }
            catch (std::exception& e)
            {
                fprintf(stderr, "%s:%s\n", fname, e.what());
            }
        }

        handler->done();
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    return 0;
}
