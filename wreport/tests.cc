/*
 * wreport/tests - Wibble test suite functions to work with wreport types
 *
 * Copyright (C) 2013  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#include "wreport/var.h"
#include <fnmatch.h>
#include <sstream>

using namespace std;

namespace wreport {
std::ostream& operator<<(std::ostream& o, Vartype type)
{
    switch (type)
    {
        case Vartype::String: return o << "string";
        case Vartype::Binary: return o << "binary";
        case Vartype::Integer: return o << "integer";
        case Vartype::Decimal: return o << "decimal";
    }
}

namespace tests {

bool test_can_run(const std::string& group_name, const std::string& test_name)
{
    const char* filter = getenv("FILTER");
    const char* except = getenv("EXCEPT");

    if (!filter && !except) return true;

    if (filter && fnmatch(filter, group_name.c_str(), 0) == FNM_NOMATCH)
        return false;

    if (!except || fnmatch(except, group_name.c_str(), 0) == FNM_NOMATCH)
        return true;

    return false;
}


#ifdef wassert
namespace {
void compare_values(WIBBLE_TEST_LOCPRM, const Var& avar, const Var& evar, const std::string& name)
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
        wibble_test_location.fail_test(ss.str());
    }
}
}
void TestVarEqual::check(WIBBLE_TEST_LOCPRM) const
{
    if (!inverted)
    {
        // Code
        if (avar.code() != evar.code())
        {
            std::stringstream ss;
            ss << "variable codes differ: expected " << varcode_format(evar.code()) << " actual " << varcode_format(avar.code());
            wibble_test_location.fail_test(ss.str());
        }

        // Value
        compare_values(wibble_test_location, avar, evar, "variable");

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
                wibble_test_location.fail_test(ss.str());
            }

            compare_values(wibble_test_location, *aattr, *eattr, "attribute " + varcode_format(aattr->code()));

            // Move to the next attribute
            aattr = aattr->next_attr();
            eattr = eattr->next_attr();
        }
    } else {
        if (avar == evar)
        {
            std::stringstream ss;
            ss << "variables should differ, but are the same";
            wibble_test_location.fail_test(ss.str());
        }
    }

}
#endif

}
}
