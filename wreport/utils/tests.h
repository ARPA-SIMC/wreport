#ifndef WREPORT_TESTS_H
#define WREPORT_TESTS_H

/**
 * @author Enrico Zini <enrico@enricozini.org>, Peter Rockai (mornfall) <me@mornfall.net>
 * @brief Utility functions for the unit tests
 *
 * Copyright (C) 2006--2007  Peter Rockai (mornfall) <me@mornfall.net>
 * Copyright (C) 2003--2013  Enrico Zini <enrico@debian.org>
 */

#include <string>
#include <sstream>
#include <exception>
#include <functional>
#include <vector>

namespace wreport {
namespace tests {
struct Location;
struct LocationInfo;
}
}

/*
 * These global arguments will be shadowed by local variables in functions that
 * implement tests.
 *
 * They are here to act as default root nodes to fulfill method signatures when
 * tests are called from outside other tests.
 */
extern const wreport::tests::LocationInfo wreport_test_location_info;

namespace wreport {
namespace tests {

/**
 * Add information to the test backtrace for the tests run in the current
 * scope.
 *
 * Example usage:
 * \code
 * test_function(...)
 * {
 *    WREPORT_TEST_INFO(info);
 *    for (unsigned i = 0; i < 10; ++i)
 *    {
 *       info() << "Iteration #" << i;
 *       ...
 *    }
 * }
 * \endcode
 */
struct LocationInfo : public std::stringstream
{
    LocationInfo() {}

    /**
     * Clear the current information and return the output stream to which new
     * information can be sent
     */
    std::ostream& operator()();
};

/// Information about one stack frame in the test execution stack
struct TestStackFrame
{
    const char* file;
    int line;
    const char* call;
    std::string local_info;

    TestStackFrame(const char* file, int line, const char* call)
        : file(file), line(line), call(call)
    {
    }

    TestStackFrame(const char* file, int line, const char* call, const LocationInfo& local_info)
        : file(file), line(line), call(call), local_info(local_info.str())
    {
    }

    void format(std::ostream& out) const;
};

/**
 * Exception raised when a test assertion fails, normally by
 * Location::fail_test
 */
struct TestFailed : public std::exception
{
    std::string message;
    std::vector<TestStackFrame> stack;

    TestFailed(const std::exception& e);

    template<typename ...Args>
    TestFailed(const std::exception& e, Args&&... args)
        : TestFailed(e)
    {
        add_stack_info(std::forward<Args>(args)...);
    }

    TestFailed(const std::string& message) : message(message) {}

    template<typename ...Args>
    TestFailed(const std::string& message, Args&&... args)
        : TestFailed(message)
    {
        add_stack_info(std::forward<Args>(args)...);
    }

    const char* what() const noexcept override { return message.c_str(); }

    template<typename ...Args>
    void add_stack_info(Args&&... args) { stack.emplace_back(std::forward<Args>(args)...); }

    /// Return the formatted backtrace for this location
    std::string backtrace() const;

    /// Write the formatted backtrace for this location to \a out
    void backtrace(std::ostream& out) const;
};

#if 0
#define WREPORT_TESTS_ALWAYS_THROWS __attribute__ ((noreturn))

    /**
     * Return a message describing a test failure, including the current
     * backtrace
     */
    std::string fail_msg(const std::string& error) const;

    /**
     * Return a message describing a test failure, including the current
     * backtrace
     */
    std::string fail_msg(std::function<void(std::ostream&)> write_error) const;

    /// Raise TestFailed for the given error message
    void fail_test(const std::string& error) const WREPORT_TESTS_ALWAYS_THROWS;

