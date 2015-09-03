#include "tests.h"
#include "options.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("localoverride", []() {
            int a = 1;
            {
                options::LocalOverride<int> o(a, 2);
                wassert(actual(a) == 2);
                wassert(actual(o.old_value) == 1);
            }
            wassert(actual(a) == 1);
        });
    }
} tests("options");

}
