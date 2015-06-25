/*
 * wreport/tests - Unit test utilities
 *
 * Copyright (C) 2005--2013  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#ifndef WREPORT_TESTS
#define WREPORT_TESTS

#include <wibble/tests.h>
#include <wreport/varinfo.h>
#include <wreport/var.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>

#define wcallchecked(func) \
    [&]() { try { \
        return func; \
    } catch (tut::failure) { \
        throw; \
    } catch (std::exception& e) { \
        wibble_test_location.fail_test(wibble_test_location_info, __FILE__, __LINE__, #func, e.what()); \
    } }()

namespace wreport {
namespace tests {

/**
 * Base class for test fixtures.
 *
 * A fixture will have a constructor and a destructor to do setup/teardown, and
 * a reset() function to be called inbetween tests.
 *
 * Fixtures do not need to descend from Fixture: this implementation is
 * provided as a default for tests that do not need one, or as a base for
 * fixtures that do not need reset().
 */
struct Fixture
{
    // Reset inbetween tests
    void reset() {}
};

/**
 * Check if a test can be run.
 *
 * This is used to implement extra test filtering features like glob matching
 * on group or test names.
 */
bool test_can_run(const std::string& group_name, const std::string& test_name);

/**
 * Alternative to tut::test_group.
 *
 * Tests are registered using a vector of lambdas, and it implements smarter
 * matching on group names and test names.
 *
 * There can be any number of tests.
 *
 * This could not have been implemented on top of the existing tut::test_group,
 * because all its working components are declared private and are not
 * accessible to subclasses.
 */
template<typename T=Fixture>
struct test_group : public tut::group_base
{
    typedef std::function<void(T&)> test_func_t;
    typedef T Fixture;

    struct Test
    {
        std::string name;
        test_func_t func;

        Test(const std::string& name, test_func_t func)
            : name(name), func(func) {}
    };
    typedef std::vector<Test> Tests;

    /// Name of this test group
    std::string name;
    /// Storage for all our tests.
    Tests tests;
    /// Current test when running them in sequence
    typename std::vector<Test>::iterator cur_test;
    /// Fixture for the tests
    T* fixture = 0;

    test_group(const char* name, const std::vector<Test>& tests)
        : name(name), tests(tests)
    {
        // register itself
        tut::runner.get().register_group(name, this);
    }

    virtual T* create_fixture()
    {
        return new T;
    }

    void delete_fixture()
    {
        try {
            delete fixture;
        } catch (std::exception& e) {
            fprintf(stderr, "Warning: exception caught while deleting fixture for test group %s: %s", name.c_str(), e.what());
        }
        fixture = 0;
    }

    void rewind() override
    {
        delete_fixture();
        cur_test = tests.begin();
    }

    tut::test_result run_next() override
    {
        // Skip those tests that should not run
        while (cur_test != tests.end() && !test_can_run(name, cur_test->name))
            ++cur_test;

        if (cur_test == tests.end())
        {
            delete_fixture();
            throw tut::no_more_tests();
        }

        tut::test_result res = run(cur_test);
        ++cur_test;
        return res;
    }

    // execute one test
    tut::test_result run_test(int n) override
    {
        --n; // From 1-based to 0-based
        if (n < 0 || (size_t)n >= tests.size())
            throw tut::beyond_last_test();

        if (!test_can_run(name, tests[n].name))
            throw tut::beyond_last_test();

        tut::test_result res = run(tests.begin() + n);
        delete_fixture();
        return res;
    }

    tut::test_result run(typename std::vector<Test>::iterator test)
    {
        int pos = test - tests.begin() + 1;
        // Create fixture if it does not exist yet
        if (!fixture)
        {
            try {
                fixture = create_fixture();
            } catch (std::exception& e) {
                fprintf(stderr, "Warning: exception caught while creating fixture for test group %s: %s", name.c_str(), e.what());
                return tut::test_result(name, pos, tut::test_result::ex_ctor, e);
            }
        } else {
            try {
                fixture->reset();
            } catch (std::exception& e) {
                fprintf(stderr, "Warning: exception caught while resetting the fixture for test group %s: %s", name.c_str(), e.what());
                return tut::test_result(name, pos, tut::test_result::ex_ctor, e);
            }
        }
        try {
            test->func(*fixture);
        } catch (const tut::failure& e) {
            // test failed because of ensure() or similar method
            return tut::test_result(name, pos, tut::test_result::fail, e);
        } catch (const std::exception& e) {
            // test failed with std::exception
            return tut::test_result(name, pos, tut::test_result::ex, e);
        } catch (...) {
            // test failed with unknown exception
            return tut::test_result(name, pos, tut::test_result::ex);
        }

        return tut::test_result(name, pos, tut::test_result::ok);
    }
};


