#include "options.h"
#include "tests.h"
#include "utils/sys.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} tests("options");

void Tests::register_tests()
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

    add_method("mastertableoverride_as_int", []() {
        options::MasterTableVersionOverride mto(42);
        wassert(actual(mto) == 42);

        mto = 12;
        wassert(actual(mto) == 12);

        mto = options::MasterTableVersionOverride::NONE;
        wassert(actual(mto) == options::MasterTableVersionOverride::NONE);

        mto = options::MasterTableVersionOverride::NEWEST;
        wassert(actual(mto) == options::MasterTableVersionOverride::NEWEST);
    });

    add_method("mastertableoverride_from_env", []() {
        {
            sys::OverrideEnvironment oe("WREPORT_MASTER_TABLE_VERSION");
            options::MasterTableVersionOverride mto;
            wassert(actual(mto) == options::MasterTableVersionOverride::NONE);
        }

        {
            sys::OverrideEnvironment oe("WREPORT_MASTER_TABLE_VERSION",
                                        "newest");
            options::MasterTableVersionOverride mto;
            wassert(actual(mto) == options::MasterTableVersionOverride::NEWEST);
        }

        {
            sys::OverrideEnvironment oe("WREPORT_MASTER_TABLE_VERSION", "42");
            options::MasterTableVersionOverride mto;
            wassert(actual(mto) == 42);
        }

        {
            sys::OverrideEnvironment oe("WREPORT_MASTER_TABLE_VERSION",
                                        "invalid");
            options::MasterTableVersionOverride mto;
            wassert(actual(mto) == options::MasterTableVersionOverride::NONE);
        }

        options::MasterTableVersionOverride from_env;
        wassert(actual(options::var_master_table_version_override) == from_env);
    });
}

} // namespace
