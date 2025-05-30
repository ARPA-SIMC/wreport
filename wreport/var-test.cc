#include "internals/varinfo.h"
#include "options.h"
#include "tests.h"
#include "var.h"
#include "vartable.h"
#include <cmath>
#include <cstring>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;

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

template <typename T> struct test_domain_error
{
    Varinfo info;
    T domain_min;
    T domain_max;
    test_domain_error(Varinfo info, T domain_min, T domain_max)
        : domain_min(domain_min), domain_max(domain_max)
    {
        T valid = domain_min + (domain_max - domain_min) / 2;
        Var var(info, valid);

        // Setting an out of bound value raises error_domain by default
        wassert_throws(error_domain, var.set(domain_max + 1));

        // If we ignore the exception, we find that the variable has been unset
        wassert(actual(var).isunset());

        // Set a value again
        wassert(var.set(valid));
        wassert(actual(var) == valid);

        // If we set var_silent_domain_errors, the var becomes unset without an
        // error being raised
        {
            auto o = options::local_override(options::var_silent_domain_errors,
                                             true);
            wassert(var.set(domain_max + 1));
            wassert(actual(var).isunset());
        }

        // Test clamping
        {
            auto o =
                options::local_override(options::var_clamp_domain_errors, true);
            wassert(var.set(domain_max + 1));
            wassert(actual(var) == domain_max);
        }

        {
            auto o =
                options::local_override(options::var_clamp_domain_errors, true);
            wassert(var.set(domain_min - 1));
            wassert(actual(var) == domain_min);
        }

        // Check that the behaviour is restored correctly
        wassert_throws(error_domain, var.set(domain_max + 1));
        wassert_throws(error_domain, var.set(domain_min - 1));
    }
};

