/*
 * wreport/test-utils-wreport - Unit test utilities, not included in the library
 *
 * Copyright (C) 2005--2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#ifndef WREPORT_TESTS_UTILS
#define WREPORT_TESTS_UTILS

#include <wibble/tests.h>
#include <wreport/varinfo.h>
#include <wreport/bulletin.h>
#include <wreport/tests.h>
#include <wreport/notes.h>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cstdlib>

namespace wreport {
struct Var;

namespace tests {

/// Return the pathname of a test file
std::string datafile(const std::string& fname);

/**
 * Read the entire contents of a test file into a string
 *
 * The file name will be resolved through datafile
 */
std::string slurpfile(const std::string& name);

/**
 * Get a list of all test files for the given encoding
 */
std::vector<std::string> all_test_files(const std::string& encoding);

void track_bulletin(const Bulletin& b, const char* tag, const char* fname);

template<typename BULLETIN>
struct MsgTester
{
	virtual ~MsgTester() {}
	virtual void test(const BULLETIN& msg) = 0;
	void operator()(const std::string& name, const BULLETIN& msg)
	{
		try {
			test(msg);
		} catch (tut::failure& f) {
			throw tut::failure("[" + name + "]" + f.what());
		}
	}

	void run(const char* name)
	{
		// Read the whole contents of the test file
		std::string raw1 = slurpfile(name);

        // Decode the original contents
        std::unique_ptr<BULLETIN> msg1 = BULLETIN::create();
        try {
            msg1->decode(raw1, name);
        } catch (wreport::error_parse& e) {
            try {
                msg1->print_structured(stderr);
            } catch (wreport::error& e) {
                std::cerr << "Dump interrupted: " << e.what();
            }
            throw;
        }
        (*this)("orig", *msg1);

		// Encode it again
		std::string raw;
		msg1->encode(raw);

        // Decode our encoder's output
        std::unique_ptr<BULLETIN> msg2 = BULLETIN::create();
        msg2->decode(raw, name);

        // Test the decoded version
        (*this)("reencoded", *msg2);

        // Ensure the two are the same
        notes::Collect c(std::cerr);
        unsigned diffs = msg1->diff(*msg2);
        if (diffs)
        {
            track_bulletin(*msg1, "orig", name);
            track_bulletin(*msg2, "reenc", name);
        }
        ensure_equals(diffs, 0);
    }

	void run(const char* tag, const BULLETIN& msg)
	{
		(*this)(tag, msg);
	}
};

} // namespace tests
} // namespace wreport

#endif
