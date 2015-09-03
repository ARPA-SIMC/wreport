#include "tests.h"
#include "fs.h"
#include <set>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("open", []() {
            // Open a directory that exists
            const char* testdatadir = getenv("WREPORT_TESTDATA");
            wassert(actual(testdatadir != nullptr).istrue());

            // Open
            fs::Directory d(testdatadir);

            // Check if exists() works
            wassert(actual(d.exists()).istrue());

            // Check if stat() works
            struct stat st;
            wassert(d.stat(st));
            wassert(actual(S_ISDIR(st.st_mode)).istrue());
        });
        add_method("notfound", []() {
            // Open a directory that does not exist
            fs::Directory d("does-not-exist");

            // Constructor succeeds but exists() returns false
            wassert(actual(d.exists()).isfalse());

            // Iterating it gives an empty sequence
            wassert(actual(d.begin() == d.end()).istrue());
        });
        add_method("iter", []() {
            // Iterate a directory that exists
            const char* testdatadir = getenv("WREPORT_TESTDATA");
            wassert(actual(testdatadir != nullptr).istrue());

            fs::Directory d(testdatadir);
            set<string> files;
            for (auto& i: d)
                files.insert(i.d_name);

            wassert(actual(files.size()) > 4u);
            wassert(actual(files.find("test-bufr-table.txt") != files.end()).istrue());
        });
    }
} test("internals_fs");

}

