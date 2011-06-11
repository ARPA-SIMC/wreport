/*
 * wreport/bulletin - BUFR encoder
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

#include "bulletin.h"
#include "bulletin/buffers.h"
#include "opcode.h"
#include "conv.h"

#include <stdio.h>
#include <netinet/in.h>

#include <stdlib.h>	/* malloc */
#include <ctype.h>	/* isspace */
#include <string.h>	/* memcpy */
#include <stdarg.h>	/* va_start, va_end */
#include <math.h>	/* NAN */
#include <time.h>
#include <errno.h>

#include <assert.h>

//#define DEFAULT_TABLE_ID "B000000000980601"
/*
For encoding our generics:
#define DEFAULT_ORIGIN 255
#define DEFAULT_MASTER_TABLE 12
#define DEFAULT_LOCAL_TABLE 0
#define DEFAULT_TABLE_ID "B000000002551200"
*/

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

#if 0
/* Dump 'count' bits of 'buf', starting at the 'ofs-th' bit */
static dba_err dump_bits(void* buf, int ofs, int count, FILE* out)
{
	bitvec vec;
	int i, j;
	DBA_RUN_OR_RETURN(bitvec_create(&vec, "mem", 0, buf, (count + ofs) / 8 + 2));
	for (i = 0, j = 0; i < ofs; i++, j++)
	{
		uint32_t val;
		DBA_RUN_OR_RETURN(bitvec_get_bits(vec, 1, &val));
		if (j != 0 && (j % 8) == 0)
			putc(' ', out);
		putc(val ? ',' : '.', out);
	}
	for (i = 0; i < count; i++, j++)
	{
		uint32_t val;
		DBA_RUN_OR_RETURN(bitvec_get_bits(vec, 1, &val));
		if (j != 0 && (j % 8) == 0)
			putc(' ', out);
		putc(val ? '1' : '0', out);
	}
	bitvec_delete(vec);
	return dba_error_ok();
}
#endif

struct DDSEncoder : public bulletin::ConstBaseDDSExecutor
{
    bulletin::BufrOutput& ob;

    DDSEncoder(const Bulletin& b, bulletin::BufrOutput& ob) : ConstBaseDDSExecutor(b), ob(ob) {}
    virtual ~DDSEncoder() {}

    virtual void encode_padding(unsigned bit_count, bool value)
    {
        ob.add_bits(value ? 0xffffffff : 0, bit_count);
    }

    virtual void encode_associated_field(unsigned bit_count, uint32_t value)
    {
        ob.add_bits(value, bit_count);
    }

    virtual void encode_attr(Varinfo info, unsigned var_pos, Varcode attr_code)
    {
        const Var& var = get_var(var_pos);
        if (const Var* a = var.enqa(attr_code))
            ob.append_var(info, *a);
        else
            ob.append_missing(info);
    }
    virtual void encode_var(Varinfo info, unsigned var_pos)
    {
        const Var& var = get_var(var_pos);
        ob.append_var(info, var);
    }
    virtual unsigned encode_repetition_count(Varinfo info, unsigned var_pos)
    {
        const Var& var = get_var(var_pos);
        ob.append_var(info, var);
        return var.enqi();
    }
    virtual unsigned encode_associated_field_significance(Varinfo info, unsigned var_pos)
    {
        const Var& var = get_var(var_pos);
        ob.append_var(info, var);
        return var.enqi();
    }
    virtual unsigned encode_bitmap_repetition_count(Varinfo info, const Var& bitmap)
    {
        ob.add_bits(bitmap.info()->len, info->bit_len);
        return bitmap.info()->len;
    }
    virtual void encode_bitmap(const Var& bitmap)
    {
        for (unsigned i = 0; i < bitmap.info()->len; ++i)
            ob.add_bits(bitmap.value()[i] == '+' ? 0 : 1, 1);
    }
    virtual void encode_char_data(Varcode code, unsigned var_pos)
    {
        const Var& var = get_var(var_pos);
        const char* val = var.value();
        if (val == NULL)
            val = "";
        ob.append_string(val, WR_VAR_Y(code) * 8);
    }
};