    /// Raise TestFailed with error message written by the given function
    void fail_test(std::function<void(std::ostream&)> write_error) const WREPORT_TESTS_ALWAYS_THROWS;
#endif

/**
 * Use this to declare a local variable with the given name that will be
 * picked up by tests as extra local info
 */
#define WREPORT_TEST_INFO(name) \
    wreport::tests::LocationInfo wreport_test_location_info; \
    wreport::tests::LocationInfo& name = wreport_test_location_info


/// Test function that ensures that the actual value is true
template<typename A>
void assert_true(const A& actual)
{
    if (actual) return;
    std::stringstream ss;
    ss << "actual value " << actual << " is not true";
    throw TestFailed(ss.str());
};

void assert_true(std::nullptr_t actual);

/// Test function that ensures that the actual value is false
template<typename A>
void assert_false(const A& actual)
{
    if (!actual) return;
    std::stringstream ss;
    ss << "actual value " << actual << " is not false";
    throw TestFailed(ss.str());
};

void assert_false(std::nullptr_t actual);

/**
 * Test function that ensures that the actual value is the same as a reference
 * one
 */
template<typename A, typename E>
void assert_equal(const A& actual, const E& expected)
{
    if (actual == expected) return;
    std::stringstream ss;
    ss << "value '" << actual << "' is different than the expected '" << expected << "'";
    throw TestFailed(ss.str());
}

/**
 * Test function that ensures that the actual value is different than a
 * reference one
 */
template<typename A, typename E>
void assert_not_equal(const A& actual, const E& expected)
{
    if (actual != expected) return;
    std::stringstream ss;
    ss << "value '" << actual << "' is not different than the expected '" << expected << "'";
    throw TestFailed(ss.str());
}

/// Ensure that the actual value is less than the reference value
template<typename A, typename E>
void assert_less(const A& actual, const E& expected)
{
    if (actual < expected) return;
    std::stringstream ss;
    ss << "value '" << actual << "' is not less than the expected '" << expected << "'";
    throw TestFailed(ss.str());
}

/// Ensure that the actual value is less or equal than the reference value
template<typename A, typename E>
void assert_less_equal(const A& actual, const E& expected)
{
    if (actual <= expected) return;
    std::stringstream ss;
    ss << "value '" << actual << "' is not less than or equals to the expected '" << expected << "'";
    throw TestFailed(ss.str());
}

/// Ensure that the actual value is greater than the reference value
template<typename A, typename E>
void assert_greater(const A& actual, const E& expected)
{
    if (actual > expected) return;
    std::stringstream ss;
    ss << "value '" << actual << "' is not greater than the expected '" << expected << "'";
    throw TestFailed(ss.str());
}

/// Ensure that the actual value is greather or equal than the reference value
template<typename A, typename E>
void assert_greater_equal(const A& actual, const E& expected)
{
    if (actual >= expected) return;
    std::stringstream ss;
    ss << "value '" << actual << "' is not greater than or equals to the expected '" << expected << "'";
    throw TestFailed(ss.str());
}

/// Ensure that the string \a actual starts with \a expected
void assert_startswith(const std::string& actual, const std::string& expected);

/// Ensure that the string \a actual ends with \a expected
void assert_endswith(const std::string& actual, const std::string& expected);

/// Ensure that the string \a actual contains \a expected
void assert_contains(const std::string& actual, const std::string& expected);

/// Ensure that the string \a actual does not contain \a expected
void assert_not_contains(const std::string& actual, const std::string& expected);


template<class A>
struct Actual
{
    A actual;
    Actual(const A& actual) : actual(actual) {}
    ~Actual() {}

    void istrue() const { assert_true(actual); }
    void isfalse() const { assert_false(actual); }
    template<typename E> void operator==(const E& expected) const { assert_equal(actual, expected); }
    template<typename E> void operator!=(const E& expected) const { assert_not_equal(actual, expected); }
    template<typename E> void operator<(const E& expected) const { return assert_less(actual, expected); }
    template<typename E> void operator<=(const E& expected) const { return assert_less_equal(actual, expected); }
    template<typename E> void operator>(const E& expected) const { return assert_greater(actual, expected); }
    template<typename E> void operator>=(const E& expected) const { return assert_greater_equal(actual, expected); }
};

struct ActualCString
{
    const char* actual;
    ActualCString(const char* s) : actual(s) {}

