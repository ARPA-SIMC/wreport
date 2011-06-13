/*
 * wreport/bulletin - BUFR decoder
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
#include "conv.h"
#include "notes.h"
#include "bulletin/buffers.h"

#include <stdio.h>
#include <netinet/in.h>

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <time.h>
#include <errno.h>

#include <assert.h>

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

// Return a value with bitlen bits set to 1
static inline uint32_t all_ones(int bitlen)
{
	return ((1 << (bitlen - 1))-1) | (1 << (bitlen - 1));
}

struct Decoder
{
    /// Input data
    bulletin::BufrInput& in;
    /* Output decoded variables */
    BufrBulletin& out;
    /// Number of expected subsets (read in decode_header, used in decode_data)
    size_t expected_subsets;
    /// True if undefined attributes are added to the output, else false
    bool conf_add_undef_attrs;

    Decoder(const std::string& buf, const char* fname, size_t offset, BufrBulletin& out)
        : in(out.reset_raw_details(buf)), out(out),
          conf_add_undef_attrs(false)
    {
        if (out.codec_options)
        {
            conf_add_undef_attrs = out.codec_options->decode_adds_undef_attrs;
        }
        in.fname = fname;
        in.start_offset = offset;
    }

    void decode_sec1ed3()
    {
        // master table number in sec1[3]
        out.master_table_number = in.read_byte(1, 3);
        // has_optional in sec1[7]
        // Once we know if the optional section is available, we can scan
        // section lengths for the rest of the message
        in.scan_other_sections(in.read_byte(1, 7) & 0x80);
        out.optional_section_length = in.sec[3] - in.sec[2];
        if (out.optional_section_length)
            out.optional_section_length -= 4;
        // subcentre in sec1[4]
        out.subcentre = in.read_byte(1, 4);
        // centre in sec1[5]
        out.centre = in.read_byte(1, 5);
        // Update sequence number sec1[6]
        out.update_sequence_number = in.read_byte(1, 6);
        out.master_table = in.read_byte(1, 10);
        out.local_table = in.read_byte(1, 11);
        out.type = in.read_byte(1, 8);
        out.subtype = 255;
        out.localsubtype = in.read_byte(1, 9);

        out.rep_year = in.read_byte(1, 12);
        // Fix the century with a bit of euristics
        if (out.rep_year > 50)
            out.rep_year += 1900;
        else
            out.rep_year += 2000;
        out.rep_month = in.read_byte(1, 13);
        out.rep_day = in.read_byte(1, 14);
        out.rep_hour = in.read_byte(1, 15);
        out.rep_minute = in.read_byte(1, 16);
        if (in.read_byte(1, 17) != 0)
            out.rep_year = in.read_byte(1, 17) * 100 + (out.rep_year % 100);
    }

    void decode_sec1ed4()
    {
        // master table number in sec1[3]
        out.master_table_number = in.read_byte(1, 3);
        // centre in sec1[4-5]
        out.centre = in.read_number(1, 4, 2);
        // subcentre in sec1[6-7]
        out.subcentre = in.read_number(1, 6, 2);
        // update sequence number sec1[8]
        out.update_sequence_number = in.read_byte(1, 8);
        // has_optional in sec1[9]
        // Once we know if the optional section is available, we can scan
        // section lengths for the rest of the message
        in.scan_other_sections(in.read_byte(1, 9) & 0x80);
        out.optional_section_length = in.sec[3] - in.sec[2];
        if (out.optional_section_length)
            out.optional_section_length -= 4;
        // category in sec1[10]
        out.type = in.read_byte(1, 10);
        // international data sub-category in sec1[11]
        out.subtype = in.read_byte(1, 11);
        // local data sub-category in sec1[12]
        out.localsubtype = in.read_byte(1, 12);
        // version number of master table in sec1[13]
        out.master_table = in.read_byte(1, 13);
        // version number of local table in sec1[14]
        out.local_table = in.read_byte(1, 14);
        // year in sec1[15-16]
        out.rep_year = in.read_number(1, 15, 2);
        // month in sec1[17]
        out.rep_month = in.read_byte(1, 17);
        // day in sec1[18]
        out.rep_day = in.read_byte(1, 18);
        // hour in sec1[19]
        out.rep_hour = in.read_byte(1, 19);
        // minute in sec1[20]
        out.rep_minute = in.read_byte(1, 20);
        // sec in sec1[21]
        out.rep_second = in.read_byte(1, 21);
    }

    /* Decode the message header only */
    void decode_header()
    {
        // Read BUFR section 0 (Indicator section)
        if (memcmp(in.data + in.sec[0], "BUFR", 4) != 0)
            in.parse_error(0, 0, "data does not start with BUFR header (\"%.4s\" was read instead)", in.data + in.sec[0]);

        // Check the BUFR edition number
        out.edition = in.read_byte(0, 7);
        if (out.edition != 2 && out.edition != 3 && out.edition != 4)
            in.parse_error(0, 7, "Only BUFR edition 3 and 4 are supported (this message is edition %d)", out.edition);

        // Looks like a BUFR, scan section starts
        in.scan_lead_sections();

        // Read bufr section 1 (Identification section)
        in.check_available_data(1, 0, out.edition == 4 ? 22 : 18, "section 1 of BUFR message (identification section)");

		switch (out.edition)
		{
			case 2: decode_sec1ed3(); break;
			case 3: decode_sec1ed3(); break;
			case 4: decode_sec1ed4(); break;
			default:
				error_consistency::throwf("BUFR edition is %d, but I can only decode 2, 3 and 4", out.edition);
		}

		TRACE("info:ed %d opt %d upd %d origin %d.%d tables %d.%d type %d.%d %04d-%02d-%02d %02d:%02d\n", 
				out.edition, out.optional_section_length, out.update_sequence_number,
				out.centre, out.subcentre,
				out.master_table, out.local_table,
				out.type, out.subtype,
				out.rep_year, out.rep_month, out.rep_day, out.rep_hour, out.rep_minute);

        /* Read BUFR section 2 (Optional section) */
        if (out.optional_section_length > 0)
        {
            out.optional_section_length = in.read_number(2, 0, 3) - 4;
            out.optional_section = new char[out.optional_section_length];
            if (out.optional_section == NULL)
                throw error_alloc("allocating space for the optional section");
            memcpy(out.optional_section, in.data + in.sec[2] + 4, out.optional_section_length);
        }

        /* Read BUFR section 3 (Data description section) */
        in.check_available_data(3, 0, 8, "section 3 of BUFR message (data description section)");
        expected_subsets = in.read_number(3, 4, 2);
        out.compression = (in.read_byte(3, 6) & 0x40) ? 1 : 0;
        for (unsigned i = 0; i < (in.sec[4] - in.sec[3] - 7)/2; i++)
            out.datadesc.push_back((Varcode)in.read_number(3, 7 + i * 2, 2));
        TRACE("info:s3length %d subsets %zd observed %d compression %d byte7 %x\n",
                in.sec[4] - in.sec[3], expected_subsets, (in.read_byte(3, 6) & 0x80) ? 1 : 0,
                out.compression, in.read_byte(3, 6));
        /*
       IFTRACE{
       TRACE(" -> data descriptor section: ");
       bufrex_opcode_print(msg->datadesc, stderr);
       TRACE("\n");
       }
       */
    }

    /* Decode message data section after the header has been decoded */
    void decode_data();
};