struct Encoder
{
    /* Input message data */
    const BufrBulletin& in;
    /// Output buffer
    bulletin::BufrOutput out;

    /* We have to memorise offsets rather than pointers, because e->out->buf
     * can get reallocated during the encoding */

    /// Offset of the start of BUFR sections
    int sec[6];

    Encoder(const BufrBulletin& in, std::string& out)
        : in(in), out(out)
    {
        for (int i = 0; i < 6; ++i)
            sec[i] = 0;
    }

    void encode_sec1ed3();
    void encode_sec1ed4();
    void encode_sec2();
    void encode_sec3();
    void encode_sec4();

    // Run the encoding, copying data from in to out
    void run();
};

void Encoder::encode_sec1ed3()
{
    /* Encode bufr section 1 (Identification section) */
    /* Length of section */
    out.add_bits(18, 24);
    /* Master table number */
    out.append_byte(in.master_table_number);
    /* Originating/generating sub-centre (defined by Originating/generating centre) */
    out.append_byte(in.subcentre);
    /* Originating/generating centre (Common Code tableC-1) */
    /*DBA_RUN_OR_RETURN(bufr_message_append_byte(e, 0xff));*/
    out.append_byte(in.centre);
    /* Update sequence number (zero for original BUFR messages; incremented for updates) */
    out.append_byte(in.update_sequence_number);
    /* Bit 1= 0 No optional section = 1 Optional section included Bits 2 ­ 8 set to zero (reserved) */
    out.append_byte(in.optional_section_length ? 0x80 : 0);

    /* Data category (BUFR Table A) */
    /* Data sub-category (defined by local ADP centres) */
    out.append_byte(in.type);
    out.append_byte(in.localsubtype);
    /* Version number of master tables used (currently 9 for WMO FM 94 BUFR tables) */
    out.append_byte(in.master_table);
    /* Version number of local tables used to augment the master table in use */
    out.append_byte(in.local_table);

    /* Year of century */
    out.append_byte(in.rep_year == 2000 ? 100 : (in.rep_year % 100));
    /* Month */
    out.append_byte(in.rep_month);
    /* Day */
    out.append_byte(in.rep_day);
    /* Hour */
    out.append_byte(in.rep_hour);
    /* Minute */
    out.append_byte(in.rep_minute);
    /* Century */
    out.append_byte(in.rep_year / 100);
}

void Encoder::encode_sec1ed4()
{
    /* Encode bufr section 1 (Identification section) */
    /* Length of section */
    out.add_bits(22, 24);
    /* Master table number */
    out.append_byte(0);
    /* Originating/generating centre (Common Code tableC-1) */
    out.append_short(in.centre);
    /* Originating/generating sub-centre (defined by Originating/generating centre) */
    out.append_short(in.subcentre);
    /* Update sequence number (zero for original BUFR messages; incremented for updates) */
    out.append_byte(in.update_sequence_number);
    /* Bit 1= 0 No optional section = 1 Optional section included Bits 2 ­ 8 set to zero (reserved) */
    out.append_byte(in.optional_section_length ? 0x80 : 0);

    /* Data category (BUFR Table A) */
    out.append_byte(in.type);
    /* International data sub-category */
    out.append_byte(in.subtype);
    /* Local subcategory (defined by local ADP centres) */
    out.append_byte(in.localsubtype);
    /* Version number of master tables used (currently 9 for WMO FM 94 BUFR tables) */
    out.append_byte(in.master_table);
    /* Version number of local tables used to augment the master table in use */
    out.append_byte(in.local_table);

    /* Year of century */
    out.append_short(in.rep_year);
    /* Month */
    out.append_byte(in.rep_month);
    /* Day */
    out.append_byte(in.rep_day);
    /* Hour */
    out.append_byte(in.rep_hour);
    /* Minute */
    out.append_byte(in.rep_minute);
    /* Second */
    out.append_byte(in.rep_second);
}

