#include "config.h"
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <wreport/bulletin.h>
#include <wreport/error.h>
#include <wreport/notes.h>
#include <wreport/tests.h>

#ifdef HAS_GETOPT_LONG
#include <getopt.h>
#endif

using namespace wreport;
using namespace std;
namespace fs = std::filesystem;

namespace {

bool verbose = false;

void do_usage(FILE* out)
{
    fputs("Usage: wrep-maketestjson dir1 [dir2, ...]\n", out);
}

void do_help(FILE* out)
{
    do_usage(out);
    fputs("Create json versions of BUFR and CREX files, used for testing\n"
          "Options:\n"
          "  -v,--verbose        verbose operation\n"
          "  -h,--help           print this help message\n"
#ifndef HAS_GETOPT_LONG
          "NOTE: long options are not supported on this system\n"
#endif
          ,
          out);
}

class File
{
protected:
    fs::path path;
    FILE* fd = nullptr;

public:
    explicit File(const fs::path& path) : path(path) {}
    ~File()
    {
        if (fd)
            fclose(fd);
    }
    File(const File&)            = delete;
    File(File&&)                 = delete;
    File& operator=(const File&) = delete;
    File& operator=(File&&)      = delete;
};

class Outfile : public File
{
public:
    using File::File;
    void write(const std::string& str)
    {
        if (fwrite(str.data(), str.size(), 1, fd) < 1)
            error_system::throwf("cannot write %zu bytes to %s", str.size(),
                                 path.c_str());
    }

    void add(const Bulletin& bulletin)
    {
        if (!fd)
        {
            fd = fopen(path.c_str(), "wb");
            if (fd == NULL)
                error_system::throwf("cannot open file %s", path.c_str());
        }
        std::stringstream buf;
        tests::dump_jsonl(bulletin, buf);
        write(buf.str());
    }
};

class Infile : public File
{
public:
    explicit Infile(const fs::path& path) : File(path)
    {
        fd = fopen(path.c_str(), "rb");
        if (fd == NULL)
            error_system::throwf("cannot open file %s", path.c_str());
    }

    void process(Outfile& out)
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
        while (BufrBulletin::read(fd, raw_data, path.c_str(), &offset))
        {
            try
            {
                // Decode the raw data. fname and offset are optional and we
                // pass them just to have nicer error messages
                auto bulletin =
                    BufrBulletin::decode(raw_data, path.c_str(), offset);

                // Do something with the decoded information
                out.add(*bulletin);
            }
            catch (std::exception& e)
            {
                fprintf(stderr, "%s:%ld:%s\n", path.c_str(), offset, e.what());
            }
        }
    }
};

void process_dir(const std::filesystem::path& path)
{
    notes::logf("%s: processing\n", path.c_str());

    for (const auto& entry : fs::recursive_directory_iterator(path))
    {
        if (entry.path().extension() != ".bufr" and
            entry.path().extension() != ".crex")
            continue;

        std::filesystem::path outpath = entry.path();
        outpath.replace_extension(outpath.extension().native() + ".json");
        notes::logf("%s → %s\n", entry.path().c_str(), outpath.c_str());

        Infile infile(entry.path());
        Outfile outfile(outpath);

        try
        {
            infile.process(outfile);
        }
        catch (std::exception& e)
        {
            fprintf(stderr, "%s: %s\n", entry.path().c_str(), e.what());
        }
    }
}

void process_file(const std::filesystem::path& path)
{
    notes::logf("%s: processing\n", path.c_str());

    Infile infile(path);
    Outfile outfile("/dev/stdout");

    try
    {
        infile.process(outfile);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s: %s\n", path.c_str(), e.what());
    }
}

} // namespace

int main(int argc, char* argv[])
{
#ifdef HAS_GETOPT_LONG
    static struct option long_options[] = {
        /* These options set a flag. */
        {"verbose", no_argument, NULL, 'v'},
        {"help",    no_argument, NULL, 'h'},
        {0,         0,           0,    0  }
    };
#endif

    // Parse command line options
    while (1)
    {
        // getopt_long stores the option index here
        int option_index = 0;

#ifdef HAS_GETOPT_LONG
        int c = getopt_long(argc, argv, "vh:", long_options, &option_index);
#else
        int c = getopt(argc, argv, "vh:");
#endif

        // Detect the end of the options
        if (c == -1)
            break;

        switch (c)
        {
            case 'v': verbose = true; break;
            case 'h':
                do_help(stdout);
                return 0;
                break;
            default:
                fprintf(stderr, "unknown option character %c (%d)\n", c, c);
                do_help(stderr);
                return 1;
        }
    }

    // Print out processing remarks if verbose
    if (verbose)
        notes::set_target(cerr);

    // Ensure we have some file to process
    if (optind >= argc)
    {
        do_usage(stderr);
        return 1;
    }

    try
    {
        while (optind < argc)
        {
            fs::path path(argv[optind++]);
            if (fs::is_directory(path))
                process_dir(path);
            else
                process_file(path);
        }
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    return 0;
}
