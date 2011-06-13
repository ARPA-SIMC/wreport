/*
 * wreport/ds_interpreter - Bulletin data descriptor section interpreter
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

#include <stdio.h>
#include <netinet/in.h>

#include <stdlib.h> /* malloc */
#include <ctype.h>  /* isspace */
#include <string.h> /* memcpy */
#include <stdarg.h> /* va_start, va_end */
#include <math.h>   /* NAN */
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

struct Interpreter : public opcode::Explorer
{
    const Bulletin& in;
    /// Input message data
    const Vartable* btable;
    /// Executor of the interpreter commands
    bulletin::DDSExecutor& out;

    /* Current value of scale change from C modifier */
    int c_scale_change;
    /* Current value of width change from C modifier */
    int c_width_change;
    /** Current value of string length override from C08 modifiers (0 for no
     * override)
     */
    int c_string_len_override;

    /* Subset we are encoding */
    //const Subset* subset;

    /* Set these to non-null if we are encoding a data present bitmap */
    const Var* bitmap_to_encode;
    int bitmap_use_cur;
    int bitmap_subset_cur;

    /**
     * Number of extra bits inserted by the current C04yyy modifier (0 for no
     * C04yyy operator in use)
     */
    int c04_bits;
    /// Meaning of C04yyy field according to code table B31021
    int c04_meaning;

    // True if a Data Present Bitmap is expected
    bool want_bitmap;

    Interpreter(const Bulletin& in, bulletin::DDSExecutor& out)
        : in(in), out(out),
          c_scale_change(0), c_width_change(0), c_string_len_override(0),
          bitmap_to_encode(0), bitmap_use_cur(0), bitmap_subset_cur(0),
          c04_bits(0), c04_meaning(63), want_bitmap(false)
    {
    }

    void start()
    {
        c_scale_change = 0;
        c_width_change = 0;
        c_string_len_override = 0;
        bitmap_to_encode = 0;
        bitmap_use_cur = 0;
        bitmap_subset_cur = 0;
        c04_bits = 0;
        c04_meaning = 63;
        want_bitmap = false;
    }

    Varinfo get_varinfo(Varcode code);

    void bitmap_next();

    void b_variable(Varcode code)
    {
        Varinfo info = get_varinfo(code);
        // Choose which value we should encode
        if (WR_VAR_F(code) == 0 && WR_VAR_X(code) == 33
                && bitmap_to_encode != NULL && (unsigned)bitmap_use_cur < bitmap_to_encode->info()->len)
        {
            // Attribute of the variable pointed by the bitmap
            TRACE("Encode attribute %01d%02d%03d %d/%d subset pos %d\n",
                    WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code),
                    bitmap_use_cur, bitmap_to_encode->info()->len,
                    bitmap_subset_cur);
            out.encode_attr(info, bitmap_subset_cur, code);
            bitmap_next();
        } else {
            // Proper variable
            TRACE("Encode variable %01d%02d%03d\n",
                    WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var));
            if (c04_bits > 0)
            {
                // TODO: only padding for now, implement retrieving the value
                out.encode_associated_field(c04_bits, 1);
#if 0
                    TRACE("decode_b_data:reading %d bits of C04 information\n", c04_bits);
                    uint32_t val = ds.get_bits(c04_bits);
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
#endif
            }

