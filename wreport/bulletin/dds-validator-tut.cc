/*
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

#include <test-utils-wreport.h>
#include <wreport/bulletin/dds-validator.h>
#include <set>

using namespace wreport;
using namespace std;

namespace tut {

struct bulletin_dds_validator_shar
{
    bulletin_dds_validator_shar()
    {
    }

    ~bulletin_dds_validator_shar()
    {
    }
};
TESTGRP(bulletin_dds_validator);

static void _validate(Bulletin& b, const wibble::tests::Location& loc)
{
    try {
        // Validate them
        bulletin::DDSValidator validator(b);
        b.visit(validator);
    } catch (std::exception& e) {
        try {
            b.print_structured(stderr);
        } catch (std::exception& e) {
            cerr << "dump interrupted: " << e.what() << endl;
        }
        throw tut::failure(loc.msg(e.what()));
    }
}

#define validate(b, name) _validate(b, wibble::tests::Location(__FILE__, __LINE__, (string("validating ") + name).c_str()))


// Ensure that the validator works against normal bufr messages
template<> template<>
void to::test<1>()
{
    std::set<std::string> blacklist;
    blacklist.insert("bufr/gen-generic.bufr");
    blacklist.insert("bufr/obs255-255.0.bufr");
    blacklist.insert("bufr/tempforecast.bufr");
    blacklist.insert("bufr/bad-edition.bufr");
    blacklist.insert("bufr/corrupted.bufr");
    blacklist.insert("bufr/test-soil1.bufr");

    std::vector<std::string> files = tests::all_test_files("bufr");
    for (std::vector<std::string>::const_iterator i = files.begin();
            i != files.end(); ++i)
    {
        if (blacklist.find(*i) != blacklist.end()) continue;

        // Read the whole contents of the test file
        std::string raw1 = tests::slurpfile(*i);

        // Decode the original contents
        BufrBulletin msg1;
        try {
            msg1.decode(raw1, i->c_str());
        } catch (std::exception& e) {
            throw tut::failure(*i + ": " + e.what());
        }

        // Validate them
        validate(msg1, *i);
    }
}

// Ensure that the validator works against normal crex messages
template<> template<>
void to::test<2>()
{
    std::set<std::string> blacklist;
    blacklist.insert("crex/test-temp0.crex");

    std::vector<std::string> files = tests::all_test_files("crex");
    for (std::vector<std::string>::const_iterator i = files.begin();
            i != files.end(); ++i)
    {
        if (blacklist.find(*i) != blacklist.end()) continue;

        // Read the whole contents of the test file
        std::string raw1 = tests::slurpfile(*i);

        // Decode the original contents
        CrexBulletin msg1;
        msg1.decode(raw1, i->c_str());

        // Validate them
        validate(msg1, *i);
    }
}

}

// vim:set ts=4 sw=4:
