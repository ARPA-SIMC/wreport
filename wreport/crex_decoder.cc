/*
 * wreport/bulletin - CREX decoder
 *
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#include "bulletin/buffers.h"

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>	/* isspace */
#include <stdlib.h>	/* malloc */
#include <string.h>	/* memcpy */
#include <math.h>	/* NAN */
#include <assert.h>	/* NAN */
#include <errno.h>	/* NAN */

// #define TRACE_DECODER

#ifdef TRACE_DECODER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {

namespace {
struct Decoder
{
    /// Input data
    bulletin::CrexInput in;

	/* Output decoded variables */
	CrexBulletin& out;

	/* Current subset we are decoding */
	Subset* current_subset;

	/* Value of the next expected check digit */
	int expected_check_digit;

    Decoder(const std::string& in, const char* fname, size_t offset, CrexBulletin& out)
        : in(in), out(out), current_subset(0),
          expected_check_digit(0)
    {
        this->in.fname = fname;
        this->in.offset = offset;
    }

    void decode_header()
    {
        /* Read crex section 0 (Indicator section) */
        in.check_available_data(6, "initial header of CREX message");
        if (strncmp((const char*)in.cur, "CREX++", 6) != 0)
            in.parse_error("data does not start with CREX header (\"%.6s\" was read instead)", in.cur);

        in.skip_data_and_spaces(6);
        TRACE(" -> is CREX\n");

        /* Read crex section 1 (Data description section) */
        in.mark_section_start(1);

        /* T<version> */
        if (*in.cur != 'T')
            in.parse_error("version not found in CREX data description");

        {
            char edition[11];
            in.read_word(edition, 11);
            if (sscanf(edition, "T%02d%02d%02d",
                        &(out.master_table_number),
                        &(out.edition),
                        &(out.table)) != 3)
                error_consistency::throwf("Edition (%s) is not in format Ttteevv", edition);
            TRACE(" -> edition %d\n", strtol(edition + 1, 0, 10));
        }

        /* A<atable code> */
        in.check_eof("A code");
        if (*in.cur != 'A')
            in.parse_error("A Table informations not found in CREX data description");
        {
            char atable[20];
            in.read_word(atable, 20);
            TRACE("ATABLE \"%s\"\n", atable);
            int val = strtol(atable+1, 0, 10);
            switch (strlen(atable)-1)
            {
                case 3:
                    out.type = val;
                    out.subtype = 255;
                    out.localsubtype = 0;
                    TRACE(" -> category %d\n", strtol(atable, 0, 10));
                    break;
                case 6:
                    out.type = val / 1000;
                    out.subtype = 255;
                    out.localsubtype = val % 1000;
                    TRACE(" -> category %d, subcategory %d\n", val / 1000, val % 1000);
                    break;
                default:
                    error_consistency::throwf("Cannot parse an A table indicator %zd digits long", strlen(atable));
            }
        }

        /* data descriptors followed by (E?)\+\+ */
        in.check_eof("data descriptor section");

        out.has_check_digit = 0;
        while (1)
        {
            if (*in.cur == 'B' || *in.cur == 'R' || *in.cur == 'C' || *in.cur == 'D')
            {
                in.check_available_data(6, "one data descriptor");
                out.datadesc.push_back(descriptor_code(in.cur));
                in.skip_data_and_spaces(6);
            }
            else if (*in.cur == 'E')
            {
                out.has_check_digit = 1;
                expected_check_digit = 1;
                in.skip_data_and_spaces(1);
            }
            else if (*in.cur == '+')
            {
                in.check_available_data(1, "end of data descriptor section");
                if (*(in.cur+1) != '+')
                    in.parse_error("data descriptor section ends with only one '+'");
                in.skip_data_and_spaces(2);
                break;
            }
        }
        IFTRACE{
            TRACE(" -> data descriptor section:");
            for (vector<Varcode>::const_iterator i = out.datadesc.begin();
                    i != out.datadesc.end(); ++i)
                TRACE(" %01d%02d%03d", WR_VAR_F(*i), WR_VAR_X(*i), WR_VAR_Y(*i));
            TRACE("\n");
        }
    }