struct DataSection;

struct Bitmap
{
    const BufrBulletin& out;
    /* Data present bitmap */
    char* bitmap;
    /* Length of data present bitmap */
    size_t len;
    /* Number of elements set to true in the bitmap */
    int count;
    /* Next bitmap element for which we decode values */
    int use_index;
    /* Next subset element for which we decode attributes */
    int subset_index;

    Bitmap(const BufrBulletin& out)
        : out(out), bitmap(0), count(0)
    {
    }
    ~Bitmap()
    {
        if (bitmap) delete[] bitmap;
    }

    void next(DataSection& ds);
};

/**
 * Variable consumer
 *
 * This provides a generic interface to which variables are sent after
 * decoding.
 */
struct VarAdder
{
    virtual ~VarAdder() {}

    /**
     * Produce a variable for the given subset.
     *
     * The variable can be modified during processing, to enhance it before it
     * reaches the final adder
     *
     * A subset number of -1 means 'the current subset' and is used when
     * decoding uncompressed BUFR messages
     */
    virtual void add_var(Var&, int subset=-1) = 0;
};

struct VarIgnorer : public VarAdder
{
    virtual void add_var(Var&, int subset=-1) {}
};

template<typename CLS>
struct VarAdderProxy : public VarAdder
{
    typedef void (CLS::*adder_meth)(Var&, int);
    CLS& obj;
    adder_meth adder;

    VarAdderProxy(CLS& obj, adder_meth adder) : obj(obj), adder(adder) {}

    virtual void add_var(Var& var, int subset=-1)
    {
        (obj.*adder)(var, subset);
    }
};

struct AnnotationVarAdder : public VarAdder
{
    VarAdder& next;
    const Var& attr;

    AnnotationVarAdder(VarAdder& next, const Var& attr)
        : next(next), attr(attr) {}

    virtual void add_var(Var& var, int subset=-1)
    {
        var.seta(attr);
        next.add_var(var, subset);
    }
};

struct DataSection
{
    bulletin::BufrInput& in;

    DataSection(bulletin::BufrInput& in) : in(in) {}
    virtual ~DataSection() {}

    void decode_b_value(Varinfo info, VarAdder& adder)
    {
        if (info->is_string())
            decode_b_string(info, adder);
        else
            decode_b_num(info, adder);
    }

    virtual void decode_b_num(Varinfo info, VarAdder& out)
    {
        /* Read a value */
        Var var(info);
        in.decode_number(var);
        out.add_var(var);
    }

    virtual void decode_b_string(Varinfo info, VarAdder& adder)
    {
        /* Read a string */
        Var var(info);
        in.decode_string(var);
        adder.add_var(var);
    }

    /**
     * Decode a delayed replication factor, send the resulting variables to
     * \a adder and return the replication count
     */
    virtual int decode_delayed_replication_factor(Varinfo info, VarAdder& adder)
    {
        // Fetch the repetition count
        uint32_t count = in.get_bits(info->bit_len);

        /* Insert the repetition count among the parsed variables */
        Var var(info, (int)count);
        adder.add_var(var);

        return count;
    }
};

struct CompressedDataSection : public DataSection
{
    unsigned subset_count;

