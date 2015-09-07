#include "tableinfo.h"
#include "error.h"

// Uncomment to trace approximate table matching
// #define TRACE_MATCH

using namespace std;

namespace wreport {

bool BufrTableID::operator<(const BufrTableID& o) const
{
    if (originating_centre < o.originating_centre) return true;
    if (originating_centre > o.originating_centre) return false;
    if (originating_subcentre < o.originating_subcentre) return true;
    if (originating_subcentre > o.originating_subcentre) return false;
    if (master_table_number < o.master_table_number) return true;
    if (master_table_number > o.master_table_number) return false;
    if (master_table_version_number < o.master_table_version_number) return true;
    if (master_table_version_number > o.master_table_version_number) return false;
    if (master_table_version_number_local < o.master_table_version_number_local) return true;
    if (master_table_version_number_local > o.master_table_version_number_local) return false;
    return false;
}

bool BufrTableID::is_acceptable_replacement(const BufrTableID& id) const
{
    if (id.master_table_number != master_table_number)
        return false;
    if (id.master_table_version_number < master_table_version_number)
        return false;
    return true;
}

bool BufrTableID::is_acceptable_replacement(const CrexTableID& id) const
{
    return false;
}

namespace {

template<typename Base, typename First, typename Second>
struct Compare
{
    const Base& base;
    const First& first;
    const Second& second;
    // -1: first < second, 0: first == second, 1: first > second
    int res = 0;

#ifdef TRACE_MATCH
    int trace(int res, const char* reason)
    {
        fprintf(stderr, "closest to "); base.print(stderr);
        fprintf(stderr, " between "); first.print(stderr);
        fprintf(stderr, " and "); second.print(stderr);
        if (res < 0)
            fprintf(stderr, " is the first: %s\n", reason);
        else if (res > 0)
            fprintf(stderr, " is the second: %s\n", reason);
        else
            fprintf(stderr, " is none of them: %s\n", reason);
        return res;
    }

#define decide_first(reason) do { res = trace(-1, reason); return true; } while (0)
#define decide_second(reason) do { res = trace(1, reason); return true; } while (0)
#define decide_same(reason) do { res = trace(0, reason); return true; } while (0)
#else
#define decide_first(reason) do { res = -1; return true; } while (0)
#define decide_second(reason) do { res = 1; return true; } while (0)
#define decide_same(reason) do { res = 0; return true; } while (0)
#endif

    Compare(const Base& base, const First& first, const Second& second)
        : base(base), first(first), second(second) {}

    bool compare_mtv()
    {
        // We only get acceptable candidates, so we have a guarantee that both mt
        // version numbers are higher than what we want, and we can just pick the
        // closest (lowest)
        if (first.master_table_version_number < second.master_table_version_number)
            decide_first("closer to the master_table we want");
        else if (second.master_table_version_number < first.master_table_version_number)
            decide_second("closer to the master_table we want");
        // mt version numbers are the same
        return false;
    }

    bool compare_mtv_bufr()
    {
        // We only get acceptable candidates, so we have a guarantee that both BUFR
        // mt version numbers are higher than what we want, and we can just pick
        // the closest (lowest)
        if (first.master_table_version_number_bufr < second.master_table_version_number_bufr)
            decide_first("closer to the bufr master_table we want");
        else if (second.master_table_version_number_bufr < first.master_table_version_number_bufr)
            decide_second("closer to the bufr master_table we want");
        // bufr mt version numbers are the same
        return false;
    }

    bool compare_mtv_bufrcrex()
    {
        // Both are acceptable candidates, pick the best one

        // We only get acceptable candidates, so we have a guarantee that both mt
        // version numbers are higher than what we want, and we can just pick the
        // closest (lowest)
        if (first.master_table_version_number < second.master_table_version_number_bufr)
            decide_first("orig bufr master table is closer to the master table we want");
        else if (second.master_table_version_number_bufr < first.master_table_version_number)
            decide_second("crex bufr master table is closer to the master_table we want");
        // mt version numbers are the same
        return false;
    }

    bool compare_centre()
    {
        auto centre_match = [](uint16_t wanted, uint16_t cand) {
            if (wanted == cand) return 3;
            if (cand == 0) return 2;
            if (cand == 0xffff) return 1;
            return 0;
        };

        unsigned first_centre_match = centre_match(base.originating_centre, first.originating_centre);
        unsigned second_centre_match = centre_match(base.originating_centre, second.originating_centre);
        if (first_centre_match > second_centre_match)
            decide_first("better match on the centre");
        if (second_centre_match > first_centre_match)
            decide_second("better match on the centre");

        // first_centre_match and second_centre_match are the same

        if (first_centre_match != 3)
            decide_same("no good match on centres");

        return false;
    }

    bool compare_subcentre()
    {
        if (first.originating_subcentre == base.originating_subcentre)
            if (second.originating_subcentre == base.originating_subcentre)
                decide_same("they also have the same originating subcentre");
            else
                decide_first("exact match on originating subcentre");
        else
            if (second.originating_subcentre == base.originating_subcentre)
                decide_second("exact match on originating subcentre");
            else
                decide_same("none of them matches the originating subcentre");

        return false;
    }