    void decode_data()
    {
        // Load tables and set category/subcategory
        out.load_tables();

        /* Decode crex section 2 (data section) */
        in.mark_section_start(2);

        // Scan the various subsections
        for (int i = 0; ; ++i)
        {
            current_subset = &out.obtain_subset(i);
            parse_data_section(Opcodes(out.datadesc));
            in.skip_spaces();
            in.check_eof("end of data section");

            if (*in.cur != '+')
                in.parse_error("there should be a '+' at the end of the data section");
            ++in.cur;

            /* Peek at the next character to see if it's end of section */
            in.check_eof("end of data section");
            if (*in.cur == '+')
            {
                ++in.cur;
                break;
            }
        }
        in.skip_spaces();

        /* Decode crex optional section 3 (optional section) */
        in.mark_section_start(3);
        in.check_available_data(4, "CREX optional section 3 or end of CREX message");
        if (strncmp(in.cur, "SUPP", 4) == 0)
        {
            for (in.cur += 4; strncmp(in.cur, "++", 2) != 0; ++in.cur)
                in.check_available_data(2, "end of CREX optional section 3");
            in.skip_spaces();
        }

        /* Decode crex end section 4 */
        in.mark_section_start(4);
        in.check_available_data(4, "end of CREX message");
        if (strncmp(in.cur, "7777", 4) != 0)
            in.parse_error("unexpected data after data section or optional section 3");
    }