    CompressedDataSection(bulletin::BufrInput& input, unsigned subset_count)
        : DataSection(input), subset_count(subset_count) {}

    virtual void decode_b_num(Varinfo info, VarAdder& out)
    {
        /* Read a value */
        Var var(info);

        uint32_t base = in.get_bits(info->bit_len);

        TRACE("datasec:decode_b_num:reading %s (%s), size %d, scale %d, starting point %d\n", info->desc, info->bufr_unit, info->bit_len, info->scale, base);

        /* Check if there are bits which are not 1 (that is, if the value is present) */
        bool missing = (base == all_ones(info->bit_len));

        /*bufr_decoder_debug(decoder, "  %s: %d%s\n", info.desc, base, info.type);*/
        TRACE("datasec:decode_b_num:len %d base %d info-len %d info-desc %s\n", info->bit_len, base, info->bit_len, info->desc);

        /* Store the variable that we found */

        /* If compression is in use, then we just decoded the base value.  Now
         * we need to decode all the offsets */

        /* Decode the number of bits (encoded in 6 bits) that these difference
         * values occupy */
        uint32_t diffbits = in.get_bits(6);
        if (missing && diffbits != 0)
            error_consistency::throwf("When decoding compressed BUFR data, the difference bit length must be 0 (and not %d like in this case) when the base value is missing", diffbits);

        TRACE("Compressed number, base value %d diff bits %d\n", base, diffbits);

        for (unsigned i = 0; i < subset_count; ++i)
        {
            in.decode_number(var, base, diffbits);

            /* Add it to this subset */
            out.add_var(var, i);
        }
    }

    void decode_b_string(Varinfo info, VarAdder& adder)
    {
        /* Read a string */
        Var var(info);
        //size_t len = 0;

        char str[info->bit_len / 8 + 2];
        size_t len;
        bool missing = !in.decode_string(info->bit_len, str, len);

        /* Store the variable that we found */

        /* If compression is in use, then we just decoded the base value.  Now
         * we need to decode all the offsets */

        /* Decode the number of bits (encoded in 6 bits) that these difference
         * values occupy */
        uint32_t diffbits = in.get_bits(6);

        TRACE("datadesc:decode_b_string:compressed string, base:%.*s, diff bits %d\n", (int)len, str, diffbits);

        if (diffbits != 0)
        {
            /* For compressed strings, the reference value must be all zeros */
            for (size_t i = 0; i < len; ++i)
                if (str[i] != 0)
                    error_unimplemented::throwf("compressed strings with %d bit deltas have non-zero reference value", diffbits);

            /* Let's also check that the number of
             * difference characters is the same length as
             * the reference string */
            if (diffbits > len)
                error_unimplemented::throwf("compressed strings with %zd characters have %d bit deltas (deltas should not be longer than field)", len, diffbits);

            for (unsigned i = 0; i < subset_count; ++i)
            {
                bool missing = in.decode_string(diffbits * 8, str, len);

                // Set the variable value
                if (missing)
                {
                    /* Missing value */
                    TRACE("datadesc:decode_b_string:decoded[%d] as missing\n", i);
                    var.unset();
                } else {
                    /* Compute the value for this subset */
                    TRACE("datadesc:decode_b_string:decoded[%d] as \"%s\"\n", i, str);
                    var.setc(str);
                }

                /* Add it to this subset */
                adder.add_var(var, i);
            }
        } else {
            /* Add the string to all the subsets */
            for (unsigned i = 0; i < subset_count; ++i)
            {
                // Set the variable value
                if (!missing) var.setc(str);

                adder.add_var(var, i);
            }
        }
    }

    /**
     * Decode a delayed replication factor, send the resulting variables to
     * \a adder and return the replication count
     */
    int decode_delayed_replication_factor(Varinfo info, VarAdder& adder)
    {
        // Fetch the repetition count
        int count = in.get_bits(info->bit_len);

        /* Insert the repetition count among the parsed variables */

        /* If compression is in use, then we just decoded the base value.  Now
         * we need to decode all the repetition factors and see
         * that they are the same */

        /* Decode the number of bits (encoded in 6 bits) that these difference
         * values occupy */
        uint32_t diffbits = in.get_bits(6);

        TRACE("datadesc:decode_delayed_replication_factor:compressed delayed repetition, base value %d diff bits %d\n", count, diffbits);

        uint32_t repval = 0;
        for (unsigned i = 0; i < subset_count; ++i)
        {
            /* Decode the difference value */
            uint32_t diff = in.get_bits(diffbits);

            /* Compute the value for this subset */
            uint32_t newval = count + diff;
            TRACE("datadesc:decode_delayed_replication_factor:decoded[%d] as %d+%d=%d\n", i, count, diff, newval);

            if (i == 0)
                repval = newval;
            else if (repval != newval)
                in.parse_error("compressed delayed replication factor has different values for subsets (%d and %d)", repval, newval);

            Var var(info, (int)newval);
            adder.add_var(var, i);
        }

        return count;
    }
};

