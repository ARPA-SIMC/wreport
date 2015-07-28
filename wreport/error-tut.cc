#include "tests.h"
#include "error.h"

using namespace wibble::tests;
using namespace wibble;
using namespace wreport;
using namespace wreport::tests;
using namespace std;
using namespace wibble;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("notfound", [](Fixture& f) {
        try {
            throw error_notfound("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_NOTFOUND);
            ensure_equals(string(e.what()), "foo");
        }

        try {
            error_notfound::throwf("%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_NOTFOUND);
            ensure_equals(string(e.what()), "42");
        }
    }),
    Test("type", [](Fixture& f) {
        try {
            throw error_type("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_TYPE);
            ensure_equals(string(e.what()), "foo");
        }

        try {
            error_type::throwf("%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_TYPE);
            ensure_equals(string(e.what()), "42");
        }
    }),
    Test("alloc", [](Fixture& f) {
        try {
            throw error_alloc("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_ALLOC);
            ensure_equals(string(e.what()), "foo");
        }
    }),
    Test("handles", [](Fixture& f) {
        try {
            throw error_handles("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_HANDLES);
            ensure_equals(string(e.what()), "foo");
        }

        try {
            error_handles::throwf("%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_HANDLES);
            ensure_equals(string(e.what()), "42");
        }
    }),
    Test("toolong", [](Fixture& f) {
        try {
            throw error_toolong("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_TOOLONG);
            ensure_equals(string(e.what()), "foo");
        }

        try {
            error_toolong::throwf("%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_TOOLONG);
            ensure_equals(string(e.what()), "42");
        }
    }),
    Test("system", [](Fixture& f) {
        try {
            throw error_system("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_SYSTEM);
            ensure_equals(string(e.what()).substr(0, 5), "foo: ");
        }

        try {
            error_system::throwf("%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_SYSTEM);
            ensure_equals(string(e.what()).substr(0, 4), "42: ");
        }
    }),
    Test("consistency", [](Fixture& f) {
        try {
            throw error_consistency("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_CONSISTENCY);
            ensure_equals(string(e.what()), "foo");
        }

        try {
            error_consistency::throwf("%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_CONSISTENCY);
            ensure_equals(string(e.what()), "42");
        }
    }),
    Test("parse", [](Fixture& f) {
        try {
            throw error_parse("file", 42, "foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_PARSE);
            ensure_equals(string(e.what()), "file:42: foo");
        }

        try {
            error_parse::throwf("file", 42, "%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_PARSE);
            ensure_equals(string(e.what()), "file:42: 42");
        }
    }),
    Test("regexp", [](Fixture& f) {
        // TODO: setup a test case involving a regexp
    }),
    Test("unimplemented", [](Fixture& f) {
        try {
            throw error_unimplemented("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_UNIMPLEMENTED);
            ensure_equals(string(e.what()), "foo");
        }

        try {
            error_unimplemented::throwf("%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_UNIMPLEMENTED);
            ensure_equals(string(e.what()), "42");
        }
    }),
    Test("domain", [](Fixture& f) {
        try {
            throw error_domain("foo");
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_DOMAIN);
            ensure_equals(string(e.what()), "foo");
        }

        try {
            error_domain::throwf("%d", 42);
        } catch (error& e) {
            ensure_equals(e.code(), WR_ERR_DOMAIN);
            ensure_equals(string(e.what()), "42");
        }
    }),
};

test_group newtg("error", tests);

}
