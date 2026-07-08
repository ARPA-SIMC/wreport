#include "bulletin.h"
#include "tests.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("obtain_subset", []() {
            unique_ptr<BufrBulletin> b(BufrBulletin::create());
            wassert(actual(b->subsets.size()) == 0u);
            try
            {
                b->obtain_subset(0);
                throw TestFailed("error_consistency was not thrown");
            }
            catch (error_consistency& e)
            {
                wassert(
                    actual(e.what()).contains("BUFR/CREX tables not loaded"));
            }
        });
    }
} test("bulletin");

} // namespace