void Bitmap::next(DataSection& ds)
{
    if (bitmap == 0)
        ds.in.parse_error("applying a data present bitmap with no current bitmap");
    TRACE("bitmap:next:pre %d %d %zd\n", use_index, subset_index, len);
    if (out.subsets.size() == 0)
        ds.in.parse_error("no subsets created yet, but already applying a data present bitmap");
    ++use_index;
    ++subset_index;
    while (use_index < 0 || (
                (unsigned)use_index < len &&
                bitmap[use_index] == '-'))
    {
        TRACE("bitmap:next:INCR\n");
        ++use_index;
        ++subset_index;
        while ((unsigned)subset_index < out.subsets[0].size() &&
                WR_VAR_F(out.subsets[0][subset_index].code()) != 0)
            ++subset_index;
    }
    if ((unsigned)use_index > len)
        ds.in.parse_error("moved past end of data present bitmap");
    if ((unsigned)subset_index == out.subsets[0].size())
        ds.in.parse_error("end of data reached when applying attributes");
    TRACE("bitmap:next:post %d %d\n", use_index, subset_index);
}

struct opcode_interpreter
{
    struct AttrMode
    {
        opcode_interpreter& i;
        AttrMode(opcode_interpreter& i) : i(i)
        {
            i.set_attr_mode();
        }
        ~AttrMode()
        {
            i.set_normal_mode();
        }
    };
    struct SubstMode
    {
        opcode_interpreter& i;
        SubstMode(opcode_interpreter& i) : i(i)
        {
            i.set_subst_mode();
        }
        ~SubstMode()
        {
            i.set_normal_mode();
        }
    };
    struct OverrideAdder
    {
        opcode_interpreter& i;
        VarAdder* old;
        OverrideAdder(opcode_interpreter& i, VarAdder& a)
            : i(i), old(i.current_adder)
        {
            i.current_adder = &a;
        }
        ~OverrideAdder()
        {
            i.current_adder = old;
        }
    };

    Decoder& d;
    DataSection& ds;
    Bitmap bitmap;
    /// Currently active variable destination
    VarAdder* current_adder;
    //VarAdder* attr_adder;

    /* Current value of scale change from C modifier */
    int c_scale_change;
    /* Current value of width change from C modifier */
    int c_width_change;
    /** Current value of string length override from C08 modifiers (0 for no
     * override)
     */
    int c_string_len_override;
    /**
     * Number of extra bits inserted by the current C04yyy modifier (0 for no
     * C04yyy operator in use)
     */
    int c04_bits;
    /// Meaning of C04yyy field according to code table B31021
    int c04_meaning;

    opcode_interpreter(Decoder& d, DataSection& ds)
        : d(d), ds(ds), bitmap(d.out), current_adder(0),
        c_scale_change(0), c_width_change(0), c_string_len_override(0),
        c04_bits(0), c04_meaning(63)
    {
    }

    ~opcode_interpreter()
    {
    }

    virtual void set_normal_mode() = 0;
    virtual void set_attr_mode() = 0;
    virtual void set_subst_mode() = 0;

    /**
     * Get the Varinfo needed to decode \a code, applying C operator changes if
     * any are active.
     */
    Varinfo get_info(Varcode code)
    {
        Varinfo peek = d.out.btable->query(code);

        if (!c_scale_change && !c_width_change && !c_string_len_override)
            return peek;

        int scale = peek->scale;
        if (c_scale_change)
        {
            TRACE("get_info:applying %d scale change\n", c_scale_change);
            scale += c_scale_change;
        }
        int bit_len = peek->bit_len;

        if (peek->is_string() && c_string_len_override)
        {
            TRACE("get_info:overriding string to %d bytes\n", c_string_len_override);
            bit_len = c_string_len_override * 8;
        }
        else if (c_width_change)
        {
            TRACE("get_info:applying %d width change\n", c_width_change);
            bit_len += c_width_change;
        }

        TRACE("get_info:requesting alteration scale:%d, bit_len:%d\n", scale, bit_len);
        return d.out.btable->query_altered(code, scale, bit_len);
    }

    unsigned decode_b_data(const Opcodes& ops)
    {
        IFTRACE {
            TRACE("decode_b_data:items: ");
            ops.print(stderr);
            TRACE("\n");
        }

        Varinfo info = get_info(ops.head());

        IFTRACE {
            TRACE("decode_b_data:parsing @%u+%d [bl %d sc %d ref %d]: %d%02d%03d %s[%s]\n",
                    ds.in.s4_cursor, 8 - ds.in.pbyte_len,
                    info->bit_len, info->scale, info->bit_ref,
                    WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var),
                    info->desc, info->unit);
            ds.in.debug_dump_next_bits(64);
            TRACE("\n");
        }