    void istrue() const { return assert_true(actual); }
    void isfalse() const { return assert_false(actual); }
    void operator==(const char* expected) const;
    void operator==(const std::string& expected) const;
    void operator!=(const char* expected) const;
    void operator!=(const std::string& expected) const;
    void operator<(const std::string& expected) const;
    void operator<=(const std::string& expected) const;
    void operator>(const std::string& expected) const;
    void operator>=(const std::string& expected) const;
    void startswith(const std::string& expected) const;
    void endswith(const std::string& expected) const;
    void contains(const std::string& expected) const;
    void not_contains(const std::string& expected) const;
};

struct ActualStdString : public Actual<std::string>
{
    ActualStdString(const std::string& s) : Actual<std::string>(s) {}

//    void operator==(const std::string& expected) const { assert_equal(actual, expected); }
//    void operator!=(const std::string& expected) const { assert_not_equal(actual, expected); }
//    void operator<(const std::string& expected) const { assert_less(actual, expected); }
//    void operator<=(const std::string& expected) const { return TestLte<std::string, std::string>(actual, expected); }
//    void operator>(const std::string& expected) const { return TestGt<std::string, std::string>(actual, expected); }
//    void operator>=(const std::string& expected) const { return TestGte<std::string, std::string>(actual, expected); }
    void startswith(const std::string& expected) const;
    void endswith(const std::string& expected) const;
    void contains(const std::string& expected) const;
    void not_contains(const std::string& expected) const;
//    TestRegexp matches(const std::string& regexp) const { return TestRegexp(actual, regexp); }
//    TestFileExists fileexists() const { return TestFileExists(actual); }
};

struct ActualDouble : public Actual<double>
{
    using Actual::Actual;

