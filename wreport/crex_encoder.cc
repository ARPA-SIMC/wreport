/*
 * wreport/bulletin - CREX encoder
 *
 * Copyright (C) 2005--2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "config.h"

#include "opcode.h"
#include "bulletin.h"

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>	/* isspace */
#include <stdlib.h>	/* malloc */
#include <string.h>	/* memcpy */
#include <math.h>	/* NAN */
#include <assert.h>	/* NAN */
#include <errno.h>	/* NAN */

// #define TRACE_ENCODER

#ifdef TRACE_ENCODER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {

namespace {

/// Iterate variables in a subset, one at a time
struct Varqueue
{
	const Subset& subset;
	unsigned cur;

	Varqueue(const Subset& subset) : subset(subset), cur(0) {}

	const bool empty() const { return cur >= subset.size(); }
	const unsigned size() const { return subset.size() - cur; }
	const Var& peek() const { return subset[cur]; }
	const Var& pop() { return subset[cur++]; }
};

struct Outbuf
{
    std::string& buf;

    /* True if the CREX message uses the check digit feature */
    int has_check_digit;
    /* Value of the next expected check digit */
    int expected_check_digit;


    Outbuf(std::string& buf) : buf(buf), has_check_digit(0), expected_check_digit(0)
    {
    }

    void raw_append(const char* str, int len)
    {
        buf.append(str, len);
    }

    void raw_appendf(const char* fmt, ...) __attribute__ ((format(printf, 2, 3)))
    {
        char sbuf[256];
        va_list ap;
        va_start(ap, fmt);
        int len = vsnprintf(sbuf, 255, fmt, ap);
        va_end(ap);

        buf.append(sbuf, len);
    }

    void encode_check_digit()
    {
        if (!has_check_digit) return;

        char c = '0' + expected_check_digit;
        raw_append(&c, 1);
        expected_check_digit = (expected_check_digit + 1) % 10;
    }

    void append_missing(Varinfo info)
    {
        TRACE("encode_b missing len: %d\n", info->len);
        for (unsigned i = 0; i < info->len; i++)
            raw_append("/", 1);
    }

    void append_var(Varinfo info, const Var& var)
    {
        if (var.value() == NULL)
            return append_missing(info);

        int len = info->len;
        raw_append(" ", 1);
        encode_check_digit();

        if (info->is_string()) {
            raw_appendf("%-*.*s", len, len, var.value());
            TRACE("encode_b string len: %d val %-*.*s\n", len, len, len, var.value());
        } else {
            int val = var.enqi();

            /* FIXME: here goes handling of active C table modifiers */

            if (val < 0) ++len;

            raw_appendf("%0*d", len, val);
            TRACE("encode_b num len: %d val %0*d\n", len, len, val);
        }
    }
};

struct DDSEncoder : public bulletin::ConstBaseVisitor
{
    Outbuf& ob;

    DDSEncoder(const Bulletin& b, Outbuf& ob) : ConstBaseVisitor(b), ob(ob) {}
    virtual ~DDSEncoder() {}

    void do_start_subset(unsigned subset_no, const Subset& current_subset)
    {
        TRACE("start_subset %u\n", subset_no);
        bulletin::ConstBaseVisitor::do_start_subset(subset_no, current_subset);

        /* Encode the subsection terminator */
        if (subset_no > 0)
            ob.raw_append("+\r\r\n", 4);
    }

    virtual void do_attr(Varinfo info, unsigned var_pos, Varcode attr_code)
    {
        throw error_unimplemented("encode_attr");
    }

    virtual void do_var(Varinfo info)
    {
        const Var& var = get_var();
        IFTRACE {
            TRACE("encode_var ");
            var.print(stderr);
        }
        ob.append_var(info, var);
    }