        if (WR_VAR_X(info->var) == 33 && bitmap.bitmap)
        {
            if (c04_bits)
                throw error_unimplemented("C04 modifiers in B33yyy attributes are not supported");
            bitmap.next(ds);

            /* Get the real datum */
            AttrMode am(*this);
            ds.decode_b_value(info, *current_adder);
        } else {
            /* Get the real datum */
            if (c04_bits)
            {
                TRACE("decode_b_data:reading %d bits of C04 information\n", c04_bits);
                uint32_t val = ds.in.get_bits(c04_bits);
                TRACE("decode_b_data:read C04 information %x\n", val);
                switch (c04_meaning)
                {
                    case 1:
                    {
                        // Add attribute B33002=val
                        Var attr(d.out.btable->query(WR_VAR(0, 33, 2)), (int)val);
                        AnnotationVarAdder ava(*current_adder, attr);
                        OverrideAdder oa(*this, ava);
                        ds.decode_b_value(info, *current_adder);
                        break;
                    }
                    case 2:
                    {
                        // Add attribute B33002=val
                        Var attr(d.out.btable->query(WR_VAR(0, 33, 3)), (int)val);
                        AnnotationVarAdder ava(*current_adder, attr);
                        OverrideAdder oa(*this, ava);
                        ds.decode_b_value(info, *current_adder);
                        break;
                    }
                    case 3 ... 5:
                        // Reserved: ignored
                        notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                                c04_meaning);
                        break;
                    case 6:
                        // Add attribute B33050=val
                        if (d.conf_add_undef_attrs || val != 15)
                        {
                            Var attr(d.out.btable->query(WR_VAR(0, 33, 50)));
                            if (val != 15) attr.seti(val);
                            AnnotationVarAdder ava(*current_adder, attr);
                            OverrideAdder oa(*this, ava);
                            ds.decode_b_value(info, *current_adder);
                        } else
                            ds.decode_b_value(info, *current_adder);
                        break;
                    case 9 ... 20:
                        // Reserved: ignored
                        notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                                c04_meaning);
                        break;
                    case 22 ... 62:
                        notes::logf("Ignoring B31021=%d, which is documented as 'reserved for local use'\n",
                                c04_meaning);
                        break;
                    case 63:
                        /*
                         * Ignore quality information if B31021 is missing.
                         * The Guide to FM94-BUFR says:
                         *   If the quality information has no meaning for some
                         *   of those following elements, but the field is
                         *   still there, there is at present no explicit way
                         *   to indicate "no meaning" within the currently
                         *   defined meanings. One must either redefine the
                         *   meaning of the associated field in its entirety
                         *   (by including 0 31 021 in the message with a data
                         *   value of 63 - "missing value") or remove the
                         *   associated field bits by the "cancel" operator: 2
                         *   04 000.
                         */
                        break;
                    default:
                        error_unimplemented::throwf("C04 modifiers with B31021=%d are not supported", c04_meaning);
                }
            } else
                ds.decode_b_value(info, *current_adder);
        }

        return 1;
    }

    unsigned decode_bitmap(const Opcodes& ops, Varcode code, VarAdder& adder);

    /**
     * Decode instant or delayed replication information.
     *
     * In case of delayed replication, store a variable in the subset(s)
     * with the repetition count.
     */
    unsigned decode_replication_info(const Opcodes& ops, int& group, int& count, VarAdder& adder)
    {
        unsigned used = 1;
        group = WR_VAR_X(ops.head());
        count = WR_VAR_Y(ops.head());

        if (count == 0)
        {
            // Delayed replication

            // We also use the delayed replication factor opcode
            ++used;

            Varcode rep_op = ops[1];

            // Fetch the repetition count
            Varinfo rep_info = d.out.btable->query(rep_op);
            count = ds.decode_delayed_replication_factor(rep_info, adder);

            TRACE("decode_replication_info:%d items %d times (delayed)\n", group, count);
        } else
            TRACE("decode_replication_info:%d items %d times\n", group, count);

        return used;
    }

    unsigned decode_r_data(const Opcodes& ops)
    {
        // Read replication information
        int group, count;
        unsigned first;
        first = decode_replication_info(ops, group, count, *current_adder);

        TRACE("decode_r_data:%01d%02d%03d %d %d\n", 
                WR_VAR_F(ops.head()), WR_VAR_X(ops.head()), WR_VAR_Y(ops.head()), group, count);

        // Extract the first `group' nodes, to handle here
        Opcodes group_ops = ops.sub(first, group);

        // decode_data_section on it `count' times
        for (int i = 0; i < count; ++i)
            decode_data_section(group_ops);

        // Number of items processed
        return first + group;
    }

	unsigned decode_c_data(const Opcodes& ops);

	/* Run the opcode interpreter to decode the data section */
	void decode_data_section(const Opcodes& ops)
	{
		/*
		fprintf(stderr, "read_data: ");
		bufrex_opcode_print(ops, stderr);
		fprintf(stderr, "\n");
		*/
		TRACE("decode_data_section:START\n");

		for (unsigned i = 0; i < ops.size(); )
		{
			IFTRACE{
				TRACE("decode_data_section:pos %u/%zd TODO: ", ds.in.s4_cursor, ds.in.data_len);
				ops.sub(i).print(stderr);
				TRACE("\n");
			}

			switch (WR_VAR_F(ops[i]))
			{
				case 0: i += decode_b_data(ops.sub(i)); break;
				case 1: i += decode_r_data(ops.sub(i)); break;
				case 2: i += decode_c_data(ops.sub(i)); break;
				case 3:
				{
					Opcodes exp = d.out.dtable->query(ops[i]);
					decode_data_section(exp);
					++i;
					break;
				}
				default:
					ds.in.parse_error("cannot handle field %01d%02d%03d",
								WR_VAR_F(ops[i]),
								WR_VAR_X(ops[i]),
								WR_VAR_Y(ops[i]));
			}
		}
	}

    virtual void run() = 0;
};

struct opcode_interpreter_plain : public opcode_interpreter
{
    Subset* current_subset;
    VarAdderProxy<opcode_interpreter_plain> adder;

