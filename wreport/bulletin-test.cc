// SPDX-License-Identifier: GPL-2.0-only

#include "bulletin.h"
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
} test("bulletin");

void Tests::register_tests()
{
    add_method("obtain_subset", [] {
        unique_ptr<BufrBulletin> b(BufrBulletin::create());
        wassert(actual(b->subsets.size()) == 0u);
        try
        {
            b->obtain_subset(0);
            throw TestFailed("error_consistency was not thrown");
        }
        catch (error_consistency& e)
        {
            wassert(actual(e.what()).contains("BUFR/CREX tables not loaded"));
        }
    });

    add_method("copy_without_subsets_bufr", [] {
        auto orig  = sys::read_file(datafile("bufr/obs1-13.36.bufr"));
        auto bufr  = BufrBulletin::decode(orig);
        auto bufr1 = bufr->copy_without_subsets();
        wassert(actual(bufr1->diff_header(*bufr)) == 0u);
    });

    add_method("copy_without_subsets_crex", [] {
        auto orig  = sys::read_file(datafile("crex/test-synop0.crex"));
        auto crex  = CrexBulletin::decode(orig);
        auto crex1 = crex->copy_without_subsets();
        crex1->subsets.emplace_back(crex->subsets[0]);
        wassert(actual(crex1->diff_header(*crex)) == 0u);
    });

    add_method("append_subset", [] {
        auto orig  = sys::read_file(datafile("bufr/obs1-13.36.bufr"));
        auto bufr  = BufrBulletin::decode(orig);
        auto bufr1 = bufr->copy_without_subsets();
        bufr1->append_subset(bufr->subset(0));
        auto encoded = bufr1->encode();
        auto decoded = BufrBulletin::decode(encoded);
        wassert(actual(decoded->diff(*bufr)) == 0u);
    });

    add_method("append_subset_with_bitmaps", [] {
        auto orig  = sys::read_file(datafile("bufr/bufr1"));
        auto bufr  = BufrBulletin::decode(orig);
        auto bufr1 = bufr->copy_without_subsets();
        bufr1->append_subset(bufr->subset(0));
        auto encoded = bufr1->encode();
        auto decoded = BufrBulletin::decode(encoded);
        wassert(actual(decoded->diff(*bufr)) == 0u);
    });

    add_method("append_subset_with_alterations", [] {
        auto orig  = sys::read_file(datafile("bufr/ascat1.bufr"));
        auto bufr  = BufrBulletin::decode(orig);
        auto bufr1 = bufr->copy_without_subsets();
        bufr1->append_subset(bufr->subset(0));
        auto encoded = bufr1->encode();
        auto decoded = BufrBulletin::decode(encoded);
        wassert(actual(decoded->subset(0).diff(bufr->subset(0))) == 0u);
    });

    add_method("append_subset_with_chardata", [] {
        auto orig  = sys::read_file(datafile("bufr/C05060.bufr"));
        auto bufr  = BufrBulletin::decode(orig);
        auto bufr1 = bufr->copy_without_subsets();
        bufr1->append_subset(bufr->subset(0));
        auto encoded = bufr1->encode();
        auto decoded = BufrBulletin::decode(encoded);
        wassert(actual(decoded->diff(*bufr)) == 0u);
    });

    add_method("append_subset_with_substituted_data", [] {
        auto orig  = sys::read_file(datafile("bufr/C23000.bufr"));
        auto bufr  = BufrBulletin::decode(orig);
        auto bufr1 = bufr->copy_without_subsets();
        bufr1->append_subset(bufr->subset(0));
        auto encoded = bufr1->encode();
        auto decoded = BufrBulletin::decode(encoded);
        wassert(actual(decoded->diff(*bufr)) == 0u);
    });

    add_method("append_subset_with_unknown_local_descriptor", [] {
        auto orig  = sys::read_file(datafile("bufr/C06006.bufr"));
        auto bufr  = BufrBulletin::decode(orig);
        auto bufr1 = bufr->copy_without_subsets();
        bufr1->append_subset(bufr->subset(0));
        auto encoded = bufr1->encode();
        auto decoded = BufrBulletin::decode(encoded);
        wassert(actual(decoded->diff(*bufr)) == 0u);
    });
}

} // namespace
