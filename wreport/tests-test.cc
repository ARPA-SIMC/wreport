#include "tests.h"
#include "vartable.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("vars", []() {
            // Test variable comparisons
            const Vartable* table = Vartable::get_bufr("B0000000000000014000");
            Var tempundef(table->query(WR_VAR(0, 12, 101)));
            Var temp12(table->query(WR_VAR(0, 12, 101)), 12.5);
            Var temp13(table->query(WR_VAR(0, 12, 101)), 13.5);
            wassert(actual(tempundef) == tempundef);
            wassert(actual(tempundef) != temp12);
            wassert(actual(tempundef) != temp13);
            wassert(actual(temp12) != tempundef);
            wassert(actual(temp12) == temp12);
            wassert(actual(temp12) != temp13);
            wassert(actual(temp13) != tempundef);
            wassert(actual(temp13) != temp12);
            wassert(actual(temp13) == temp13);

            Var tempundefa1(tempundef);
            Var tempundefa2(tempundef);
            tempundefa1.seta(
                unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 75)));
            tempundefa2.seta(
                unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 50)));

            wassert(actual(tempundef) != tempundefa1);
            wassert(actual(tempundef) != tempundefa2);
            wassert(actual(tempundefa1) == tempundefa1);
            wassert(actual(tempundefa1) != tempundefa2);
            wassert(actual(tempundefa2) != tempundefa1);
            wassert(actual(tempundefa2) == tempundefa2);

            Var temp12a1(temp12);
            Var temp12a2(temp12);
            temp12a1.seta(
                unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 75)));
            temp12a2.seta(
                unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 50)));

            wassert(actual(temp12) != temp12a1);
            wassert(actual(temp12) != temp12a2);
            wassert(actual(temp12a1) == temp12a1);
            wassert(actual(temp12a1) != temp12a2);
            wassert(actual(temp12a2) != temp12a1);
            wassert(actual(temp12a2) == temp12a2);

            Var tempa1(temp12);
            Var tempa2(temp12);
            tempa1.seta(
                unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 7)), 1)));
            tempa2.seta(
                unique_ptr<Var>(new Var(table->query(WR_VAR(0, 33, 9)), 1)));
            wassert(actual(tempa1) == tempa1);
            wassert(actual(tempa1) != tempa2);
            wassert(actual(tempa2) != tempa1);
            wassert(actual(tempa2) == tempa2);

            Var test_i(table->query(WR_VAR(0, 1, 1)), 42);
            wassert(actual(test_i) == 42);
            wassert(actual(test_i) == 42.0);
            wassert(actual(test_i) == "42");
            wassert(actual(test_i) != 24);

            Var test_d(table->query(WR_VAR(0, 1, 14)), 0.5);
            wassert(actual(test_d) == 0.5);
            wassert(actual(test_d) == 50);
            wassert(actual(test_d) == "50");
            wassert(actual(test_d) != 5.0);

            Var test_s(table->query(WR_VAR(0, 1, 19)), "foo");
            wassert(actual(test_s) == "foo");
            wassert(actual(test_s) != "bar");
        });
    }
} test("tests");

} // namespace