#ifndef ensure_contains
#define ensure_contains(x, y) wreport::tests::impl_ensure_contains(wibble::tests::Location(__FILE__, __LINE__, #x " == " #y), (x), (y))
#define inner_ensure_contains(x, y) wreport::tests::impl_ensure_contains(wibble::tests::Location(loc, __FILE__, __LINE__, #x " == " #y), (x), (y))
static inline void impl_ensure_contains(const wibble::tests::Location& loc, const std::string& haystack, const std::string& needle)
{
	if( haystack.find(needle) == std::string::npos )
	{
		std::stringstream ss;
		ss << "'" << haystack << "' does not contain '" << needle << "'";
		throw tut::failure(loc.msg(ss.str()));
	}
}

#define ensure_not_contains(x, y) arki::tests::impl_ensure_not_contains(wibble::tests::Location(__FILE__, __LINE__, #x " == " #y), (x), (y))
#define inner_ensure_not_contains(x, y) arki::tests::impl_ensure_not_contains(wibble::tests::Location(loc, __FILE__, __LINE__, #x " == " #y), (x), (y))
static inline void impl_ensure_not_contains(const wibble::tests::Location& loc, const std::string& haystack, const std::string& needle)
{
	if( haystack.find(needle) != std::string::npos )
	{
		std::stringstream ss;
		ss << "'" << haystack << "' must not contain '" << needle << "'";
		throw tut::failure(loc.msg(ss.str()));
	}
}
#endif

#define ensure_varcode_equals(x, y) wreport::tests::_ensure_varcode_equals(wibble::tests::Location(__FILE__, __LINE__, #x " == " #y), (x), (y))
#define inner_ensure_varcode_equals(x, y) wreport::tests::_ensure_varcode_equals(wibble::tests::Location(loc, __FILE__, __LINE__, #x " == " #y), (x), (y))
static inline void _ensure_varcode_equals(const wibble::tests::Location& loc, Varcode actual, Varcode expected)
{
	if( expected != actual )
	{
		char buf[40];
		snprintf(buf, 40, "expected %01d%02d%03d actual %01d%02d%03d",
				WR_VAR_F(expected), WR_VAR_X(expected), WR_VAR_Y(expected),
				WR_VAR_F(actual), WR_VAR_X(actual), WR_VAR_Y(actual));
		throw tut::failure(loc.msg(buf));
	}
}

#define ensure_var_equals(x, y) wreport::tests::_ensure_var_equals(wibble::tests::Location(__FILE__, __LINE__, #x " == " #y), (x), (y))
#define inner_ensure_var_equals(x, y) wreport::tests::_ensure_var_equals(wibble::tests::Location(loc, __FILE__, __LINE__, #x " == " #y), (x), (y))
static inline void _ensure_var_equals(const wibble::tests::Location& loc, const Var& var, int val)
{
	inner_ensure_equals(var.enqi(), val);
}
static inline void _ensure_var_equals(const wibble::tests::Location& loc, const Var& var, double val)
{
	inner_ensure_equals(var.enqd(), val);
}
static inline void _ensure_var_equals(const wibble::tests::Location& loc, const Var& var, const std::string& val)
{
	inner_ensure_equals(std::string(var.enqc()), val);
}

#define ensure_var_undef(x) wreport::tests::_ensure_var_undef(wibble::tests::Location(__FILE__, __LINE__, #x " is undef"), (x))
#define inner_ensure_var_undef(x) wreport::tests::_ensure_var_undef(wibble::tests::Location(loc, __FILE__, __LINE__, #x " is undef"), (x))
static inline void _ensure_var_undef(const wibble::tests::Location& loc, const Var& var)
{
	inner_ensure_equals(var.value(), (const char*)0);
}

/// RAII-style override of an environment variable
class LocalEnv
{
	/// name of the environment variable that we override
	std::string key;
	/// stored original value of the variable
	std::string oldVal;
public:
	/**
	 * @param key the environment variable to override
	 * @param val the new value to assign to \a key
	 */
	LocalEnv(const std::string& key, const std::string& val)
		: key(key)
	{
		const char* v = getenv(key.c_str());
		oldVal = v == NULL ? "" : v;
		setenv(key.c_str(), val.c_str(), 1);
	}
	~LocalEnv()
	{
		setenv(key.c_str(), oldVal.c_str(), 1);
	}
};

#ifdef wassert
/// Check that actual and expected have the same vars
struct TestVarEqual
{
    Var avar;
    Var evar;
    bool inverted;

    TestVarEqual(const Var& actual, const Var& expected, bool inverted=false) : avar(actual), evar(expected), inverted(inverted) {}
    TestVarEqual operator!() { return TestVarEqual(avar, evar, !inverted); }

    void check(WIBBLE_TEST_LOCPRM) const;
};

struct ActualVar : public wibble::tests::Actual<Var>
{
    ActualVar(const Var& actual) : wibble::tests::Actual<Var>(actual) {}

    TestVarEqual operator==(const Var& expected) const { return TestVarEqual(actual, expected); }
    TestVarEqual operator!=(const Var& expected) const { return TestVarEqual(actual, expected, true); }
};
#endif

}
}

#ifdef wassert
namespace wibble {
namespace tests {

inline wreport::tests::ActualVar actual(const wreport::Var& actual) { return wreport::tests::ActualVar(actual); }

}
}
#endif

#endif
