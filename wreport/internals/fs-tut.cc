#include "tests.h"
#include "fs.h"
#include <set>

using namespace wibble::tests;
using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

typedef test_group<> test_group;
typedef test_group::Test Test;
typedef test_group::Fixture Fixture;

std::vector<Test> tests {
    Test("open", [](Fixture& f) {
        // Open a directory that exists
        const char* testdatadir = getenv("WREPORT_TESTDATA");
        wassert(actual(testdatadir != nullptr).istrue());

        // Open
        fs::Directory d(testdatadir);

        // Check if exists() works
        wassert(actual(d.exists()).istrue());

        // Check if stat() works
        struct stat st;
        wrunchecked(d.stat(st));
        wassert(actual(S_ISDIR(st.st_mode)).istrue());
    }),
    Test("notfound", [](Fixture& f) {
        // Open a directory that does not exist
        fs::Directory d("does-not-exist");

        // Constructor succeeds but exists() returns false
        wassert(actual(d.exists()).isfalse());

        // Iterating it gives an empty sequence
        wassert(actual(d.begin() == d.end()).istrue());
    }),
    Test("iter", [](Fixture& f) {
        // Iterate a directory that exists
        const char* testdatadir = getenv("WREPORT_TESTDATA");
        wassert(actual(testdatadir != nullptr).istrue());

        fs::Directory d(testdatadir);
        set<string> files;
        for (auto& i: d)
            files.insert(i.d_name);

        wassert(actual(files.size()) > 4);
        wassert(actual(files.find("test-bufr-table.txt") != files.end()).istrue());
    }),
};

test_group newtg("internals_fs", tests);

}