void Encoder::encode_sec2()
{
    /* Encode BUFR section 2 (Optional section) */
    /* Nothing to do */
    if (in.optional_section_length)
    {
        int pad;
        /* Length of section */
        if ((pad = (in.optional_section_length % 2 == 1)))
            out.add_bits(4 + in.optional_section_length + 1, 24);
        else
            out.add_bits(4 + in.optional_section_length, 24);
        /* Set to 0 (reserved) */
        out.append_byte(0);

        out.raw_append(in.optional_section, in.optional_section_length);
        // Padd to even number of bytes
        if (pad) out.append_byte(0);
    }
}

void Encoder::encode_sec3()
{
	/* Encode BUFR section 3 (Data description section) */

	if (in.subsets.empty())
		throw error_consistency("message to encode has no data subsets");

	if (in.datadesc.empty())
		throw error_consistency("message to encode has no data descriptors");
#if 0
	if (in.datadesc.empty())
	{
		TRACE("Regenerating datadesc\n");
		/* If the data descriptor list is not already present, try to generate it
		 * from the varcodes of the variables in the first subgroup to encode */
		DBA_RUN_OR_GOTO(fail, bufrex_msg_generate_datadesc(e->in));

		/* Reread the descriptors */
		DBA_RUN_OR_GOTO(fail, bufrex_msg_get_datadesc(e->in, &ops));
	} else {
		TRACE("Reusing datadesc\n");
	}
#endif
    /* Length of section */
    out.add_bits(8 + 2*in.datadesc.size(), 24);
    /* Set to 0 (reserved) */
    out.append_byte(0);
    /* Number of data subsets */
    out.append_short(in.subsets.size());
    /* Bit 0 = observed data; bit 1 = use compression */
    out.append_byte(128);

    /* Data descriptors */
    for (unsigned i = 0; i < in.datadesc.size(); ++i)
        out.append_short(in.datadesc[i]);

    /* One padding byte to make the section even */
    out.append_byte(0);
}

void Encoder::encode_sec4()
{
    /* Encode BUFR section 4 (Data section) */

    /* Length of section (currently set to 0, will be filled in later) */
    out.add_bits(0, 24);
    out.append_byte(0);

    DDSEncoder e(in, out);
    in.run_dds(e);

    /* Write all the bits and pad the data section to reach an even length */
    out.flush();
    if ((out.out.size() % 2) == 1)
        out.append_byte(0);
    out.flush();

    /* Write the length of the section in its header */
    {
        uint32_t val = htonl(out.out.size() - sec[4]);
        memcpy((char*)out.out.data() + sec[4], ((char*)&val) + 1, 3);

        TRACE("sec4 size %zd\n", out.out.size() - sec[4]);
    }
}

void Encoder::run()
{
    /* Encode bufr section 0 (Indicator section) */
    out.raw_append("BUFR\0\0\0", 7);
    out.append_byte(in.edition);

    TRACE("sec0 ends at %zd\n", out.out.size());
    sec[1] = out.out.size();

	switch (in.edition)
	{
		case 2:
		case 3: encode_sec1ed3(); break;
		case 4: encode_sec1ed4(); break;
		default:
			error_unimplemented::throwf("Encoding BUFR edition %d is not implemented", in.edition);
	}


    TRACE("sec1 ends at %zd\n", out.out.size());
    sec[2] = out.out.size();
    encode_sec2();

    TRACE("sec2 ends at %zd\n", out.out.size());
    sec[3] = out.out.size();
    encode_sec3();

    TRACE("sec3 ends at %zd\n", out.out.size());
    sec[4] = out.out.size();
    encode_sec4();

    TRACE("sec4 ends at %zd\n", out.out.size());
    sec[5] = out.out.size();

    /* Encode section 5 (End section) */
    out.raw_append("7777", 4);
    TRACE("sec5 ends at %zd\n", out.out.size());

    /* Write the length of the BUFR message in its header */
    {
        uint32_t val = htonl(out.out.size());
        memcpy((char*)out.out.data() + 4, ((char*)&val) + 1, 3);

        TRACE("msg size %zd\n", out.out.size());
    }
}

} // Unnamed namespace

void BufrBulletin::encode(std::string& out) const
{
    Encoder e(*this, out);
    e.run();
}


} // wreport namespace

/* vim:set ts=4 sw=4: */
