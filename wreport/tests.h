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

#include <wreport/utils/tests.h>
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

void track_bulletin(Bulletin& b, const char* tag, const char* fname);

template<typename BULLETIN>
std::unique_ptr<BULLETIN> decode_checked(const std::string& buf, const char* name)
{
    try {
        return BULLETIN::decode(buf, name);
    } catch (wreport::error_parse& e) {
        try {
            auto h = BULLETIN::decode_header(buf, name);
            h->print_structured(stderr);
        } catch (wreport::error& e) {
            std::cerr << "Dump interrupted: " << e.what();
        }
        throw;
    }
}

template<typename BULLETIN>
std::unique_ptr<BULLETIN> decode_checked(const std::string& buf, const char* name, FILE* verbose)
{
    try {
        return BULLETIN::decode_verbose(buf, verbose, name);
    } catch (wreport::error_parse& e) {
        try {
            auto h = BULLETIN::decode_header(buf, name);
            h->print_structured(stderr);
        } catch (wreport::error& e) {
            std::cerr << "Dump interrupted: " << e.what();
        }
        throw;
    }
}

template<typename BULLETIN>
struct TestCodec
{
    std::string fname;
    std::function<void(const BULLETIN&)> check_contents = [](const BULLETIN&) {};

    TestCodec(const std::string& fname) : fname(fname) {}
    virtual ~TestCodec() {}

    void run();
};

void assert_var_equal(const Var& actual, const Var& expected);
void assert_var_not_equal(const Var& actual, const Var& expected);
template<typename Val>
void assert_var_value_equal(const Var& actual, Val expected);
template<typename Val>
void assert_var_value_not_equal(const Var& actual, Val expected);


struct ActualVar : public Actual<Var>
{
    ActualVar(const Var& actual) : Actual<Var>(actual) {}

    void operator==(const Var& expected) const { assert_var_equal(_actual, expected); }
    void operator!=(const Var& expected) const { assert_var_not_equal(_actual, expected); }
    template<typename Val>
    void operator==(Val expected) const { assert_var_value_equal(_actual, expected); }
    template<typename Val>
    void operator!=(Val expected) const { assert_var_value_not_equal(_actual, expected); }
    void isset() const;
    void isunset() const;
};

inline ActualVar actual(const wreport::Var& actual) { return ActualVar(actual); }

struct ActualVarcode : public Actual<Varcode>
{
    using Actual::Actual;

    void operator==(Varcode expected) const;
    void operator!=(Varcode expected) const;
};

inline ActualVarcode actual_varcode(Varcode actual) { return ActualVarcode(actual); }

}
}

#endif
