#include "dtable.h"
#include "config.h"
#include "error.h"
#include "internals/tabledir.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

using namespace std;

namespace wreport {

DTable::~DTable() {}

namespace {

/**
 * D-table entry, with index pointers to beginning and end of each D table
 * sequence
 */
struct Entry
{
    /// Varcode to be expanded
    Varcode code;
    /// Position in the main table where the expansion begins
    unsigned begin;
    /// Position in the main table one past where the expansion ends
    unsigned end;

    Entry(Varcode code, unsigned begin, unsigned end)
        : code(code), begin(begin), end(end) {}
};

struct fd_closer
{
    FILE* fd;
    fd_closer(FILE* fd) : fd(fd) {}
    ~fd_closer() { fclose(fd); }
};

struct DTableBase : public DTable
{
    std::string m_pathname;

    /**
     * One single table with the concatenation of all the expansion
     * varcodes
     */
    std::vector<Varcode> varcodes;

    /**
     * Expansion entries with pointers inside \a varcodes
     */
    std::vector<Entry> entries;

    DTableBase(const std::string& pathname)
        : m_pathname(pathname)
    {
        FILE* in = fopen(pathname.c_str(), "rt");
        if (in == NULL) error_system::throwf("opening D table file %s", pathname.c_str());
        fd_closer closer(in); // Close `in' on exit

        Varcode dcode = 0; // D code of the last code block
        unsigned begin = 0; // Begin of the last code block
        int nentries_check = 0; // Length of sequence advertised at the beginning of the code block

        char line[200];
        int line_no = 0;
        while (fgets(line, 200, in) != NULL)
        {
            line_no++;
            if (strlen(line) < 18)
                throw error_parse(pathname.c_str(), line_no, "line too short");

            // Start of a new D entry
            if (line[1] == 'D' || line[1] == '3')
            {
                int last_count = varcodes.size() - begin;
                if (last_count != nentries_check)
                    error_parse::throwf(pathname.c_str(), line_no, "advertised number of expansion items (%d) does not match the number of items found (%d)", nentries_check, last_count);

                nentries_check = strtol(line + 7, 0, 10);
                if (nentries_check < 1)
                    throw error_parse(pathname.c_str(), line_no, "less than one entry advertised in the expansion");

                if (!varcodes.empty())
                    entries.push_back(Entry(dcode, begin, varcodes.size()));
                begin = varcodes.size();
                dcode = descriptor_code(line + 1);
                varcodes.push_back(descriptor_code(line + 11));

                // fprintf(stderr, "Debug: D%05d %d entries\n", dcode, nentries);
            }
            else if (strncmp(line, "           ", 11) == 0)
            {
                int last_count;
                // Check that there has been at least one entry filed before
                if (varcodes.empty())
                    throw error_parse(pathname.c_str(), line_no, "expansion line found before the first entry");
                // Check that we are not appending too many entries
                last_count = varcodes.size() - begin;
                if (last_count == nentries_check)
                    error_parse::throwf(pathname.c_str(), line_no, "too many entries found (expected %d)", nentries_check);

                // Finally append the code
                varcodes.push_back(descriptor_code(line + 11));
            }
            else
                error_parse::throwf(pathname.c_str(), line_no, "unrecognized line: \"%s\"", line);
        }

        // Check that we actually read something
        if (varcodes.empty())
            throw error_parse(pathname.c_str(), line_no, "no entries found in the file");
        else
            entries.push_back(Entry(dcode, begin, varcodes.size()));

        // Check that the last entry is complete
        int last_count = varcodes.size() - begin;
        if (last_count != nentries_check)
            error_parse::throwf(pathname.c_str(), line_no, "advertised number of expansion items (%d) does not match the number of items found (%d)", nentries_check, last_count);
    }

    ~DTableBase()
    {
    }

    std::string pathname() const override { return m_pathname; }

    Opcodes query(Varcode var) const override
    {
        int begin, end;

        // Binary search the entry
        begin = -1, end = entries.size();
        while (end - begin > 1)
        {
            int cur = (end + begin) / 2;
            if (entries[cur].code > var)
                end = cur;
            else
                begin = cur;
        }
        if (begin == -1 || entries[begin].code != var)
            error_notfound::throwf(
                    "missing D table expansion for variable %d%02d%03d in file %s",
                    WR_VAR_F(var), WR_VAR_X(var), WR_VAR_Y(var), m_pathname.c_str());
        else
            return Opcodes(varcodes, entries[begin].begin, entries[begin].end);
    }
};

}

const DTable* DTable::load_bufr(const std::string& pathname)
{
    static std::map<string, DTable*>* tables = 0;
    if (!tables) tables = new std::map<string, DTable*>;

    // Return it from cache if we have it
    auto i = tables->find(pathname);
    if (i != tables->end())
        return i->second;

    // Else, instantiate it
    return (*tables)[pathname] = new DTableBase(pathname);
}

const DTable* DTable::load_crex(const std::string& pathname)
{
    static std::map<string, DTable*>* tables = 0;
    if (!tables) tables = new std::map<string, DTable*>;

    // Return it from cache if we have it
    auto i = tables->find(pathname);
    if (i != tables->end())
        return i->second;

    // Else, instantiate it
    return (*tables)[pathname] = new DTableBase(pathname);
}

}