    void almost_equal(double expected, unsigned places) const;
    void not_almost_equal(double expected, unsigned places) const;
};

template<typename A>
inline Actual<A> actual(const A& actual) { return Actual<A>(actual); }
inline ActualCString actual(const char* actual) { return ActualCString(actual); }
inline ActualCString actual(char* actual) { return ActualCString(actual); }
inline ActualStdString actual(const std::string& actual) { return ActualStdString(actual); }
inline ActualDouble actual(double actual) { return ActualDouble(actual); }


/**
 * Run the given command, raising TestFailed with the appropriate backtrace
 * information if it threw an exception.
 *
 * If the command raises TestFailed, it adds the current stack to its stack
 * information.
 */
#define wassert(...) \
    do { try { \
        __VA_ARGS__ ; \
    } catch (TestFailed& e) { \
        e.add_stack_info(__FILE__, __LINE__, #__VA_ARGS__, wreport_test_location_info); \
        throw; \
    } catch (std::exception& e) { \
        throw TestFailed(e, __FILE__, __LINE__, #__VA_ARGS__, wreport_test_location_info); \
    } } while(0)

/**
 * Call a function returning its result, and raising TestFailed with the
 * appropriate backtrace information if it threw an exception.
 *
 * If the function raises TestFailed, it adds the current stack to its stack
 * information.
 */
#define wcallchecked(func) \
    [&]() { try { \
        return func; \
    } catch (TestFailed& e) { \
        e.add_stack_info(__FILE__, __LINE__, #func, wreport_test_location_info); \
        throw; \
    } catch (std::exception& e) { \
        throw TestFailed(e, __FILE__, __LINE__, #func, wreport_test_location_info); \
    } }()


struct TestCase;

/**
 * Result of running a test method.
 */
struct TestMethodResult
{
    /// Name of the test case
    std::string test_case;

    /// Name of the test method
    std::string test_method;

    /// If non-empty, the test failed with this error
    std::string error_message;

    /// If non-empty, the test raised an exception and this is its type ID
    std::string exception_typeid;

    /// True if the test has been skipped
    bool skipped = false;


    TestMethodResult(const std::string& test_case, const std::string& test_method)
        : test_case(test_case), test_method(test_method) {}

    void set_failed(TestFailed& e)
    {
        error_message = e.what();
        if (error_message.empty())
            error_message = "test failed with an empty error message";
    }

    void set_exception(std::exception& e)
    {
        error_message = e.what();
        if (error_message.empty())
            error_message = "test threw an exception with an empty error message";
        exception_typeid = typeid(e).name();
    }

    void set_unknown_exception()
    {
        error_message = "unknown exception caught";
    }

    void set_setup_exception(std::exception& e)
    {
        error_message = "[setup failed: ";
        error_message += e.what();
        error_message += "]";
    }

    void set_teardown_exception(std::exception& e)
    {
        error_message = "[teardown failed: ";
        error_message += e.what();
        error_message += "]";
    }

    bool is_success() const
    {
        return error_message.empty();
    }
};

/**
 * Result of running a whole test case
 */
struct TestCaseResult
{
    /// Name of the test case
    std::string test_case;
    /// Outcome of all the methods that have been run
    std::vector<TestMethodResult> methods;
    /// Set to a non-empty string if the setup method of the test case failed
    std::string fail_setup;
    /// Set to a non-empty string if the teardown method of the test case
    /// failed
    std::string fail_teardown;
    /// Set to true if this test case has been skipped
    bool skipped = false;

    TestCaseResult(const std::string& test_case) : test_case(test_case) {}

    void set_setup_failed()
    {
        fail_setup = "test case setup method threw an unknown exception";
    }

    void set_setup_failed(std::exception& e)
    {
        fail_setup = "test case setup method threw an exception: ";
        fail_setup += e.what();
    }

    void set_teardown_failed()
    {
        fail_teardown = "test case teardown method threw an unknown exception";
    }

    void set_teardown_failed(std::exception& e)
    {
        fail_teardown = "test case teardown method threw an exception: ";
        fail_teardown += e.what();
    }

    void add_test_method(TestMethodResult&& e)
    {
        methods.emplace_back(std::move(e));
    }

    bool is_success() const
    {
        if (!fail_setup.empty() || !fail_teardown.empty()) return false;
        for (const auto& m: methods)
            if (!m.is_success())
                return false;
        return true;
    }
};

/**
 * Abstract interface for the objects that supervise test execution.
 *
 * This can be used for printing progress, or to skip test methods or test
 * cases.
 */
struct TestController
{
    virtual ~TestController() {}

    /**
     * Called before running a test case.
     *
     * @returns true if the test case should be run, false if it should be skipped
     */
    virtual bool test_case_begin(const TestCaseResult& test_case) { return true; }

    /**
     * Called after running a test case.
     */
    virtual void test_case_end(const TestCaseResult& test_case) {}

    /**
     * Called before running a test method.
     *
     * @returns true if the test method should be run, false if it should be skipped
     */
    virtual bool test_method_begin(const TestMethodResult& test_method) { return true; }

    /**
     * Called after running a test method.
     */
    virtual void test_method_end(const TestMethodResult& test_method) {}
};

/**
 * Simple default implementation of TestController.
 *
 * It does progress printing to stdout and basic glob-based test method
 * filtering.
 */
struct SimpleTestController : public TestController
{
    /// Any method not matching this glob expression will not be run
    std::string whitelist;

    /// Any method matching this glob expression will not be run
    std::string blacklist;

    bool test_case_begin(const TestCaseResult& test_case) override;
    void test_case_end(const TestCaseResult& test_case) override;
    bool test_method_begin(const TestMethodResult& test_method) override;
    void test_method_end(const TestMethodResult& test_method) override;
};


/**
 * Test registry.
 *
 * It collects information about all known test cases and takes care of running
 * them.
 */
struct TestRegistry
{
    /// All known test cases
    std::vector<TestCase*> entries;

    /**
     * Register a new test case.
     *
     * No memory management is done: test_case needs to exist for the whole
     * lifetime of TestRegistry.
     */
    void register_test_case(TestCase& test_case);

    /**
     * Run all the registered tests using the given controller
     */
    std::vector<TestCaseResult> run_tests(TestController& controller);

    /// Get the singleton instance of TestRegistry
    static TestRegistry& get();
};

/**
 * Test method information
 */
struct TestMethod
{
    /// Name of the test method
    std::string name;

    /// Main body of the test method
    std::function<void()> test_function;

    TestMethod(const std::string& name, std::function<void()> test_function)
        : name(name), test_function(test_function) {}
};


/**
 * Test case collecting several test methods, and self-registering with the
 * singleton instance of TestRegistry.
 */
struct TestCase
{
    /// Name of the test case
    std::string name;

    /// All registered test methods
    std::vector<TestMethod> methods;

    TestCase(const std::string& name)
        : name(name)
    {
        TestRegistry::get().register_test_case(*this);
    }
    virtual ~TestCase() {}

    /**
     * This will be called before running the test case, to populate it with
     * its test methods.
     *
     * This needs to be reimplemented with a function that will mostly be a
     * sequence of calls to add_method().
     */
    virtual void register_tests() = 0;

    /**
     * Set up the test case before it is run.
     */
    virtual void setup() {}

    /**
     * Clean up after the test case is run
     */
    virtual void teardown() {}

    /**
     * Set up before the test method is run
     */
    virtual void method_setup(TestMethodResult&) {}

    /**
     * Clean up after the test method is run
     */
    virtual void method_teardown(TestMethodResult&) {}

    /**
     * Call setup(), run all the tests that have been registered, then
     * call teardown().
     *
     * Exceptions in setup() and teardown() are caught and reported in
     * TestCaseResult. Test are run using run_test().
     */
    virtual TestCaseResult run_tests(TestController& controller);

    /**
     * Run a test method.
     *
     * Call method_setup(), run all the tests that have been registered, then
     * call method_teardown().
     *
     * Exceptions thrown by the test method are caught and reported in
     * TestMethodResult.
     *
     * Exceptions in method_setup() and method_teardown() are caught and
     * reported in TestMethodResult.
     */
    virtual TestMethodResult run_test(TestController& controller, TestMethod& method);

    /**
     * Register a new test method
     */
    template<typename ...Args>
    void add_method(const std::string& name, std::function<void()> test_function)
    {
        methods.emplace_back(name, test_function);
    }

    /**
     * Register a new test method
     */
    template<typename ...Args>
    void add_method(const std::string& name, std::function<void()> test_function, Args&&... args)
    {
        methods.emplace_back(name, test_function, std::forward<Args>(args)...);
    }

    /**
     * Register a new test metheod, with arguments.
     *
     * Any extra arguments to the function will be passed to the test method.
     */
    template<typename FUNC, typename ...Args>
    void add_method(const std::string& name, FUNC test_function, Args&&... args)
    {
        methods.emplace_back(name, [test_function, args...]() { test_function(args...); });
    }
};


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
    virtual ~Fixture() {}

    // Called before each test
    virtual void test_setup() {}

    // Called after each test
    virtual void test_teardown() {}
};

/**
 * Test case that includes a fixture
 */
template<typename Fixture>
struct FixtureTestCase : public TestCase
{
    Fixture* fixture = 0;
    std::function<Fixture*()> make_fixture;

    template<typename... Args>
    FixtureTestCase(const std::string& name, Args... args)
        : TestCase(name)
    {
        make_fixture = [&]() { return new Fixture(&args...); };
    }

    void setup() override
    {
        TestCase::setup();
        fixture = make_fixture();
    }

    void teardown() override
    {
        delete fixture;
        fixture = 0;
        TestCase::teardown();
    }

    void method_setup(TestMethodResult& mr) override
    {
        TestCase::method_setup(mr);
        if (fixture) fixture.test_setup();
    }

    void method_teardown(TestMethodResult& mr) override
    {
        if (fixture) fixture.test_teardown();
        TestCase::method_teardown(mr);
    }

    /**
     * Add a method that takes a reference to the fixture as argument.
     *
     * Any extra arguments to the function will be passed to the test method
     * after the fixture.
     */
    template<typename FUNC, typename ...Args>
    void add_method(const std::string& name, FUNC test_function, Args&&... args)
    {
        methods.emplace_back(name, [this, test_function, args...] { test_function(*fixture, args...); });
    }
};

#if 0
    struct Test
    {
        std::string name;
        std::function<void()> test_func;
    };

    /// Add tests to the test case
    virtual void add_tests() {}
#endif


}
}

#endif
