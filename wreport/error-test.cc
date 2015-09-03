#include "tests.h"
#include "error.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("notfound", []() {
            try {
                throw error_notfound("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_NOTFOUND);
                wassert(actual(string(e.what())) == "foo");
            }

            try {
                error_notfound::throwf("%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_NOTFOUND);
                wassert(actual(string(e.what())) == "42");
            }
        });

        add_method("type", []() {
            try {
                throw error_type("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_TYPE);
                wassert(actual(string(e.what())) == "foo");
            }

            try {
                error_type::throwf("%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_TYPE);
                wassert(actual(string(e.what())) == "42");
            }
        });

        add_method("alloc", []() {
            try {
                throw error_alloc("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_ALLOC);
                wassert(actual(string(e.what())) == "foo");
            }
        });

        add_method("handles", []() {
            try {
                throw error_handles("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_HANDLES);
                wassert(actual(string(e.what())) == "foo");
            }

            try {
                error_handles::throwf("%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_HANDLES);
                wassert(actual(string(e.what())) == "42");
            }
        });

        add_method("toolong", []() {
            try {
                throw error_toolong("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_TOOLONG);
                wassert(actual(string(e.what())) == "foo");
            }

            try {
                error_toolong::throwf("%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_TOOLONG);
                wassert(actual(string(e.what())) == "42");
            }
        });

        add_method("system", []() {
            try {
                throw error_system("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_SYSTEM);
                wassert(actual(string(e.what()).substr(0, 5)) == "foo: ");
            }

            try {
                error_system::throwf("%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_SYSTEM);
                wassert(actual(string(e.what()).substr(0, 4)) == "42: ");
            }
        });

        add_method("consistency", []() {
            try {
                throw error_consistency("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_CONSISTENCY);
                wassert(actual(string(e.what())) == "foo");
            }

            try {
                error_consistency::throwf("%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_CONSISTENCY);
                wassert(actual(string(e.what())) == "42");
            }
        });

        add_method("parse", []() {
            try {
                throw error_parse("file", 42, "foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_PARSE);
                wassert(actual(string(e.what())) == "file:42: foo");
            }

            try {
                error_parse::throwf("file", 42, "%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_PARSE);
                wassert(actual(string(e.what())) == "file:42: 42");
            }
        });

        add_method("regexp", []() {
            // TODO: setup a test case involving a regexp
        });

        add_method("unimplemented", []() {
            try {
                throw error_unimplemented("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_UNIMPLEMENTED);
                wassert(actual(string(e.what())) == "foo");
            }

            try {
                error_unimplemented::throwf("%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_UNIMPLEMENTED);
                wassert(actual(string(e.what())) == "42");
            }
        });

        add_method("domain", []() {
            try {
                throw error_domain("foo");
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_DOMAIN);
                wassert(actual(string(e.what())) == "foo");
            }

            try {
                error_domain::throwf("%d", 42);
            } catch (error& e) {
                wassert(actual(e.code()) == WR_ERR_DOMAIN);
                wassert(actual(string(e.what())) == "42");
            }
        });
    }
} tests("error");

}
