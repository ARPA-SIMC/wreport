/*
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

#include <test-utils-wreport.h>
#include <wreport/var.h>
#include <wreport/vartable.h>
#include <wreport/options.h>
#include <math.h>

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("create", [](Fixture& f) {
        // Test variable creation
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        Varinfo info = table->query(WR_VAR(0, 6, 1));

        {
            Var var(info);
            ensure_equals(var.code(), WR_VAR(0, 6, 1));
            ensure_equals(var.info()->code, WR_VAR(0, 6, 1));
            ensure_equals(var.value(), (const char*)0);
        }

        {
            Var var(info, 123);
            ensure_equals(var.code(), WR_VAR(0, 6, 1));
            ensure_equals(var.info()->code, WR_VAR(0, 6, 1));
            ensure(var.value() != 0);
            ensure_var_equals(var, 123);
            var.seti(-123);
            ensure_var_equals(var, -123);
        }

        {
            Var var(info, 123.456);
            ensure_equals(var.code(), WR_VAR(0, 6, 1));
            ensure_equals(var.info()->code, WR_VAR(0, 6, 1));
            ensure(var.value() != 0);
            ensure_var_equals(var, 123.456);
            var.setd(-123.456);
            ensure_var_equals(var, -123.456);
        }

        {
            Var var(info, "123");
            ensure_equals(var.code(), WR_VAR(0, 6, 1));
            ensure_equals(var.info()->code, WR_VAR(0, 6, 1));
            ensure(var.value() != 0);
            ensure_var_equals(var, "123");
        }

#if 0
        {
            Var var(WR_VAR(0, 6, 1));
            ensure_equals(var.code(), WR_VAR(0, 6, 1));
            ensure_equals(var.value(), (const char*)0);
        }
#endif
    }),
    Test("get_set", [](Fixture& f) {
        // Get and set values
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");

        // setc
        {
            // STATION OR SITE NAME, 20 chars
            Varinfo info = table->query(WR_VAR(0, 1, 15));
            Var var(info);

            // Normal setc
            var.setc("foobar");
            ensure_var_equals(var, "foobar");

            // Setc with truncation
            var.setc_truncate("Budapest Pestszentlorinc-kulterulet");
            ensure_var_equals(var, "Budapest Pestszentl>");

            // ensure that setc would complain for the length
            try {
                var.setc("Budapest Pestszentlorinc-kulterulet");
            } catch (std::exception& e) {
                ensure_contains(e.what(), "is too long");
            }
        }
    }),
    Test("copy", [](Fixture& f) {
        // Test variable copy
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");

        Var var(table->query(WR_VAR(0, 6, 1)));
        var.seti(234);

        var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 75)));
        var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 15)), 45)));

        ensure(var.enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_var_equals(*var.enqa(WR_VAR(0, 33, 7)), 75);

        ensure(var.enqa(WR_VAR(0, 33, 15)) != NULL);
        ensure_var_equals(*var.enqa(WR_VAR(0, 33, 15)), 45);


        Var var1 = var;
        ensure_var_equals(var1, 234);
        ensure(var == var1);
        ensure(var1 == var);

        ensure(var1.enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_var_equals(*var1.enqa(WR_VAR(0, 33, 7)), 75);

        ensure(var1.enqa(WR_VAR(0, 33, 15)) != NULL);
        ensure_var_equals(*var1.enqa(WR_VAR(0, 33, 15)), 45);

        // Fiddle with the attribute and make sure dba_var_equals notices
        var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 10)));
        ensure(var != var1);
        ensure(var1 != var);
    }),
    Test("missing", [](Fixture& f) {
        // Test missing checks
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        Var var(table->query(WR_VAR(0, 12, 103)));

        try {
            var.setd(logf(0));
            ensure(false);
        } catch (error_domain& e) {
            ensure(var.value() == NULL);
        }

        try {
            var.setd(logf(0)/logf(0));
            ensure(false);
        } catch (error_domain& e) {
            ensure(var.value() == NULL);
        }
    }),
    Test("ranges", [](Fixture& f) {
        // Test ranges
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        Var var(table->query(WR_VAR(0, 21, 143)));

        var.setd(1.0);
        ensure_equals(var.enqd(), 1.0);

        var.setd(-1.0);
        ensure_equals(var.enqd(), -1.0);
    }),
    Test("attributes", [](Fixture& f) {
        // Test attributes
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        Var var(table->query(WR_VAR(0, 21, 143)));

        // No attrs at the beginning
        ensure(var.enqa(WR_VAR(0, 33, 7)) == NULL);

        // Set an attr
        var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 42)));

        // Query it back
        ensure(var.enqa(WR_VAR(0, 33, 7)) != NULL);
        ensure_var_equals(*var.enqa(WR_VAR(0, 33, 7)), 42);

        // Unset it
        var.unseta(WR_VAR(0, 33, 7));

        // Query it back: it should be NULL
        ensure(var.enqa(WR_VAR(0, 33, 7)) == NULL);
    }),
    Test("enq", [](Fixture& f) {
        // Test templated enq
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        Var var(table->query(WR_VAR(0, 21, 143)));

        ensure_equals(var.enq(2.0), 2.0);
        ensure_equals(var.enq(42), 42);
        ensure_equals(string(var.enq("foo")), "foo");

        var.set(1.0);
        ensure_equals(var.enq<double>(), 1.0);
        ensure_equals(var.enq<int>(), 100);
        ensure_equals(string(var.enq<const char*>()), "100");
    }),
    Test("format", [](Fixture& f) {
        // Test formatting and reparsing
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");

        // Missing
        {
            Varinfo info = table->query(WR_VAR(0, 1, 1));
            Var var(info);
            string f = var.format("");
            Var var1(info);
            var1.set_from_formatted(f.c_str());
            ensure(var == var1);
        }

        // String
        {
            Varinfo info = table->query(WR_VAR(0, 1, 15));
            Var var(info, "antani");
            string f = var.format("");
            Var var1(info);
            var1.set_from_formatted(f.c_str());
            ensure(var == var1);
        }

        // Integer
        {
            Varinfo info = table->query(WR_VAR(0, 1, 2));
            Var var(info, 123);
            string f = var.format("");
            Var var1(info);
            var1.set_from_formatted(f.c_str());
            ensure(var == var1);
        }

        // Double
        {
            Varinfo info = table->query(WR_VAR(0, 5, 1));
            Var var(info, 12.345);
            string f = var.format("");
            Var var1(info);
            var1.set_from_formatted(f.c_str());
            ensure(var == var1);
        }
    }),
    Test("truncation", [](Fixture& f) {
        // Test truncation of altered strings when copied to normal strings
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        // STATION OR SITE NAME, 20 chars
        Varinfo info = table->query(WR_VAR(0, 1, 15));
        // Create an amended version for longer site names
        Varinfo extended = table->query_altered(WR_VAR(0, 1, 15), 0, 40*8);

        // Create a variable with an absurdly long value
        Var ext(extended, "Budapest Pestszentlorinc-kulterulet");

        // Try to fit it into a normal variable
        Var norm(info);
        norm.set(ext);
        ensure_var_equals(norm, "Budapest Pestszentl>");
    }),
    Test("domain", [](Fixture& f) {
        // Test domain erros and var_silent_domain_errors
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        // WMO BLOCK NUMBER, 0--99
        Varinfo info = table->query(WR_VAR(0, 1, 1));
        Var var(info, 10);

        // Setting an out of bound value raises error_domain by default
        try {
            var.seti(200);
            ensure(false);
        } catch (error_domain& e) {
            // ok, it should throw
        }

        // If we ignore the exception, we find that the variable has been unset
        ensure(!var.isset());

        // Set a value again
        var.seti(10);
        ensure_equals(var.enqi(), 10);

        // If we set var_silent_domain_errors, the var becomes unset without an
        // error being raised
        {
            options::LocalOverride<bool> o(options::var_silent_domain_errors, true);
            var.seti(200);
            ensure(!var.isset());
        }

        // Check that the behaviour is restored correctly
        try {
            var.seti(200);
            ensure(false);
        } catch (error_domain& e) {
            // ok, it should throw
        }
    }),
    Test("binary", [](Fixture& f) {
        _Varinfo info06; info06.set_binary(WR_VAR(0, 0, 0), "TEST BINARY 06 bits", 6);
        _Varinfo info16; info16.set_binary(WR_VAR(0, 0, 0), "TEST BINARY 16 bits", 16);
        _Varinfo info18; info18.set_binary(WR_VAR(0, 0, 0), "TEST BINARY 18 bits", 18);

        Var var06a(&info06, "\xaf\x5f");
        wassert(actual(var06a.format()) == "A");
        Var var16a(&info06, "\xaf\x5f");
        wassert(actual(var16a.format()) == "A");
        Var var18a(&info06, "\xaf\x5f");
        wassert(actual(var18a.format()) == "A");
    }),

#if 0
// FIXME: this rounding bias doesn't seem to be fixable at this stage
// Test geopotential conversions
template<> template<>
void to::test<10>()
{
    const Vartable* table = Vartable::get_bufr("B0000000000000014000");
    // Start with B10003 (old ECMWF TEMP templates)
    Var var0(table->query(WR_VAR(0, 10, 3)), 152430.0);
    var0.print(stderr);
    // Convert to B10008 (used for geopotential by DB-All.e)
    Var var1(table->query(WR_VAR(0, 10, 8)), var0);
    var1.print(stderr);
    // Convert to B10009 (new GTS TEMP templates)
    Var var2(table->query(WR_VAR(0, 10, 9)), var1);
    var2.print(stderr);
    // Convert to B10008 (used for geopotential by DB-All.e)
    Var var3(table->query(WR_VAR(0, 10, 8)), var2);
    var3.print(stderr);
    // Convert back to B10003
    Var var4(table->query(WR_VAR(0, 10, 3)), var3);
    var4.print(stderr);

    ensure_var_equals(var4, 152430.0);
}
#endif
};

test_group newtg("var", tests);

}
