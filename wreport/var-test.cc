#include "tests.h"
#include "var.h"
#include "vartable.h"
#include "options.h"
#include <cmath>
#include <cstring>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("create", []() {
            // Test variable creation
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");
            // LONGITUDE (HIGH ACCURACY) 5 decimal digits, -18000000 bit_ref, 26 bits
            Varinfo info = table->query(WR_VAR(0, 6, 1));

            {
                Var var(info);
                wassert(actual(var.code()) == WR_VAR(0, 6, 1));
                wassert(actual(var.info()->code) == WR_VAR(0, 6, 1));
                wassert(actual(var.isset()).isfalse());
            }

            {
                Var var(info, 123);
                wassert(actual(var.code()) == WR_VAR(0, 6, 1));
                wassert(actual(var.info()->code) == WR_VAR(0, 6, 1));
                wassert(actual(var.isset()).istrue());
                wassert(actual(var) == 123);
                var.seti(-123);
                wassert(actual(var) == -123);
            }

            {
                Var var(info, 123.456);
                wassert(actual(var.code()) == WR_VAR(0, 6, 1));
                wassert(actual(var.info()->code) == WR_VAR(0, 6, 1));
                wassert(actual(var.isset()).istrue());
                wassert(actual(var) == 123.456);
                var.setd(-123.456);
                wassert(actual(var) == -123.456);
            }

            {
                Var var(info, "123");
                wassert(actual(var.code()) == WR_VAR(0, 6, 1));
                wassert(actual(var.info()->code) == WR_VAR(0, 6, 1));
                wassert(actual(var.isset()).istrue());
                wassert(actual(var) == "123");
            }

            {
                Var var(info, string("123"));
                wassert(actual(var.code()) == WR_VAR(0, 6, 1));
                wassert(actual(var.info()->code) == WR_VAR(0, 6, 1));
                wassert(actual(var.isset()).istrue());
                wassert(actual(var) == "123");
            }

#if 0
            {
                Var var(WR_VAR(0, 6, 1));
                ensure_equals(var.code(), WR_VAR(0, 6, 1));
                ensure_equals(var.value(), (const char*)0);
            }