    void parse_data_section(const Opcodes& ops);
    unsigned parse_b_data(const Opcodes& ops);
    unsigned parse_r_data(const Opcodes& ops);
    void parse_value(int len, int is_signed, const char** d_start, const char** d_end);
};

void Decoder::parse_value(int len, int is_signed, const char** d_start, const char** d_end)
{
    TRACE("crex_decoder_parse_value(%d, %s): ", len, is_signed ? "signed" : "unsigned");

    /* Check for 2 more because we may have extra sign and check digit */
    in.check_available_data(len + 2, "end of data descriptor section");

    if (out.has_check_digit)
    {
        if ((*in.cur - '0') != expected_check_digit)
            in.parse_error("check digit mismatch: expected %d, found %d, rest of message: %.*s",
                    expected_check_digit,
                    (*in.cur - '0'),
                    (int)in.remaining(),
                    in.cur);

        expected_check_digit = (expected_check_digit + 1) % 10;
        ++in.cur;
    }

    /* Set the value to start after the check digit (if present) */
    *d_start = in.cur;

    /* Cope with one extra character in case the sign is present */
    if (is_signed && *in.cur == '-')
        ++len;

    /* Go to the end of the message */
    in.cur += len;

    /* Set the end value, removing trailing spaces */
    for (*d_end = in.cur; *d_end > *d_start && isspace(*(*d_end - 1)); (*d_end)--)
        ;

    /* Skip trailing spaces */
    in.skip_spaces();

    TRACE("%.*s\n", *d_end - *d_start, *d_start);
}

#if 0
/**
 * Compute a value from a CREX message
 *
 * @param value
 *   The value as found in the CREX message
 *
 * @param info
 *   The B table informations for the value
 *
 * @param cmodifier
 *   The C table modifier in effect for this value, or NULL if no C table
 *   modifier is in effect
 *
 * @returns
 *   The decoded value
 */
/* TODO: implement c modifier computation */
static double crex_decoder_compute_value(bufrex_decoder decoder, const char* value, dba_varinfo* info)
{
	double val;

	/* TODO use the C table values */
	
	if (value[0] == '/')
		return NAN;
 
	val = strtol(value, NULL, 10);

	if (info->scale != 0)
	{
		int scale = info->scale;

		if (info->scale > 0)
			while (scale--)
				val /= 10;
		else
			while (scale++)
				val *= 10;
	}

	3A
	return val;
}
#endif

unsigned Decoder::parse_b_data(const Opcodes& ops)
{
	IFTRACE{
		TRACE("crex_decoder_parse_b_data: items: ");
		ops.print(stderr);
		TRACE("\n");
	}

	// Get variable information
	Varinfo info = out.btable->query(ops.head());

	// Create the new Var
	Var var(info);

	// Parse value from the data section
	const char* d_start;
	const char* d_end;
	parse_value(info->len, !info->is_string(), &d_start, &d_end);

	/* If the variable is not missing, set its value */
	if (*d_start != '/')
	{
		if (info->is_string())
		{
			const int len = d_end - d_start;
			string buf(d_start, len);
			var.setc(buf.c_str());
		} else {
			int val = strtol((const char*)d_start, 0, 10);

			/* FIXME: here goes handling of active C table modifiers */

			var.seti(val);
		}
	}

	/* Store the variable that we found */
	current_subset->store_variable(var);
	IFTRACE{
		TRACE("  stored variable: "); var.print(stderr); TRACE("\n");
	}

#ifdef TRACE_DECODER
	{
		int left = (start + in.size()) - cur;
		TRACE("crex_decoder_parse_b_data -> %.*s (items:", left > 30 ? 30 : left, cur);
		ops.sub(1).print(stderr);
		TRACE(")\n");
	}
#endif

	return 1;
}

#if 0
static dba_err crex_read_c_data(bufrex_decoder decoder, bufrex_opcode* ops)
{
	bufrex_opcode op;
	dba_err err;
	/* Node affected by the operator */
	bufrex_opcode affected_op;

	/* Pop the C modifier node */
	DBA_RUN_OR_RETURN(bufrex_opcode_pop(ops, &op));

	TRACE("read_c_data\n");

	/* Pop the first node, since we handle it here */
	if ((err = bufrex_opcode_pop(ops, &affected_op)) != DBA_OK)
		goto fail1;

	/* Activate this C modifier */
	switch (WR_VAR_X(op->val))
	{
		case 1:
			decoder->c_width = WR_VAR_Y(op->val);
			break;
		case 2:
			decoder->c_scale = WR_VAR_Y(op->val);
			break;
		case 5:
		case 7:
		case 60:
			err = dba_error_parse(decoder->fname, decoder->line_no,
					"C modifier %d is not supported", WR_VAR_X(op->val));
			goto fail;
		default:
			err = dba_error_parse(decoder->fname, decoder->line_no,
					"Unknown C modifier %d", WR_VAR_X(op->val));
			goto fail;
	}

	/* Decode the affected data */
	if ((err = crex_read_data(decoder, &affected_op)) != DBA_OK)
		goto fail;

	/* Deactivate the C modifier */
	decoder->c_width = 0;
	decoder->c_scale = 0;

	/* FIXME: affected_op should always be NULL */
	assert(affected_op == NULL);
	bufrex_opcode_delete(&affected_op);
	return dba_error_ok();

fail:
	bufrex_opcode_delete(&affected_op);
fail1:
	bufrex_opcode_delete(&op);
	return err;
}
#endif

unsigned Decoder::parse_r_data(const Opcodes& ops)
{
	unsigned first = 1;
	int group = WR_VAR_X(ops.head());
	int count = WR_VAR_Y(ops.head());
	
	TRACE("R DATA %01d%02d%03d %d %d", 
			WR_VAR_F(ops.head()), WR_VAR_X(ops.head()), WR_VAR_Y(ops.head()), group, count);

	if (count == 0)
	{
		/* Delayed replication */
		
		/* Fetch the repetition count */
		const char* d_start;
		const char* d_end;
		parse_value(4, 0, &d_start, &d_end);
		count = strtol((const char*)d_start, NULL, 10);

		/* Insert the repetition count among the parsed variables */
		current_subset->store_variable_i(WR_VAR(0, 31, 1), count);

		TRACE("read_c_data %d items %d times (delayed)\n", group, count);
	} else
		TRACE("read_c_data %d items %d times\n", group, count);

	// Extract the first `group' nodes, to handle here
	Opcodes group_ops = ops.sub(first, group);

	// parse_data_section on it `count' times
	for (int i = 0; i < count; ++i)
		parse_data_section(group_ops);

	// Number of items processed
	return first + group;
}

void Decoder::parse_data_section(const Opcodes& ops)
{
	/*
	fprintf(stderr, "read_data: ");
	bufrex_opcode_print(ops, stderr);
	fprintf(stderr, "\n");
	*/
	TRACE("crex_decoder_parse_data_section: START\n");

	for (unsigned i = 0; i < ops.size(); )
	{
		IFTRACE{
			TRACE("crex_decoder_parse_data_section TODO: ");
			ops.sub(i).print(stderr);
			TRACE("\n");
		}

		switch (WR_VAR_F(ops[i]))
		{
			case 0: i += parse_b_data(ops.sub(i)); break;
			case 1: i += parse_r_data(ops.sub(i)); break;
			case 2: in.parse_error("C modifiers are not yet supported for CREX");
			case 3:
			{
				Opcodes exp = out.dtable->query(ops[i]);
				parse_data_section(exp);
				++i;
				break;
			}
			default:
				in.parse_error("cannot handle field %01d%02d%03d",
							WR_VAR_F(ops[i]),
							WR_VAR_X(ops[i]),
							WR_VAR_Y(ops[i]));
		}
	}
}

}

void CrexBulletin::decode_header(const std::string& buf, const char* fname, size_t offset)
{
	clear();
	Decoder d(buf, fname, offset, *this);
	d.decode_header();
}

void CrexBulletin::decode(const std::string& buf, const char* fname, size_t offset)
{
	clear();
	Decoder d(buf, fname, offset, *this);
	d.decode_header();
	d.decode_data();
}

}

/* vim:set ts=4 sw=4: */
