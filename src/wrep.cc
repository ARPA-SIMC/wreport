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
#include "config.h"
#include "wrep/decoder.h"
#include "wrep/options.h"
#include "wrep/read_raw.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <wreport/bulletin/dds-scanfeatures.h>
#include <wreport/error.h>
#include <wreport/internals/tabledir.h>
#include <wreport/notes.h>

#ifdef HAS_GETOPT_LONG
#include <getopt.h>
#endif

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

// Print information about the library
void do_info()
{
    printf("Tables search paths (tried in order):\n");
    printf("Extra tables directory: %s (env var WREPORT_EXTRA_TABLES)\n",
           getenv("WREPORT_EXTRA_TABLES"));
    printf("System tables directory: %s (env var WREPORT_TABLES)\n",
           getenv("WREPORT_TABLES"));
    printf("Compiled-in default tables directory: %s\n", TABLE_DIR);
}

// List directories where tables are looked up
void do_list_tabledirs() { wreport::tabledir::Tabledirs::get().print(stdout); }

/// Trace decoding of a file
struct PrintTrace : public DecodeFull
{
    void handle(wreport::Bulletin& b) override
    {
        fprintf(stdout, "%s:%ld:ok\n", b.fname.c_str(), (long)b.offset);
    }
};

/// Dump message contents
struct PrintContents : public DecodeFull
{
    /// Dump the contents of a message
    void handle(wreport::Bulletin& b) override { b.print(stdout); }
};

/// Dump message structure
struct PrintStructure : public DecodeHeader
{
    /// Dump the contents of a message, with structure
    void handle(wreport::Bulletin& b) override { b.print_structured(stdout); }
};

/// Dump data descriptor sections
struct PrintDDS : public DecodeHeader
{
    /// Dump the contents of the Data Descriptor Section a message
    void handle(wreport::Bulletin& b) override { b.print_datadesc(stdout); }
};

/// Print selected variables from decoded messages
struct PrintVars : public DecodeFull
{
    const std::vector<wreport::Varcode>& codes;

    explicit PrintVars(const std::vector<wreport::Varcode>& codes)
        : codes(codes)
    {
    }

    const wreport::Var* find_varcode(const wreport::Subset& subset,
                                     wreport::Varcode code)
    {
        for (size_t i = 0; i < subset.size(); ++i)
            if (subset[i].code() == code)
                return &subset[i];
        return NULL;
    }

    void handle(wreport::Bulletin& b) override
    {
        for (size_t sset = 0; sset < b.subsets.size(); ++sset)
        {
            printf("%s:%zu", b.fname.c_str(), sset + 1);
            for (size_t i = 0; i < codes.size(); ++i)
            {
                const wreport::Var* var =
                    find_varcode(b.subsets[sset], codes[i]);
                if (var)
                {
                    std::string formatted = var->format();
                    printf("\t%s", formatted.c_str());
                }
            }
            putchar('\n');
        }
    }
};

struct CopyUnparsable : public RawMessageHandler
{
    unsigned unparsed = 0;

    void handle_raw_bufr(const std::string& raw_data, const char* fname,
                         long offset) override
    {
        try
        {
            wreport::BufrBulletin::decode(raw_data, fname, offset);
        }
        catch (std::exception& e)
        {
            fprintf(stderr, "%s\n", e.what());
            fwrite(raw_data.data(), raw_data.size(), 1, stdout);
            ++unparsed;
        }
    }

    void handle_raw_crex(const std::string& raw_data, const char* fname,
                         long offset) override
    {
        try
        {
            wreport::CrexBulletin::decode(raw_data, fname, offset);
        }
        catch (std::exception& e)
        {
            fprintf(stderr, "%s\n", e.what());
            fwrite(raw_data.data(), raw_data.size(), 1, stdout);
            ++unparsed;
        }
    }
};

/// Print tables used by messages
struct PrintTables : public DecodeHeader
{
    bool header_printed = false;

    /// Dump the contents of the Data Descriptor Section a message
    void handle(wreport::Bulletin& b) override
    {
        if (const auto* m = dynamic_cast<const wreport::BufrBulletin*>(&b))
        {
            if (!header_printed)
            {
                printf("%-*s\tOffset\tCentre\tSubc.\tMaster\tLocal\n",
                       (int)b.fname.size(), "Filename");
                header_printed = true;
            }
            printf("%s\t%zd\t%d\t%d\t%d\t%d\n", b.fname.c_str(), b.offset,
                   m->originating_centre, m->originating_subcentre,
                   m->master_table_version_number,
                   m->master_table_version_number_local);
        }
        else if (const auto* m = dynamic_cast<const wreport::CrexBulletin*>(&b))
        {
            if (!header_printed)
            {
                printf("Filename\tOffset\tMaster\tEdition\tTable\n");
                header_printed = true;
            }
            printf("%s\t%zd\t%d\t%d\t%d\n", b.fname.c_str(), b.offset,
                   m->master_table_number, m->edition_number,
                   m->master_table_version_number);
        }
        else
        {
            printf("%s\t%zd\tunknown message type\n", b.fname.c_str(),
                   b.offset);
        }
    }
};

/// Print what decoder features are used by messages
struct PrintFeatures : public DecodeHeader
{
    /// Dump the contents of the Data Descriptor Section a message
    void handle(wreport::Bulletin& b) override
    {
        wreport::bulletin::ScanFeatures scan(b.tables, b.datadesc);
        scan.run();
        printf("%s:%zd:", b.fname.c_str(), b.offset);
        bool first = true;
        for (const auto& f : scan.features)
            if (first)
            {
                printf("%s", f.c_str());
                first = false;
            }
            else
                printf(",%s", f.c_str());
        putchar('\n');
    }
};

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
        wreport::notes::set_target(std::cerr);

    // Choose the right handler for the action requested by the user
    std::unique_ptr<RawMessageHandler> decoder;
    switch (options.action)
    {
        case HELP:        do_help(stdout); return 0;
        case INFO:        do_info(); return 0;
        case LIST_TABLES: do_list_tabledirs(); return 0;
        case TRACE:       decoder = std::make_unique<PrintTrace>(); break;
        case DUMP:        decoder = std::make_unique<PrintContents>(); break;
        case DUMP_STRUCTURE:
            decoder = std::make_unique<PrintStructure>();
            break;
        case DUMP_DDS: decoder = std::make_unique<PrintDDS>(); break;
        case PRINT_VARS:
            decoder = std::make_unique<PrintVars>(options.varcodes);
            break;
        case UNPARSABLE: decoder = std::make_unique<CopyUnparsable>(); break;
        case TABLES:     decoder = std::make_unique<PrintTables>(); break;
        case FEATURES:   decoder = std::make_unique<PrintFeatures>(); break;
    }

    // Ensure we have some file to process
    if (optind >= argc)
    {
        do_usage(stderr);
        return 1;
    }

    // Pick the reader we want
    std::function<void(const char* filename, RawMessageHandler& handler)>
        reader = read_bufr_raw;
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
                reader(fname, *decoder);
            }
            catch (std::exception& e)
            {
                fprintf(stderr, "%s:%s\n", fname, e.what());
            }
        }

        decoder->done();
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    return 0;
}