    opcode_interpreter_plain(Decoder& d, DataSection& ds)
        : opcode_interpreter(d, ds),
          current_subset(0),
          adder(*this, &opcode_interpreter_plain::add_var)
    {
        current_adder = &adder;
    }

    virtual void set_normal_mode()
    {
        adder.adder = &opcode_interpreter_plain::add_var;
    }
    virtual void set_attr_mode()
    {
        adder.adder = &opcode_interpreter_plain::add_attr;
    }
    virtual void set_subst_mode()
    {
        adder.adder = &opcode_interpreter_plain::add_subst;
    }

    void add_var(Var& var, int subset=-1)
    {
        TRACE("bulletin:adding var %01d%02d%03d %s to current subset\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value());
        current_subset->store_variable(var);
    }

    void add_attr(Var& var, int subset=-1)
    {
        TRACE("bulletin:adding var %01d%02d%03d %s as attribute to %01d%02d%03d bsi %d/%zd\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F((*current_subset)[bitmap.subset_index].code()),
                WR_VAR_X((*current_subset)[bitmap.subset_index].code()),
                WR_VAR_Y((*current_subset)[bitmap.subset_index].code()),
                bitmap.subset_index, current_subset->size());
        (*current_subset)[bitmap.subset_index].seta(var);
    }

    void add_subst(Var& var, int subset=-1)
    {
        TRACE("bulletin:adding substitute value %01d%02d%03d %s as attribute to %01d%02d%03d bsi %d/%zd\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F((*current_subset)[bitmap.subset_index].code()),
                WR_VAR_X((*current_subset)[bitmap.subset_index].code()),
                WR_VAR_Y((*current_subset)[bitmap.subset_index].code()),
                bitmap.subset_index, current_subset->size());
        (*current_subset)[bitmap.subset_index].seta(var);
    }

    virtual void run()
    {
        /* Iterate on the number of subgroups */
        for (size_t i = 0; i < d.out.subsets.size(); ++i)
        {
            current_subset = &d.out.obtain_subset(i);
            decode_data_section(Opcodes(d.out.datadesc));
        }

        IFTRACE {
            if (ds.in.bits_left() > 32)
            {
                fprintf(stderr, "The data section of %s:%zd still contains %d unparsed bits\n",
                        ds.in.fname, ds.in.start_offset, ds.in.bits_left() - 32);
                /*
                   err = dba_error_parse(msg->file->name, POS + vec->cursor,
                   "the data section still contains %d unparsed bits",
                   bitvec_bits_left(vec));
                   goto fail;
                   */
            }
        }
    }
};

struct opcode_interpreter_compressed : public opcode_interpreter
{
    VarAdderProxy<opcode_interpreter_compressed> adder;

    opcode_interpreter_compressed(Decoder& d, CompressedDataSection& ds)
        : opcode_interpreter(d, ds),
          adder(*this, &opcode_interpreter_compressed::add_var)
    {
        current_adder = &adder;
    }

    virtual void set_normal_mode()
    {
        adder.adder = &opcode_interpreter_compressed::add_var;
    }
    virtual void set_attr_mode()
    {
        adder.adder = &opcode_interpreter_compressed::add_attr;
    }
    virtual void set_subst_mode()
    {
        adder.adder = &opcode_interpreter_compressed::add_subst;
    }

