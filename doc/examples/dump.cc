/*
 * dump - Example simple BUFR/CREX dump program
 *
 * Copyright (C) 2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

// Messages are read in a Bulletin class
#include <wreport/bulletin.h>
#include <string>
#include <cstdio>
#include <strings.h>

using namespace std;

/**
 * Little enum we use to represent the choice of encoding from the command line
 */
enum Encoding { BUFR, CREX };

/**
 * Dump a BUFR or CREX file.
 *
 * We use a single template for both bulletin types, as they have exactly the
 * same API.
 */
template<typename TYPE>
void dump(FILE* in, const char* fname)
{
	try {
		// Each bulletin type has a read function that reads the next
		// message from any input stream into a string buffer
		string buf;
		while (TYPE::read(in, buf, fname))
		{
			// Decode the raw data creating a new bulletin
			unique_ptr<TYPE> bulletin = TYPE::decode(buf);
			// Dump it all to stdout
			bulletin->print(stdout);
		}
	} catch (std::exception& e) {
		// Errors are reported through exceptions. All exceptions used
		// by wreport are descendents of std::exception
		fprintf(stderr, "Error reading %s: %s\n", fname, e.what());
	}
}

/// Select the right bulletin type according to the requested format
void dump(Encoding type, FILE* in, const char* fname)
{
	switch (type)
	{
		case BUFR: dump<wreport::BufrBulletin>(in, fname); break;
		case CREX: dump<wreport::CrexBulletin>(in, fname); break;
	}
}

const char* argv0 = "dump";

/// Print program usage
void usage()
{
	fprintf(stderr, "Usage: %s {BUFR|CREX} [file1 [file2...]]\n", argv0);
	fprintf(stderr, "Dumps bufr or crex file contents to standard output\n");
}

int main(int argc, const char* argv[])
{
	argv0 = argv[0];

	if (argc == 1)
	{
		usage();
		return 1;
	}

	// Detect what encoding type is requested
	Encoding type;
	if (strcasecmp(argv[1], "BUFR") == 0)
		type = BUFR;
	else if (strcasecmp(argv[1], "CREX") == 0)
		type = CREX;
	else
	{
		usage();
		return 1;
	}

	if (argc == 2)
	{
		// Dump stdin if no input file was given
		printf("Dumping standard input...\n");
		dump(type, stdin, "standard input");
	} else {
		// Else dump all input files
		for (int i = 2; i < argc; ++i)
		{
			printf("Dumping file %s...\n", argv[i]);
			FILE* in = fopen(argv[i], "rb");
			if (in == NULL) 
			{
				perror("cannot open file");
				continue;
			}
			dump(type, in, argv[i]);
			fclose(in);
		}
	}

	return 0;
}