void Tests::register_tests()
{

    add_method("create", []() {
        // Test variable creation
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        // LONGITUDE (HIGH ACCURACY) 5 decimal digits, -18000000 bit_ref, 26
        // bits
        Varinfo info          = table->query(WR_VAR(0, 6, 1));

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
            try
            {
                var.setc("Budapest Pestszentlorinc-kulterulet");
            }
            catch (std::exception& e)
            {
                wassert(actual(e.what()).contains("is too long"));
            }
        }
    });
    add_method("copy", []() {
        // Test variable copy
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");

        Var var(table->query(WR_VAR(0, 6, 1)));
        var.seti(234);

        var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 75)));
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
        var.seta(unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 10)));
        wassert(actual(var) != var1);
        wassert(actual(var1) != var);
    });
    add_method("missing", []() {
        // Test missing checks
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");
        Var var(table->query(WR_VAR(0, 12, 103)));

        try
        {
            var.setd(logf(0));
            throw TestFailed("error_domain was not thrown");
        }
        catch (error_domain& e)
        {
            wassert(actual(var.isset()).isfalse());
        }

        try
        {
            var.setd(logf(0) / logf(0));
            throw TestFailed("error_domain was not thrown");
        }
        catch (error_domain& e)
        {
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
    add_method("enq",
               []() {
                   // Test templated enq
                   const Vartable* table =
                       Vartable::get_bufr("B0000000000000014000");
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
        Varinfo info          = table->query(WR_VAR(0, 1, 15));
        // Create an amended version for longer site names
        Varinfo extended = table->query_altered(WR_VAR(0, 1, 15), 0, 40 * 8, 0);

        // Create a variable with an absurdly long value
        Var ext(extended, "Budapest Pestszentlorinc-kulterulet");

        // Try to fit it into a normal variable
        Var norm(info);
        norm.set(ext);
        wassert(actual(norm) == "Budapest Pestszentlo");
    });
    add_method("domain", []() {
        // Test domain errors and var_silent_domain_errors
        const Vartable* table = Vartable::get_bufr("B0000000000000014000");

        Varinfo info = table->query(WR_VAR(0, 1, 1));
        wassert(test_domain_error<int>(info, 0, 126));

        info = table->query(WR_VAR(0, 2, 106));
        wassert(test_domain_error<double>(info, 0, 6.2));
    });

    add_method("domain_hook", []() {
        // Test domain errors and var_silent_domain_errors
        struct Hook : public wreport::options::DomainErrorHook
        {
            unsigned int_called    = 0;
            unsigned double_called = 0;

            void handle_domain_error_int(Var& var, int32_t val) override
            {
                const Vartable* table =
                    Vartable::get_bufr("B0000000000000014000");
                Varinfo qinfo = table->query(WR_VAR(0, 33, 7));

                ++int_called;
                var.seti(0);
                var.seta(Var(qinfo, 1));
            }
            void handle_domain_error_double(Var& var, double val) override
            {
                const Vartable* table =
                    Vartable::get_bufr("B0000000000000014000");
                Varinfo qinfo = table->query(WR_VAR(0, 33, 7));

                ++double_called;
                var.setd(0.0);
                var.seta(Var(qinfo, 1));
            }
        } hook;

        const Vartable* table = Vartable::get_bufr("B0000000000000014000");

        Varinfo info = table->query(WR_VAR(0, 1, 1));
        // wassert(test_domain_error<int>(info, 0, 126));
        {
            Var var(info, 1);
            auto o =
                options::local_override(options::var_hook_domain_errors, &hook);
            wassert(var.set(127));
            wassert(actual(var.enqi()) == 0);
            wassert(actual(hook.int_called) == 1u);
            wassert(actual(hook.double_called) == 0u);

            const Var* q = var.enqa(WR_VAR(0, 33, 7));
            wassert_true(q);
            wassert(actual_varcode(q->code()) == WR_VAR(0, 33, 7));
            wassert(actual(q->enqi()) == 1);
        }

        info = table->query(WR_VAR(0, 2, 106));
        // wassert(test_domain_error<double>(info, 0, 6.2));
        {
            Var var(info, 1.0);
            auto o =
                options::local_override(options::var_hook_domain_errors, &hook);
            wassert(var.set(6.3));
            wassert(actual(var.enqd()) == 0.0);
            wassert(actual(hook.int_called) == 1u);
            wassert(actual(hook.double_called) == 1u);

            const Var* q = var.enqa(WR_VAR(0, 33, 7));
            wassert_true(q);
            wassert(actual_varcode(q->code()) == WR_VAR(0, 33, 7));
            wassert(actual(q->enqi()) == 1);

            var.unset();
            var.clear_attrs();

            wassert(var.set(nan("")));
            wassert(actual(var.enqd()) == 0.0);
            wassert(actual(hook.int_called) == 1u);
            wassert(actual(hook.double_called) == 2u);

            q = var.enqa(WR_VAR(0, 33, 7));
            wassert_true(q);
            wassert(actual_varcode(q->code()) == WR_VAR(0, 33, 7));
            wassert(actual(q->enqi()) == 1);
        }
    });

    add_method("binary", []() {
        // Test binary values
        _Varinfo info06;
        varinfo::set_binary(info06, WR_VAR(0, 0, 0), "TEST BINARY 06 bits", 6);
        wassert(actual(info06.len) == 1u);
        _Varinfo info16;
        varinfo::set_binary(info16, WR_VAR(0, 0, 0), "TEST BINARY 16 bits", 16);
        _Varinfo info18;
        varinfo::set_binary(info18, WR_VAR(0, 0, 0), "TEST BINARY 18 bits", 18);

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

    add_method("binary_padding", []() {
        // Check that everything is 0-padded
        // Use enqc to access the storage buffer
        auto to_hex = [](const char* buf) {
            std::string res;
            for (unsigned i = 0; i < 4; ++i)
            {
                char cbuf[4];
                snprintf(cbuf, 3, "%02hhX", static_cast<unsigned char>(buf[i]));
                res += cbuf;
            }
            return res;
        };
        _Varinfo info;
        varinfo::set_binary(info, WR_VAR(0, 0, 0), "TEST BINARY 3 bytes", 24);
        wassert(actual(info.len) == 3u);

        Var var(&info, "\0\0\0\0");
        wassert(actual(to_hex(var.enqc())) == "00000000");

        // Check that 0-padding happens
        memcpy(const_cast<char*>(var.enqc()), "\xff\xff\xff\xff", 4);
        var.setc("\xaa\x00\x00");
        wassert(actual(to_hex(var.enqc())) == "AA000000");

        // Check that 0-padding happens also when setting the full length
        memcpy(const_cast<char*>(var.enqc()), "\xff\xff\xff\xff", 4);
        var.setc("\xaa\xaa\xaa");
        wassert(actual(to_hex(var.enqc())) == "AAAAAA00");

        // Check that 0-padding happens also when setting with a longer input
        memcpy(const_cast<char*>(var.enqc()), "\xff\xff\xff\xff", 4);
        var.setc("\xaa\xaa\xaa\xaa\xaa");
        wassert(actual(to_hex(var.enqc())) == "AAAAAA00");
    });

    add_method("seti_positive", []() {
        _Varinfo vi;
        varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "TEST", "?", 31);
        Var var(&vi);
        wassert(var.seti(0));
        wassert(actual(var.enqc()) == "0");
        wassert(var.seti(1));
        wassert(actual(var.enqc()) == "1");
        wassert(var.seti(100));
        wassert(actual(var.enqc()) == "100");
        wassert(var.seti(1000));
        wassert(actual(var.enqc()) == "1000");
        wassert(var.seti(1234567890));
        wassert(actual(var.enqc()) == "1234567890");
        wassert(var.seti(45));
        wassert(actual(var.enqc()) == "45");
    });

    add_method("seti_negative", []() {
        _Varinfo vi;
        varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "TEST", "?", 31, -0x7fffffff);
        Var var(&vi);
        wassert(var.seti(-1));
        wassert(actual(var.enqc()) == "-1");
        wassert(var.seti(-10800));
        wassert(actual(var.enqc()) == "-10800");
        wassert(var.seti(-11000000));
        wassert(actual(var.enqc()) == "-11000000");
        wassert(var.seti(-2147483647));
        wassert(actual(var.enqc()) == "-2147483647");
    });

    add_method("setd", []() {
        _Varinfo vi;
        varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "TEST", "?", 16, 0, 2);
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
        varinfo::set_string(vi, WR_VAR(0, 0, 0), "TEST", 5);
        Var var(&vi);
        var.setc("");
        wassert(actual(var.enqc()) == "");
        var.setc("ciao");
        wassert(actual(var.enqc()) == "ciao");
        var.setc("ciao!");
        wassert(actual(var.enqc()) == "ciao!");
        var.setc("ciaone");
        wassert(actual(var.enqc()) == "ciaon");

        Var var1(&vi, var);
        wassert(actual(var1.enqc()) == "ciaon");

        Var var2(var);
        wassert(actual(var2.enqc()) == "ciaon");

        Var var3(std::move(var2));
        wassert(actual(var3.enqc()) == "ciaon");
    });

    add_method("sets", []() {
        _Varinfo vi;
        varinfo::set_string(vi, WR_VAR(0, 0, 0), "TEST", 5);
        Var var(&vi);
        var.sets("");
        wassert(actual(var.enqc()) == "");
        var.sets("ciao");
        wassert(actual(var.enqc()) == "ciao");
        var.sets("ciao!");
        wassert(actual(var.enqc()) == "ciao!");
        var.sets("ciaone");
        wassert(actual(var.enqc()) == "ciaon");

        Var var1(&vi, var);
        wassert(actual(var1.enqc()) == "ciaon");

        Var var2(var);
        wassert(actual(var2.enqc()) == "ciaon");

        Var var3(std::move(var2));
        wassert(actual(var3.enqc()) == "ciaon");
    });

    add_method("issue17", []() {
        _Varinfo vi;
        varinfo::set_bufr(vi, WR_VAR(0, 0, 0), "TEST", "?", 16, 0, 2);
        Var var(&vi);
        wassert(var.sets("None"));
        wassert_false(var.isset());
        wassert(var.setc("None"));
        wassert_false(var.isset());
    });

    add_method("issue42", []() {
        const Vartable* table = Vartable::get_bufr("B0000000000000031000");
        Varinfo info          = table->query(WR_VAR(0, 13, 11));
        Var var(info, 0);
        wassert(actual(var.enqd()) == 0);
        var.seti(-1);
        wassert(actual(var.enqd()) == -0.1);
    });
}

} // namespace