            out.encode_var(info);
        }
    }

    void c_modifier(Varcode code)
    {
        TRACE("C DATA %01d%02d%03d\n", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));
    }
    void c_change_data_width(Varcode code, int change)
    {
        c_width_change = change;
        TRACE("Set width change to %d\n", c_width_change);
    }
    void c_change_data_scale(Varcode code, int change)
    {
        c_scale_change = change;
        TRACE("Set scale change to %d\n", c_scale_change);
    }
    void c_associated_field(Varcode code, Varcode sig_code, unsigned nbits)
    {
        // Add associated field
        TRACE("Set C04 bits to %d\n", WR_VAR_Y(code));
        // FIXME: nested C04 modifiers are not currently implemented
        if (WR_VAR_Y(code) && c04_bits)
            throw error_unimplemented("nested C04 modifiers are not yet implemented");
        if (WR_VAR_Y(code) > 32)
            error_unimplemented::throwf("C04 modifier wants %d bits but only at most 32 are supported", WR_VAR_Y(code));
        if (WR_VAR_Y(code))
        {
            // Get encoding informations for this associated_field_significance
            Varinfo info = btable->query(WR_VAR(0, 31, 21));

            // Encode B31021
            Var var = out.encode_semantic_var(info);
            c04_meaning = var.enqi();
        }
        c04_bits = WR_VAR_Y(code);
    }
    void c_char_data(Varcode code)
    {
        out.encode_char_data(code);
    }
    void c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits)
    {
        // Length of next local descriptor
        if (WR_VAR_Y(code) > 32)
            error_unimplemented::throwf("C06 modifier found for %d bits but only at most 32 are supported", WR_VAR_Y(code));
        if (WR_VAR_Y(code))
        {
            bool skip = true;
            if (btable->contains(desc_code))
            {
                Varinfo info = get_varinfo(desc_code);
                if (info->bit_len == WR_VAR_Y(code))
                {
                    // If we can resolve the descriptor and the size is the
                    // same, attempt decoding
                    out.encode_var(info);
                    skip = false;
                }
            }
            if (skip)
            {
                MutableVarinfo info(MutableVarinfo::create_singleuse());
                info->set(code, "UNKNOWN LOCAL DESCRIPTOR", "UNKNOWN", 0, 0,
                        ceil(log10(exp2(WR_VAR_Y(code)))), 0, WR_VAR_Y(code), VARINFO_FLAG_STRING);
                out.encode_var(info);
            }
        }
    }
    void c_char_data_override(Varcode code, unsigned new_length)
    {
        IFTRACE {
            if (new_length)
                TRACE("decode_c_data:character size overridden to %d chars for all fields\n", new_length);
            else
                TRACE("decode_c_data:character size overridde end\n");
        }
        c_string_len_override = new_length;
    }
    void c_quality_information_bitmap(Varcode code)
    {
        // Quality information
        if (WR_VAR_Y(code) != 0)
            error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                        WR_VAR_F(code),
                        WR_VAR_X(code),
                        WR_VAR_Y(code));
        want_bitmap = true;
    }
    void c_substituted_value_bitmap(Varcode code)
    {
        want_bitmap = true;
    }
    void c_substituted_value(Varcode code)
    {
        if (bitmap_to_encode == NULL)
            error_consistency::throwf("found C23255 with no active bitmap");
        if ((unsigned)bitmap_use_cur >= bitmap_to_encode->info()->len)
            error_consistency::throwf("found C23255 while at the end of active bitmap");

        // Use the details of the corrisponding variable for decoding
        Varinfo info = in.subsets[0][bitmap_subset_cur].info();
        // Encode the value
        out.encode_attr(info, bitmap_subset_cur, info->var);
        bitmap_next();
    }

    /* If using delayed replication and count is not -1, use count for the delayed
     * replication factor; else, look for a delayed replication factor among the
     * input variables */
    void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
    {
        unsigned used = 1;
        int group = WR_VAR_X(code);
        int count = WR_VAR_Y(code);

        IFTRACE{
            TRACE("bufr_message_encode_r_data %01d%02d%03d %d %d: items: ",
                    WR_VAR_F(ops.head()), WR_VAR_X(ops.head()), WR_VAR_Y(ops.head()), group, count);
            ops.print(stderr);
            TRACE("\n");
        }

        if (want_bitmap)
        {
            bitmap_to_encode = out.get_bitmap();
            bitmap_use_cur = -1;
            bitmap_subset_cur = -1;
            bitmap_next();

            IFTRACE{
                TRACE("Encoding data present bitmap:");
                bitmap_to_encode->print(stderr);
            }

            if (count == 0)
            {
                Varinfo info = btable->query(delayed_code ? delayed_code : WR_VAR(0, 31, 12));
                count = out.encode_bitmap_repetition_count(info, *bitmap_to_encode);
            }
            TRACE("encode_r_data bitmap %d items %d times%s\n", group, count, delayed_code ? " (delayed)" : "");

            // Encode the bitmap here directly
            if (ops[0] != WR_VAR(0, 31, 31))
                error_consistency::throwf("bitmap data descriptor is %d%02d%03d instead of B31031",
                        WR_VAR_F(ops[used]), WR_VAR_X(ops[used]), WR_VAR_Y(ops[used]));
            if (ops.size() != 1)
                error_consistency::throwf("repeated sequence for bitmap encoding contains more than just B31031");

            out.encode_bitmap(*bitmap_to_encode);
            want_bitmap = false;
        } else {
            if (count == 0)
            {
                Varinfo info = btable->query(delayed_code ? delayed_code : WR_VAR(0, 31, 12));
                Var var = out.encode_semantic_var(info);
                count = var.enqi();
            }
            TRACE("encode_r_data %d items %d times%s\n", group, count, delayed_code ? " (delayed)" : "");
            IFTRACE {
                TRACE("Repeat opcodes: ");
                ops.print(stderr);
            }

            // encode_data_section on it `count' times
            out.push_repetition(group, count);
            for (int i = 0; i < count; ++i)
            {
                out.start_repetition();
                ops.explore(*this);
            }
            out.pop_repetition();
        }
    }
};

Varinfo Interpreter::get_varinfo(Varcode code)
{
    Varinfo peek = btable->query(code);

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
    return btable->query_altered(code, scale, bit_len);
}

void Interpreter::bitmap_next()
{
    if (bitmap_to_encode == NULL)
        throw error_consistency("applying a data present bitmap with no current bitmap");
    TRACE("bitmap_next pre %d %d %d\n", bitmap_use_cur, bitmap_subset_cur, bitmap_to_encode->info()->len);
    ++bitmap_use_cur;
    ++bitmap_subset_cur;
    while (bitmap_use_cur < 0 || (
                (unsigned)bitmap_use_cur < bitmap_to_encode->info()->len &&
                bitmap_to_encode->value()[bitmap_use_cur] == '-'))
    {
        TRACE("INCR\n");
        ++bitmap_use_cur;
        ++bitmap_subset_cur;

        while ((unsigned)bitmap_subset_cur < out.subset_size() &&
                out.is_special_var(bitmap_subset_cur))
            ++bitmap_subset_cur;
    }
    if ((unsigned)bitmap_use_cur > bitmap_to_encode->info()->len)
        throw error_consistency("moved past end of data present bitmap");
    if ((unsigned)bitmap_subset_cur == out.subset_size())
        throw error_consistency("end of data reached when applying attributes");
    TRACE("bitmap_next post %d %d\n", bitmap_use_cur, bitmap_subset_cur);
}

} // Unnamed namespace

void Bulletin::run_dds(bulletin::DDSExecutor& out) const
{
    Interpreter e(*this, out);
    e.btable = btable;
    e.dtable = dtable;

    /* Encode all the subsets, uncompressed */
    for (unsigned i = 0; i < subsets.size(); ++i)
    {
        TRACE("run_dds: start encoding subset %u\n", i);
        /* Encode the data of this subset */
        out.start_subset(i);
        e.start();
        Opcodes(datadesc).explore(e);
        TRACE("run_dds: done encoding subset %u\n", i);
    }
}

} // wreport namespace

/* vim:set ts=4 sw=4: */