#endif
        });
        add_method("get_set", []() {
            // Get and set values
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");

            // setc
            {
                // STATION OR SITE NAME, 20 chars
                Varinfo info = table->query(WR_VAR(0, 1, 15));
                Var var(info);

                // Normal setc
                var.setc("foobar");
                wassert(actual(var) == "foobar");

                // Setc with truncation
                var.setc_truncate("Budapest Pestszentlorinc-kulterulet");
                wassert(actual(var) == "Budapest Pestszentlo");

                // ensure that setc would complain for the length
                try {
                    var.setc("Budapest Pestszentlorinc-kulterulet");
                } catch (std::exception& e) {
                    wassert(actual(e.what()).contains("is too long"));
                }
            }
        });
        add_method("copy", []() {
            // Test variable copy
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");

            Var var(table->query(WR_VAR(0, 6, 1)));
            var.seti(234);

            var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 75)));
            var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 15)), 45)));

            wassert(actual(var.enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(*var.enqa(WR_VAR(0, 33, 7))) == 75);

            wassert(actual(var.enqa(WR_VAR(0, 33, 15)) != nullptr).istrue());
            wassert(actual(*var.enqa(WR_VAR(0, 33, 15))) == 45);


            Var var1 = var;
            wassert(actual(var1) == 234);
            wassert(actual(var) == var1);
            wassert(actual(var1) == var);

            wassert(actual(var1.enqa(WR_VAR(0, 33, 7)) != nullptr).istrue());
            wassert(actual(*var1.enqa(WR_VAR(0, 33, 7))) == 75);

            wassert(actual(var1.enqa(WR_VAR(0, 33, 15)) != nullptr).istrue());
            wassert(actual(*var1.enqa(WR_VAR(0, 33, 15))) == 45);

            // Fiddle with the attribute and make sure dba_var_equals notices
            var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33,  7)), 10)));
            wassert(actual(var) != var1);
            wassert(actual(var1) != var);
        });
        add_method("missing", []() {
            // Test missing checks
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");
            Var var(table->query(WR_VAR(0, 12, 103)));

            try {
                var.setd(logf(0));
                throw TestFailed("error_domain was not thrown");
            } catch (error_domain& e) {
                wassert(actual(var.isset()).isfalse());
            }

            try {
                var.setd(logf(0)/logf(0));
                throw TestFailed("error_domain was not thrown");
            } catch (error_domain& e) {
                wassert(actual(var.isset()).isfalse());
            }
        });
        add_method("ranges", []() {
            // Test ranges
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");
            Var var(table->query(WR_VAR(0, 21, 143)));

            var.setd(1.0);
            wassert(actual(var) == 1.0);

            var.setd(-1.0);
            wassert(actual(var) == -1.0);
        });
        add_method("attributes", []() {
            // Test attributes
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");
            Var var(table->query(WR_VAR(0, 21, 143)));

            // No attrs at the beginning
            wassert(actual(var.enqa(WR_VAR(0, 33, 7))).isfalse());

            // Set an attr
            var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 42)));

            // Query it back
            wassert(actual(var.enqa(WR_VAR(0, 33, 7))).istrue());
            wassert(actual(*var.enqa(WR_VAR(0, 33, 7))) == 42);

            // Unset it
            var.unseta(WR_VAR(0, 33, 7));

            // Query it back: it should be NULL
            wassert(actual(var.enqa(WR_VAR(0, 33, 7))).isfalse());
        });
        add_method("enq", []() {
            // Test templated enq
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");
            Var var(table->query(WR_VAR(0, 21, 143)));

            wassert(actual(var.enq(2.0)) == 2.0);
            wassert(actual(var.enq(42)) == 42);
            wassert(actual(var.enq("foo")) == "foo");

            var.set(1.0);
            wassert(actual(var.enq<double>()) == 1.0);
            wassert(actual(var.enq<int>()) == 100);
            wassert(actual(var.enq<const char*>()) == "100");
            wassert(actual(var.enq<string>()) == "100");

            var.set(-1.0);
            wassert(actual(var.enq<double>()) == -1.0);
            wassert(actual(var.enq<int>()) == -100);
            wassert(actual(var.enq<const char*>()) == "-100");
            wassert(actual(var.enq<string>()) == "-100");
        }),
        add_method("format", []() {
            // Test formatting and reparsing
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");

            // Missing
            {
                Varinfo info = table->query(WR_VAR(0, 1, 1));
                Var var(info);
                string f = var.format("");
                Var var1(info);
                var1.setf(f.c_str());
                wassert(actual(var) == var1);
            }

            // String
            {
                Varinfo info = table->query(WR_VAR(0, 1, 15));
                Var var(info, "antani");
                string f = var.format("");
                Var var1(info);
                var1.setf(f.c_str());
                wassert(actual(var) == var1);
            }

            // Integer
            {
                Varinfo info = table->query(WR_VAR(0, 1, 2));
                Var var(info, 123);
                string f = var.format("");
                Var var1(info);
                var1.setf(f.c_str());
                wassert(actual(var) == var1);
            }

            // Double
            {
                Varinfo info = table->query(WR_VAR(0, 5, 1));
                Var var(info, 12.345);
                string f = var.format("");
                wassert(actual(f) == "12.34500");
                Var var1(info);
                var1.setf(f.c_str());
                wassert(actual(var) == var1);
            }
        });
        add_method("truncation", []() {
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
            wassert(actual(norm) == "Budapest Pestszentlo");
        });
        add_method("domain", []() {
            // Test domain erros and var_silent_domain_errors
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");
            // WMO BLOCK NUMBER, 0--99
            Varinfo info = table->query(WR_VAR(0, 1, 1));
            Var var(info, 10);

            // Setting an out of bound value raises error_domain by default
            try {
                var.seti(200);
                throw TestFailed("error_domain was not thrown");
            } catch (error_domain& e) {
                // ok, it should throw
            }

            // If we ignore the exception, we find that the variable has been unset
            wassert(actual(var).isunset());

            // Set a value again
            var.seti(10);
            wassert(actual(var) == 10);

            // If we set var_silent_domain_errors, the var becomes unset without an
            // error being raised
            {
                auto o = options::local_override(options::var_silent_domain_errors, true);
                var.seti(200);
                wassert(actual(var).isunset());
            }

            // Check that the behaviour is restored correctly
            try {
                var.seti(200);
                throw TestFailed("error_domain was not thrown");
            } catch (error_domain& e) {
                // ok, it should throw
            }
        });
        add_method("binary", []() {
            // Test binary values
            _Varinfo info06; info06.set_binary(WR_VAR(0, 0, 0), "TEST BINARY 06 bits", 6);
            wassert(actual(info06.len) == 1u);
            _Varinfo info16; info16.set_binary(WR_VAR(0, 0, 0), "TEST BINARY 16 bits", 16);
            _Varinfo info18; info18.set_binary(WR_VAR(0, 0, 0), "TEST BINARY 18 bits", 18);

            // Bit buffers are zero-padded
            Var var06a(&info06, "\xaf\x5f");
            wassert(actual(var06a.format()) == "2F");
            Var var16a(&info16, "\xaf\x5f");
            wassert(actual(var16a.format()) == "AF5F");
            Var var18a(&info18, "\xaf\x5f\xff");
            wassert(actual(var18a.format()) == "AF5F03");

            var06a.setc("\xff");
            wassert(actual(memcmp(var06a.enqc(), "\x3f", 1) == 0).istrue());
        });
        add_method("seti", []() {
            _Varinfo vi;
            vi.set_bufr(WR_VAR(0, 0, 0), "TEST", "?", 0, 10, 0, 32);
            Var var(&vi);
            var.seti(0); wassert(actual(var.enqc()) == "0");
            var.seti(1); wassert(actual(var.enqc()) == "1");
            var.seti(100); wassert(actual(var.enqc()) == "100");
            var.seti(1000); wassert(actual(var.enqc()) == "1000");
            var.seti(1234567890); wassert(actual(var.enqc()) == "1234567890");
            var.seti(45); wassert(actual(var.enqc()) == "45");
            var.seti(-1); wassert(actual(var.enqc()) == "-1");
            var.seti(-10800); wassert(actual(var.enqc()) == "-10800");
            var.seti(-11000000); wassert(actual(var.enqc()) == "-11000000");
            var.seti(-2147483647); wassert(actual(var.enqc()) == "-2147483647");
        });
        add_method("setd", []() {
            _Varinfo vi;
            vi.set_bufr(WR_VAR(0, 0, 0), "TEST", "?", 2, 5, 0, 16);
            Var var(&vi);
            var.setd(0);
            wassert(actual(var.enqc()) == "0");
            wassert(actual(var.enqi()) == 0);
            wassert(actual(var.enqd()) == 0.0);
            var.setd(1);
            wassert(actual(var.enqc()) == "100");
            wassert(actual(var.enqi()) == 100);
            wassert(actual(var.enqd()) == 1.0);
            var.setd(100);
            wassert(actual(var.enqc()) == "10000");
            wassert(actual(var.enqi()) == 10000);
            wassert(actual(var.enqd()) == 100.0);
            var.setd(1.5);
            wassert(actual(var.enqc()) == "150");
            wassert(actual(var.enqi()) == 150);
            wassert(actual(var.enqd()) == 1.5);
            var.setd(1.234567890);
            wassert(actual(var.enqc()) == "123");
            wassert(actual(var.enqi()) == 123);
            wassert(actual(var.enqd()) == 1.23);
        });
        add_method("setc", []() {
            _Varinfo vi;
            vi.set_string(WR_VAR(0, 0, 0), "TEST", 5);
            Var var(&vi);
            var.setc(""); wassert(actual(var.enqc()) == "");
            var.setc("ciao"); wassert(actual(var.enqc()) == "ciao");
            var.setc("ciao!"); wassert(actual(var.enqc()) == "ciao!");
            var.setc("ciaone"); wassert(actual(var.enqc()) == "ciaon");

            Var var1(&vi, var);
            wassert(actual(var1.enqc()) == "ciaon");

            Var var2(var);
            wassert(actual(var2.enqc()) == "ciaon");

            Var var3(move(var2));
            wassert(actual(var3.enqc()) == "ciaon");
        });
        add_method("sets", []() {
            _Varinfo vi;
            vi.set_string(WR_VAR(0, 0, 0), "TEST", 5);
            Var var(&vi);
            var.sets(""); wassert(actual(var.enqc()) == "");
            var.sets("ciao"); wassert(actual(var.enqc()) == "ciao");
            var.sets("ciao!"); wassert(actual(var.enqc()) == "ciao!");
            var.sets("ciaone"); wassert(actual(var.enqc()) == "ciaon");

            Var var1(&vi, var);
            wassert(actual(var1.enqc()) == "ciaon");

            Var var2(var);
            wassert(actual(var2.enqc()) == "ciaon");

            Var var3(move(var2));
            wassert(actual(var3.enqc()) == "ciaon");
        });
        add_method("issue17", []() {
            _Varinfo vi;
            vi.set_bufr(WR_VAR(0, 0, 0), "TEST", "?", 2, 5, 0, 16);
            Var var(&vi);
            wassert(var.sets("None"));
            wassert_false(var.isset());
            wassert(var.setc("None"));
            wassert_false(var.isset());
        });
    }

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
} test("var");

}