    void add_var(Var& var, int subset)
    {
        TRACE("bulletin:adding var %01d%02d%03d %s to subset %d\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(), subset);
        d.out.subsets[subset].store_variable(var);
    }

    void add_attr(Var& var, int subset)
    {
        TRACE("bulletin:adding var %01d%02d%03d %s as attribute to %01d%02d%03d bsi %d/%zd\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F(d.out.subsets[subset][bitmap.subset_index].code()),
                WR_VAR_X(d.out.subsets[subset][bitmap.subset_index].code()),
                WR_VAR_Y(d.out.subsets[subset][bitmap.subset_index].code()),
                bitmap.subset_index, d.out.subsets[subset].size());
        d.out.subsets[subset][bitmap.subset_index].seta(var);
    }

    void add_subst(Var& var, int subset=-1)
    {
        TRACE("bulletin:adding substitute var %01d%02d%03d %s as attribute to %01d%02d%03d bsi %d/%zd\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F(d.out.subsets[subset][bitmap.subset_index].code()),
                WR_VAR_X(d.out.subsets[subset][bitmap.subset_index].code()),
                WR_VAR_Y(d.out.subsets[subset][bitmap.subset_index].code()),
                bitmap.subset_index, d.out.subsets[subset].size());
        d.out.subsets[subset][bitmap.subset_index].seta(var);
    }

    virtual void run()
    {
        /* Only needs to parse once */
        decode_data_section(Opcodes(d.out.datadesc));

        IFTRACE {
            if (ds.in.bits_left() > 32)
            {
                fprintf(stderr, "The data section of %s:%zd still contains %d unparsed bits\n",
                        ds.in.fname, ds.in.start_offset, ds.in.bits_left() - 32);
                /*
                   err = dba_error_parse(msg->file->name, POS + vec->cursor,
                   "the data section still contains %d unparsed bits",
                   bitvec_bits_left(vec));
                   goto fail;
                   */
            }
        }
    }
};

void Decoder::decode_data()
{
    // Once we filled the Bulletin header info, load decoding tables and allocate subsets
    out.load_tables();
    out.obtain_subset(expected_subsets - 1);

    /* Read BUFR section 4 (Data section) */
    TRACE("decode_data:section 4 is %d bytes long (%02x %02x %02x %02x)\n",
            in.read_number(4, 0, 3),
            in.read_byte(4, 0),
            in.read_byte(4, 1),
            in.read_byte(4, 2),
            in.read_byte(4, 3));

    if (out.compression)
    {
        CompressedDataSection ds(in, out.subsets.size());
        opcode_interpreter_compressed interpreter(*this, ds);
        interpreter.run();
    } else {
        DataSection ds(in);
        opcode_interpreter_plain interpreter(*this, ds);
        interpreter.run();
    }

    /* Read BUFR section 5 (Data section) */
    in.check_available_data(5, 0, 4, "section 5 of BUFR message (end section)");

    if (memcmp(in.data + in.sec[5], "7777", 4) != 0)
        in.parse_error(5, 0, "section 5 does not contain '7777'");

    //if (subsets_no != out.subsets.size())
    //    parse_error(sec5, "header advertised %u subsets but only %zd found", subsets_no, out.subsets.size());
}

}


void BufrBulletin::decode_header(const std::string& buf, const char* fname, size_t offset)
{
	clear();
	Decoder d(buf, fname, offset, *this);
	d.decode_header();
}

void BufrBulletin::decode(const std::string& buf, const char* fname, size_t offset)
{
	clear();
	Decoder d(buf, fname, offset, *this);
	d.decode_header();
	d.decode_data();
}


unsigned opcode_interpreter::decode_bitmap(const Opcodes& ops, Varcode code, VarAdder& adder)
{
	if (bitmap.bitmap) delete[] bitmap.bitmap;
	bitmap.bitmap = 0;

    int group;
    int count;
    VarIgnorer ignorer;
    unsigned used = decode_replication_info(ops, group, count, ignorer);

    // Sanity checks

    if (group != 1)
        ds.in.parse_error("bitmap section replicates %d descriptors instead of one", group);

    if (used >= ops.size())
        ds.in.parse_error("there are no descriptor after bitmap replicator (expected B31031)");

    if (ops[used] != WR_VAR(0, 31, 31))
        ds.in.parse_error("bitmap element descriptor is %02d%02d%03d instead of B31031",
                WR_VAR_F(ops[used]), WR_VAR_X(ops[used]), WR_VAR_Y(ops[used]));

    // If compressed, ensure that the difference bits are 0 and they are
    // not trying to transmit odd things like delta bitmaps 
    if (d.out.compression)
    {
        /* Decode the number of bits (encoded in 6 bits) that these difference
         * values occupy */
        uint32_t diffbits = ds.in.get_bits(6);
        if (diffbits != 0)
            ds.in.parse_error("bitmap declares %d difference bits per bitmap value, but we only support 0", diffbits);
    }

	// Consume the data present indicator from the opcodes to process
	++used;

	// Bitmap size is now in count

	// Read the bitmap
	bitmap.count = 0;
	bitmap.bitmap = new char[count + 1];
	for (int i = 0; i < count; ++i)
	{
		uint32_t val = ds.in.get_bits(1);
		bitmap.bitmap[i] = (val == 0) ? '+' : '-';
		if (val == 0) ++bitmap.count;
	}
	bitmap.bitmap[count] = 0;
	bitmap.len = count;

	// Create a single use varinfo to store the bitmap
	MutableVarinfo info(MutableVarinfo::create_singleuse());
	info->set_string(code, "DATA PRESENT BITMAP", count);

	// Store the bitmap
	Var bmp(info, bitmap.bitmap);

    // Add var to subset(s)
    if (d.out.compression)
    {
        for (unsigned i = 0; i < d.out.subsets.size(); ++i)
            adder.add_var(bmp, i);
    } else {
        adder.add_var(bmp);
    }

	// Bitmap will stay set as a reference to the variable to use as the
	// current bitmap. The subset(s) are taking care of memory managing it.

	IFTRACE {
		TRACE("Decoded bitmap count %d: ", bitmap.count);
		for (unsigned i = 0; i < bitmap.len; ++i)
			TRACE("%c", bitmap.bitmap[i]);
		TRACE("\n");
	}

    // Move to first bitmap use index
    bitmap.use_index = -1;
    bitmap.subset_index = -1;

    return used;
}

unsigned opcode_interpreter::decode_c_data(const Opcodes& ops)
{
	unsigned used = 1;
	
	Varcode code = ops.head();

	TRACE("decode_c_data:%01d%02d%03d\n", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));

	switch (WR_VAR_X(code))
	{
		case 1:
			c_width_change = WR_VAR_Y(code) == 0 ? 0 : WR_VAR_Y(code) - 128;
			break;
		case 2:
			c_scale_change = WR_VAR_Y(code) == 0 ? 0 : WR_VAR_Y(code) - 128;
			break;
        case 4:
            if (d.out.compression)
                throw error_unimplemented("C04 modifiers in compressed BUFRs are not yet implemented");
                /*
                // If we are decoding a compressed BUFR, ensure that the variable
                // is the same in all subsets
                if (var != *copy)
                {
                    string name = varcode_format(copy->code());
                    string val1 = copy->format();
                    string val2 = var.format();
                    error_consistency::throwf("%s has different values across compressed subsets (first is %s, second is %s)", name.c_str(), val1.c_str(), val2.c_str());
                }
                */
            if (WR_VAR_Y(code) && c04_bits)
                throw error_unimplemented("nested C04 modifiers are not yet implemented");
            if (WR_VAR_Y(code) > 32)
                error_unimplemented::throwf("C04 modifier wants %d bits but only at most 32 are supported", WR_VAR_Y(code));
            if (WR_VAR_Y(code))
            {
                // Read associated fiels significance
                if (ops[1] != WR_VAR(0, 31, 21))
                    ds.in.parse_error("C04yyy is followed by %s instead of B31021", varcode_format(ops[1]).c_str());
                Varinfo info = d.out.btable->query(ops[1]);
                Var B31021(info);
                d.in.decode_number(B31021);
                current_adder->add_var(B31021);
                ++used;
                // Read B31021
                c04_meaning = B31021.enq(63);
            }
            c04_bits = WR_VAR_Y(code);
            break;
		case 5: {
			int cdatalen = WR_VAR_Y(code);
			char buf[cdatalen + 1];
			TRACE("decode_c_data:character data %d long\n", cdatalen);
			int i;
			for (i = 0; i < cdatalen; ++i)
			{
				uint32_t bitval = ds.in.get_bits(8);
				TRACE("decode_c_data:decoded character %d %c\n", (int)bitval, (char)bitval);
				buf[i] = bitval;
			}
			buf[i] = 0;

            // Add as C variable to the subset

            // Create a single use varinfo to store the bitmap
            MutableVarinfo info(MutableVarinfo::create_singleuse());
            info->set_string(code, "CHARACTER DATA", cdatalen);

            // Store the bitmap
            Var cdata(info, buf);

            // TODO: if compressed, extract the data from each subset? Store it in each dataset?
            if (d.out.compression)
                error_unimplemented::throwf("C05%03d character data found in compressed message and it is not clear how it should be handled", cdatalen);
            else
                current_adder->add_var(cdata);

            TRACE("decode_c_data:decoded string %s\n", buf);
            break;
        }
        case 6:
            if (WR_VAR_Y(code) > 32)
                error_unimplemented::throwf("C06 modifier found for %d bits but only at most 32 are supported", WR_VAR_Y(code));
            if (WR_VAR_Y(code))
            {
                bool skip = true;
                if (d.out.btable->contains(ops[1]))
                {
                    Varinfo info = get_info(ops[1]);
                    if (info->bit_len == WR_VAR_Y(code))
                    {
                        // If we can resolve the descriptor and the size is the
                        // same, attempt decoding
                        used += decode_b_data(ops.sub(1));
                        skip = false;
                    }
                }
                if (skip)
                {
                    MutableVarinfo info(MutableVarinfo::create_singleuse());
                    info->set(code, "UNKNOWN LOCAL DESCRIPTOR", "UNKNOWN", 0, 0,
                            ceil(log10(exp2(WR_VAR_Y(code)))), 0, WR_VAR_Y(code));
                    ds.decode_b_value(info, *current_adder);
                    used += 1;
                }
            }
            break;
        case 8: {
            int cdatalen = WR_VAR_Y(code);
            IFTRACE {
                if (cdatalen)
                    TRACE("decode_c_data:character size overridden to %d chars for all fields\n", cdatalen);
                else
                    TRACE("decode_c_data:character size overridde end\n");
            }
            c_string_len_override = cdatalen;
            break;
        }
        case 22:
            if (WR_VAR_Y(code) == 0)
            {
                used += decode_bitmap(ops.sub(1), code, *current_adder);
            } else
                ds.in.parse_error("decode_c_data:C modifier %d%02d%03d not yet supported",
                            WR_VAR_F(code),
                            WR_VAR_X(code),
                            WR_VAR_Y(code));
            break;
        case 23:
            if (WR_VAR_Y(code) == 0)
            {
                used += decode_bitmap(ops.sub(1), code, *current_adder);
            } else if (WR_VAR_Y(code) == 255) {
                if (!bitmap.bitmap)
                    ds.in.parse_error("C23255 found but there is no active bitmap");
                bitmap.next(ds);
                // Read substituted value

                // Use the details of the corrisponding variable for decoding
                Varinfo info = d.out.subsets[0][bitmap.subset_index].info();
                // Switch to 'add substituted value as attribute' mode
                SubstMode sm(*this);
                // Decode the value
                ds.decode_b_value(info, *current_adder);
            } else
                ds.in.parse_error("decode_c_data:C modifier %d%02d%03d not yet supported",
                        WR_VAR_F(code),
                        WR_VAR_X(code),
                        WR_VAR_Y(code));
            break;
        case 24:
            if (WR_VAR_Y(code) == 0)
            {
                used += decode_r_data(ops.sub(1));
            } else
                ds.in.parse_error("decode_c_data:C modifier %d%02d%03d not yet supported",
                            WR_VAR_F(code),
                            WR_VAR_X(code),
                            WR_VAR_Y(code));
            break;
        default:
            ds.in.parse_error("decode_c_data:C modifiers (%d%02d%03d in this case) are not yet supported",
                        WR_VAR_F(code),
                        WR_VAR_X(code),
                        WR_VAR_Y(code));
    }

    return used;
}

}
/* vim:set ts=4 sw=4: */
