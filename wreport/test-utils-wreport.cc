/*
 * wreport/test-utils-wreport - Unit test utilities
 *
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "test-utils-wreport.h"

#include <wibble/sys/fs.h>
#include <wibble/string.h>

#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

using namespace std;
using namespace wibble;

namespace wreport {
namespace tests {

std::string datafile(const std::string& fname)
{
    const char* testdatadirenv = getenv("WREPORT_TESTDATA");
    std::string testdatadir = testdatadirenv ? testdatadirenv : ".";
    return str::joinpath(testdatadir, fname);
}

std::string slurpfile(const std::string& name)
{
	string fname = datafile(name);
	string res;

	FILE* fd = fopen(fname.c_str(), "rb");
	if (fd == NULL)
		error_system::throwf("opening %s", fname.c_str());
		
	/* Read the entire file contents */
	while (!feof(fd))
	{
		char c;
		if (fread(&c, 1, 1, fd) == 1)
			res += c;
	}

	fclose(fd);

	return res;
}

std::vector<std::string> all_test_files(const std::string& encoding)
{
    const char* testdatadirenv = getenv("WREPORT_TESTDATA");
    std::string testdatadir = testdatadirenv ? testdatadirenv : ".";
    testdatadir = str::joinpath(testdatadir, encoding);

    vector<string> res;
    sys::fs::Directory dir(testdatadir);
    for (sys::fs::Directory::const_iterator i = dir.begin(); i != dir.end(); ++i)
        if (str::endsWith(*i, "."+encoding))
            res.push_back(str::joinpath(encoding, *i));
    return res;
}

void track_bulletin(const Bulletin& b, const char* tag, const char* fname)
{
    string dumpfname = "/tmp/bulletin-" + str::basename(fname) + "-" + tag;
    FILE* out = fopen(dumpfname.c_str(), "wt");
    fprintf(out, "Contents of %s %s:\n", fname, tag);
    b.print(out);
    fprintf(out, "\nData descriptor section of %s %s:\n", fname, tag);
    b.print_datadesc(out);
    fprintf(out, "\nStructure of %s %s:\n", fname, tag);
    b.print_structured(out);
    fclose(out);
    fprintf(stderr, "%s %s dumped as %s\n", fname, tag, dumpfname.c_str());
}

} // namespace tests
} // namespace wreport

// vim:set ts=4 sw=4:
