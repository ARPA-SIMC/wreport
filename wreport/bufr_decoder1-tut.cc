/*
 * Copyright (C) 2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#include <test-utils-wreport.h>

using namespace wreport;
using namespace std;

namespace tut {

struct bufr_decoder1_shar
{
    bufr_decoder1_shar()
    {
    }

    ~bufr_decoder1_shar()
    {
    }
};
TESTGRP(bufr_decoder1);

typedef tests::MsgTester<BufrBulletin> MsgTester;

template<> template<>
void to::test<1>()
{
    struct Tester : public MsgTester {
        void test(const BufrBulletin& msg)
        {
            ensure_equals(msg.edition, 3);
            ensure_equals(msg.type, 1);
            ensure_equals(msg.subtype, 255);
            ensure_equals(msg.localsubtype, 0);
            ensure_equals(msg.subsets.size(), 1);

            /*
            const Subset& s = msg.subset(0);
            ensure_equals(s.size(), 35u);

            ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
            ensure_equals(s[9].enqd(), 68.27);
            ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
            ensure_equals(s[10].enqd(),  9.68);

            ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

            ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
            */
        }
    } test;

    test.run("bufr/gts-buoy1.bufr");
}

template<> template<>
void to::test<2>()
{
    struct Tester : public MsgTester {
        void test(const BufrBulletin& msg)
        {
            ensure_equals(msg.edition, 4);
            ensure_equals(msg.type, 0);
            ensure_equals(msg.subtype, 1);
            ensure_equals(msg.localsubtype, 0);
            ensure_equals(msg.subsets.size(), 25);

            /*
            const Subset& s = msg.subset(0);
            ensure_equals(s.size(), 35u);

            ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
            ensure_equals(s[9].enqd(), 68.27);
            ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
            ensure_equals(s[10].enqd(),  9.68);

            ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

            ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
            */
        }
    } test;

    test.run("bufr/gts-synop-rad1.bufr");
}

template<> template<>
void to::test<3>()
{
    struct Tester : public MsgTester {
        void test(const BufrBulletin& msg)
        {
            ensure_equals(msg.edition, 4);
            ensure_equals(msg.type, 0);
            ensure_equals(msg.subtype, 6);
            ensure_equals(msg.localsubtype, 150);
            ensure_equals(msg.subsets.size(), 1);

            /*
            const Subset& s = msg.subset(0);
            ensure_equals(s.size(), 35u);

            ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
            ensure_equals(s[9].enqd(), 68.27);
            ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
            ensure_equals(s[10].enqd(),  9.68);

            ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

            ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
            */
        }
    } test;

    test.run("bufr/gts-synop-rad2.bufr");
}

template<> template<>
void to::test<4>()
{
    struct Tester : public MsgTester {
        void test(const BufrBulletin& msg)
        {
            ensure_equals(msg.edition, 4);
            ensure_equals(msg.type, 0);
            ensure_equals(msg.subtype, 1);
            ensure_equals(msg.localsubtype, 0);
            ensure_equals(msg.subsets.size(), 1);

            /*
            const Subset& s = msg.subset(0);
            ensure_equals(s.size(), 35u);

            ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
            ensure_equals(s[9].enqd(), 68.27);
            ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
            ensure_equals(s[10].enqd(),  9.68);

            ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

            ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
            */
        }
    } test;

    test.run("bufr/gts-synop-tchange.bufr");
}

template<> template<>
void to::test<5>()
{
    struct Tester : public MsgTester {
        void test(const BufrBulletin& msg)
        {
            ensure_equals(msg.edition, 4);
            ensure_equals(msg.type, 0);
            ensure_equals(msg.subtype, 1);
            ensure_equals(msg.localsubtype, 0);
            ensure_equals(msg.subsets.size(), 1);

            /*
            const Subset& s = msg.subset(0);
            ensure_equals(s.size(), 35u);

            ensure_varcode_equals(s[9].code(), WR_VAR(0, 5, 2));
            ensure_equals(s[9].enqd(), 68.27);
            ensure_varcode_equals(s[10].code(), WR_VAR(0, 6, 2));
            ensure_equals(s[10].enqd(),  9.68);

            ensure(s[0].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[0].enqa(WR_VAR(0, 33, 7))->enqi(), 70);

            ensure(s[1].enqa(WR_VAR(0, 33, 7)) != NULL);
            ensure_equals(s[1].enqa(WR_VAR(0, 33, 7))->enqi(), 70);
            */
        }
    } test;

    test.run("bufr/new-003.bufr");
}

}
