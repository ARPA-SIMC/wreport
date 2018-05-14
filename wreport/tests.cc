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

#include "tests.h"
#include "utils/string.h"
#include "internals/fs.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

using namespace std;

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
    fs::Directory dir(testdatadir);
    for (fs::Directory::const_iterator i = dir.begin(); i != dir.end(); ++i)
        if (str::endswith(i->d_name, "." + encoding))
            res.push_back(str::joinpath(encoding, i->d_name));
    return res;
}

void track_bulletin(Bulletin& b, const char* tag, const char* fname)
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

namespace {
void compare_values(const Var& avar, const Var& evar, const std::string& name)
{
    if (!avar.value_equals(evar))
    {
        std::stringstream ss;
        ss << name << " values differ: ";
        if (!evar.isset())
            ss << "expected undefined";
        else
            ss << "expected is " << evar.format();
        ss << " but actual ";
        if (!avar.isset())
            ss << "is undefined";
        else
            ss << "is " << avar.format();
        throw TestFailed(ss.str());
    }
}
}


void assert_var_equal(const Var& avar, const Var& evar)
{
    // Code
    if (avar.code() != evar.code())
    {
        std::stringstream ss;
        ss << "variable codes differ: expected " << varcode_format(evar.code()) << " actual " << varcode_format(avar.code());
        throw TestFailed(ss.str());
    }

    // Value
    compare_values(avar, evar, "variable");

    // Attributes
    const Var* aattr = avar.next_attr();
    const Var* eattr = evar.next_attr();
    while (true)
    {
        if (!aattr && !eattr) break;
        // If both exists but codes are different, one of the two is missing an attribute
        if (aattr && eattr && aattr->code() != eattr->code())
        {
            // Set the highest one to NULL, and use the next check to
            // trigger the appropriate test failure
            if (aattr->code() < eattr->code())
                eattr = NULL;
            else
                aattr = NULL;
        }
        if (!aattr || !eattr)
        {
            std::stringstream ss;
            ss << "attributes differ: ";
            if (aattr)
                ss << "actual has " << varcode_format(aattr->code()) << " which was not expected";
            else
                ss << "actual does not have attribute " << varcode_format(eattr->code()) << " which was expected to be " << eattr->format("undefined");
            throw TestFailed(ss.str());
        }

        compare_values(*aattr, *eattr, "attribute " + varcode_format(aattr->code()));

        // Move to the next attribute
        aattr = aattr->next_attr();
        eattr = eattr->next_attr();
    }
}

void assert_var_not_equal(const Var& actual, const Var& expected)
{
    if (actual == expected)
    {
        std::stringstream ss;
        ss << "variables should differ, but are the same";
        throw TestFailed(ss.str());
    }
}

namespace {
template<typename T> bool equals(T a, T b) { return a == b; }
bool equals(const char* a, const char* b) { return strcmp(a, b) == 0; }
}

template<typename Val>
void assert_var_value_equal(const Var& actual, Val expected)
{
    Var vexpected(actual.info(), expected);
    if (!actual.value_equals(vexpected))
    {
        std::stringstream ss;
        ss << "actual variable value is " << actual.format() << " (" << actual.enq<Val>() << ") instead of " << expected;
        throw TestFailed(ss.str());
    }
}

template<typename Val>
void assert_var_value_not_equal(const Var& actual, Val expected)
{
    if (equals(actual.enq<Val>(), expected))
    {
        std::stringstream ss;
        ss << "actual variable value is " << actual.format() << " while it should be " << expected;
        throw TestFailed(ss.str());
    }
}

template void assert_var_value_equal<int>(const Var& actual, int expected);
template void assert_var_value_equal<double>(const Var& actual, double expected);
template void assert_var_value_equal<char*>(const Var& actual, char* expected);
template void assert_var_value_equal<char const*>(const Var& actual, char const* expected);
template void assert_var_value_equal<std::string>(const Var& actual, std::string expected);
template void assert_var_value_not_equal<int>(const Var& actual, int expected);
template void assert_var_value_not_equal<double>(const Var& actual, double expected);
template void assert_var_value_not_equal<char*>(const Var& actual, char* expected);
template void assert_var_value_not_equal<char const*>(const Var& actual, char const* expected);
template void assert_var_value_not_equal<std::string>(const Var& actual, std::string expected);

void ActualVar::isset() const
{
    if (_actual.isset()) return;
    std::stringstream ss;
    ss << "actual variable is unset, but it should not be";
    throw TestFailed(ss.str());
}

void ActualVar::isunset() const
{
    if (!_actual.isset()) return;
    std::stringstream ss;
    ss << "actual variable value is " << _actual.format() << ", but it should be unset";
    throw TestFailed(ss.str());
}

void ActualVarcode::operator==(Varcode expected) const
{
    if (expected == _actual) return;
    std::stringstream ss;
    ss << "actual varcode value is " << varcode_format(_actual) << " but it should be " << varcode_format(expected);
    throw TestFailed(ss.str());
}

void ActualVarcode::operator!=(Varcode expected) const
{
    if (expected != _actual) return;
    std::stringstream ss;
    ss << "actual varcode value is " << varcode_format(_actual) << " but it should not be";
    throw TestFailed(ss.str());
}

template<typename BULLETIN>
void TestCodec<BULLETIN>::run()
{
    WREPORT_TEST_INFO(test_info);

    // Read the whole contents of the test file
    std::string raw1 = wcallchecked(slurpfile(fname));

    test_info() << fname << ": decode original version";
    auto msg1 = wcallchecked(decode_checked<BULLETIN>(raw1, fname.c_str()));
    wassert(check_contents(*msg1));

    test_info() << fname << ": decode original version, verbose version";
    FILE* out = fopen("/dev/null", "w");
    auto msg1a = wcallchecked(decode_checked<BULLETIN>(raw1, fname.c_str(), out));
    wassert(check_contents(*msg1a));
    fclose(out);

    test_info() << fname << ": comparing normal and verbose decoder output";
    {
        notes::Collect c(std::cerr);
        unsigned diffs = msg1->diff(*msg1a);
        if (diffs)
        {
            track_bulletin(*msg1, "orig", fname.c_str());
            track_bulletin(*msg1a, "verb", fname.c_str());
        }
        wassert(actual(diffs) == 0u);
    }

    // Encode it again
    test_info() << fname << ": re-encode original version";
    std::string raw = wcallchecked(msg1->encode());

    // Decode our encoder's output
    test_info() << fname << ": decode what we encoded";
    auto msg2 = wcallchecked(decode_checked<BULLETIN>(raw, fname.c_str()));

    // Test the decoded version
    wassert(check_contents(*msg2));

    // Ensure the two are the same
    test_info() << fname << ": comparing original and re-encoded";
    notes::Collect c(std::cerr);
    unsigned diffs = msg1->diff(*msg2);
    if (diffs)
    {
        track_bulletin(*msg1, "orig", fname.c_str());
        track_bulletin(*msg2, "reenc", fname.c_str());
    }
    wassert(actual(diffs) == 0u);
}

template class TestCodec<BufrBulletin>;
template class TestCodec<CrexBulletin>;

}
}