    virtual Var do_semantic_var(Varinfo info)
    {
        const Var& var = get_var();
        IFTRACE {
            TRACE("encode_semantic_var ");
            var.print(stderr);
        }
        switch (info->var)
        {
            case WR_VAR(0, 31, 1):
            case WR_VAR(0, 31, 2):
            case WR_VAR(0, 31, 11):
            case WR_VAR(0, 31, 12):
            {
                unsigned count = var.enqi();

                /* Encode the repetition count */
                ob.raw_append(" ", 1);
                ob.encode_check_digit();
                ob.raw_appendf("%04u", count);
                break;
            }
            default:
                ob.append_var(info, var);
                break;
        }
        return var;
    }

    virtual const Var* do_bitmap(Varcode code, Varcode delayed_code, const Opcodes& ops)
    {
        throw error_unimplemented("do_bitmap");
    }

    virtual void do_char_data(Varcode code)
    {
        throw error_unimplemented("do_char_data");
    }

    virtual void do_associated_field(unsigned bit_count, unsigned significance)
    {
        // Do nothing: CREX does not have associated fields
        //throw error_unimplemented("do_associated_field");
    }
};


struct Encoder
{
    /* Input message data */
    const CrexBulletin& in;
    /* Output decoded variables */
    Outbuf out;

	/* Offset of the start of CREX section 1 */
	int sec1_start;
	/* Offset of the start of CREX section 2 */
	int sec2_start;
	/* Offset of the start of CREX section 3 */
	int sec3_start;
	/* Offset of the start of CREX section 4 */
	int sec4_start;

	/* Subset we are encoding */
	const Subset* subset;

	Encoder(const CrexBulletin& in, std::string& out)
		: in(in), out(out),
		  sec1_start(0), sec2_start(0), sec3_start(0), sec4_start(0),
		  subset(0)
	{
	}

    void encode_sec1()
    {
        out.raw_appendf("T%02d%02d%02d A%03d%03d",
                in.master_table_number,
                in.edition,
                in.table,
                in.type,
                in.localsubtype);

        /* Encode the data descriptor section */

		for (vector<Varcode>::const_iterator i = in.datadesc.begin();
				i != in.datadesc.end(); ++i)
		{
			char prefix;
			switch (WR_VAR_F(*i))
			{
				case 0: prefix = 'B'; break;
				case 1: prefix = 'R'; break;
				case 2: prefix = 'C'; break;
				case 3: prefix = 'D'; break;
				default: prefix = '?'; break;
			}

			// Don't put delayed replication counts in the data section
			if (WR_VAR_F(*i) == 0 && WR_VAR_X(*i) == 31 && WR_VAR_Y(*i) < 3)
				continue;

            out.raw_appendf(" %c%02d%03d", prefix, WR_VAR_X(*i), WR_VAR_Y(*i));
        }

        if (out.has_check_digit)
        {
            out.raw_append(" E", 2);
            out.expected_check_digit = 1;
        }

        out.raw_append("++\r\r\n", 5);
    }

    void run()
    {
        /* Encode section 0 */
        out.raw_append("CREX++\r\r\n", 9);

        /* Encode section 1 */
        sec1_start = out.buf.size();
        encode_sec1();
        TRACE("SEC1 encoded as [[[%s]]]", out.buf.substr(sec1_start).c_str());

        /* Encode section 2 */
        sec2_start = out.buf.size();

        DDSEncoder e(in, out);
        in.visit(e);
        out.raw_append("++\r\r\n", 5);

        TRACE("SEC2 encoded as [[[%s]]]", out.buf.substr(sec2_start).c_str());

        /* Encode section 3 */
        sec3_start = out.buf.size();
        /* Nothing to do, as we have no custom section */

        /* Encode section 4 */
        sec4_start = out.buf.size();
        out.raw_append("7777\r\r\n", 7);
    }
};


} // Unnamed namespace

void CrexBulletin::encode(std::string& buf) const
{
	Encoder e(*this, buf);
	e.run();
	//out.encoding = CREX;
}

} // bufrex namespace

/* vim:set ts=4 sw=4: */