    bool compare_mt_local()
    {
        if (base.master_table_version_number_local != 0)
        {
            if (first.master_table_version_number_local < base.master_table_version_number_local)
                if (second.master_table_version_number_local < base.master_table_version_number_local)
                {
                    if (first.master_table_version_number_local < second.master_table_version_number_local)
                        decide_second("closest match for local master table version number");
                    if (first.master_table_version_number_local > second.master_table_version_number_local)
                        decide_first("closest match for local master table version number");
                }
                else
                    decide_second("valid candidate for local master table version number");
            else
                if (second.master_table_version_number_local < base.master_table_version_number_local)
                    decide_first("valid candidate for local master table version number");
                else
                {
                    // Look for the closest match for the local table: they are
                    // both >= the one we want, so pick the smallest
                    if (first.master_table_version_number_local < second.master_table_version_number_local)
                        decide_first("closest match for local master table version number");
                    if (first.master_table_version_number_local > second.master_table_version_number_local)
                        decide_second("closest match for local master table version number");
                }
        }
        return false;
    }
};

template<typename Base, typename First, typename Second>
Compare<Base, First, Second> compare(const Base& base, const First& first, const Second& second)
{
    return Compare<Base, First, Second>(base, first, second);
}

}

int BufrTableID::closest_match(const BufrTableID& first, const BufrTableID& second) const
{
    auto comp = compare(*this, first, second);

    if (comp.compare_mtv()) return comp.res;
    if (comp.compare_centre()) return comp.res;

    // They have the exact originating centres, look at local table versions
    if (comp.compare_mt_local()) return comp.res;

    // They have the same master_table_version_number_local: try to match the
    // exact subcentre
    if (comp.compare_subcentre()) return comp.res;

    return 0;
}

int BufrTableID::closest_match(const CrexTableID& first, const CrexTableID& second) const
{
    return 0;
}

int BufrTableID::closest_match(const BufrTableID& first, const CrexTableID& second) const
{
    return -1;
}


void BufrTableID::print(FILE* out) const
{
    fprintf(out, "BUFR(%03hu:%02hu, %02hhu:%02hhu:%02hhu)",
        originating_centre, originating_subcentre,
        master_table_number, master_table_version_number,
        master_table_version_number_local);
}


bool CrexTableID::operator<(const CrexTableID& o) const
{
    if (edition_number < o.edition_number) return true;
    if (edition_number > o.edition_number) return false;
    if (originating_centre < o.originating_centre) return true;
    if (originating_centre > o.originating_centre) return false;
    if (originating_subcentre < o.originating_subcentre) return true;
    if (originating_subcentre > o.originating_subcentre) return false;
    if (master_table_number < o.master_table_number) return true;
    if (master_table_number > o.master_table_number) return false;
    if (master_table_version_number < o.master_table_version_number) return true;
    if (master_table_version_number > o.master_table_version_number) return false;
    if (master_table_version_number_local < o.master_table_version_number_local) return true;
    if (master_table_version_number_local > o.master_table_version_number_local) return false;
    if (master_table_version_number_bufr < o.master_table_version_number_bufr) return true;
    if (master_table_version_number_bufr > o.master_table_version_number_bufr) return false;
    return false;
}

bool CrexTableID::is_acceptable_replacement(const BufrTableID& id) const
{
    // Master table number must be the same
    if (id.master_table_number != master_table_number)
        return false;

    // Edition must be greater or equal to what we want
    if (id.master_table_version_number < master_table_version_number_bufr)
        return false;

    return true;
}

bool CrexTableID::is_acceptable_replacement(const CrexTableID& id) const
{
    // Edition number must be the same
    if (id.edition_number != edition_number)
        return false;

    // Master table number must be the same
    if (id.master_table_number != master_table_number)
        return false;

    // Master table version number most be greater or equal than what we want
    if (id.master_table_version_number < master_table_version_number)
        return false;

    // BUFR master table version number most be greater or equal than what we want
    if (id.master_table_version_number_bufr < master_table_version_number_bufr)
        return false;

    return true;
}

int CrexTableID::closest_match(const BufrTableID& first, const BufrTableID& second) const
{
    auto comp = compare(*this, first, second);

    if (comp.compare_mtv()) return comp.res;
    if (comp.compare_centre()) return comp.res;

    // They have the exact originating centres, look at local table versions
    if (comp.compare_mt_local()) return comp.res;

    // They have the same master_table_version_number_local: try to match the
    // exact subcentre
    if (comp.compare_subcentre()) return comp.res;

    return 0;
}

int CrexTableID::closest_match(const CrexTableID& first, const CrexTableID& second) const
{
    auto comp = compare(*this, first, second);

    if (comp.compare_mtv()) return comp.res;
    if (comp.compare_mtv_bufr()) return comp.res;
    if (comp.compare_centre()) return comp.res;

    // They have the exact originating centres, look at local table versions
    if (comp.compare_mt_local()) return comp.res;

    // They have the same master_table_version_number_local: try to match the
    // exact subcentre
    if (comp.compare_subcentre()) return comp.res;

    return 0;
}

int CrexTableID::closest_match(const BufrTableID& first, const CrexTableID& second) const
{
    auto comp = compare(*this, first, second);

    if (comp.compare_mtv_bufrcrex()) return comp.res;
    if (comp.compare_centre()) return comp.res;

    // They have the exact originating centres, look at local table versions
    if (comp.compare_mt_local()) return comp.res;

    // They have the same master_table_version_number_local: try to match the
    // exact subcentre
    if (comp.compare_subcentre()) return comp.res;

    return 0;
}

void CrexTableID::print(FILE* out) const
{
    fprintf(out, "CREX(%02hhu, %02hu:%02hu, %02hhu:%02hhu:%02hhu:%02hhu)",
        edition_number, originating_centre, originating_subcentre,
        master_table_number, master_table_version_number,
        master_table_version_number_local,
        master_table_version_number_bufr);
}

}
