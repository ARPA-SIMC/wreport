#include "dds-validator.h"
#include "wreport/tests.h"
#include "wreport/utils/string.h"
#include <set>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

void validate(Bulletin& b)
{
    try
    {
        // Validate them
        for (unsigned i = 0; i < b.subsets.size(); ++i)
        {
            bulletin::DDSValidator validator(b, i);
            validator.run();
        }
    }
    catch (std::exception& e1)
    {
        try
        {
            b.print_structured(stderr);
        }
        catch (std::exception& e2)
        {
            cerr << "dump interrupted: " << e2.what() << endl;
        }
        throw TestFailed(e1.what());
    }
}

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        // Ensure that the validator works against normal bufr messages
        add_method("bufr", []() {
            std::set<std::string> blacklist;
            blacklist.insert("bufr/gen-generic.bufr");
            blacklist.insert("bufr/obs255-255.0.bufr");
            blacklist.insert("bufr/tempforecast.bufr");
            blacklist.insert("bufr/bad-edition.bufr");
            blacklist.insert("bufr/corrupted.bufr");
            blacklist.insert("bufr/test-soil1.bufr");
            blacklist.insert("bufr/short0.bufr");
            blacklist.insert("bufr/short1.bufr");
            blacklist.insert("bufr/short2.bufr");
            blacklist.insert("bufr/short3.bufr");
            blacklist.insert("bufr/issue58.bufr");

            auto files = tests::all_test_files("bufr");
            for (const auto& i : files)
            {
                if (str::startswith(i, "bufr/afl-"))
                    continue;
                WREPORT_TEST_INFO(test_info);
                if (blacklist.find(i) != blacklist.end())
                    continue;

                // Read the whole contents of the test file
                std::string raw1 = tests::slurpfile(i);

                // Decode the original contents
                unique_ptr<BufrBulletin> msg1 =
                    wcallchecked(BufrBulletin::decode(raw1, i.c_str()));

                // Validate them
                test_info() << i;
                wassert(validate(*msg1));
            }
        });
        // Ensure that the validator works against normal crex messages
        add_method("crex", []() {
            std::set<std::string> blacklist;
            blacklist.insert("crex/test-temp0.crex");

            auto files = tests::all_test_files("crex");
            for (const auto& i : files)
            {
                WREPORT_TEST_INFO(test_info);
                if (blacklist.find(i) != blacklist.end())
                    continue;

                // Read the whole contents of the test file
                std::string raw1 = tests::slurpfile(i);

                // Decode the original contents
                unique_ptr<CrexBulletin> msg1 =
                    wcallchecked(CrexBulletin::decode(raw1, i.c_str()));

                // Validate them
                test_info() << i;
                wassert(validate(*msg1));
            }
        });
    }
} test("bulletin_dds_validator");

} // namespace
