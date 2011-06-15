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

struct Bitmap
{
    const Var* bitmap;
    const Subset* subset;
    vector<unsigned> refs;
    vector<unsigned>::const_reverse_iterator iter;
    unsigned old_anchor;

    Bitmap() : bitmap(0), subset(0) {}
    ~Bitmap() {}

    void reset()
    {
        bitmap = 0;
        subset = 0;
        old_anchor = 0;
        refs.clear();
        iter = refs.rend();
    }

    /**
     * Initialise the bitmap handler
     *
     * @param bitmap
     *   The bitmap
     * @param subset
     *   The subset to which the bitmap refers
     * @param anchor
     *   The index to the first element after the end of the bitmap (usually
     *   the C operator that defines or uses the bitmap)
     */
    void init(const Var& bitmap, const Subset& subset, unsigned anchor)
    {
        this->bitmap = &bitmap;
        this->subset = &subset;
        refs.clear();

        // From the specs it looks like bitmaps refer to all data that precedes
        // the C operator that defines or uses the bitmap, but from the data
        // samples that we have it look like when multiple bitmaps are present,
        // they always refer to the same set of variables. For this reason we
        // remember the first anchor point that we see and always refer the
        // other bitmaps that we see to it.
        if (old_anchor)
            anchor = old_anchor;
        else
            old_anchor = anchor;

        unsigned b_cur = bitmap.info()->len;
        unsigned s_cur = anchor;
        if (b_cur == 0) throw error_consistency("data present bitmap has length 0");
        if (s_cur == 0) throw error_consistency("data present bitmap is anchored at start of subset");

        while (true)
        {
            --b_cur;
            --s_cur;
            while (WR_VAR_F(subset[s_cur].code()) != 0)
            {
                if (s_cur == 0) throw error_consistency("bitmap refers to variables before the start of the subset");
                --s_cur;
            }

            if (bitmap.value()[b_cur] == '+')
                refs.push_back(s_cur);

            if (b_cur == 0)
                break;
            if (s_cur == 0)
                throw error_consistency("bitmap refers to variables before the start of the subset");
        }

        iter = refs.rbegin();
    }

    bool eob() const { return iter == refs.rend(); }
    unsigned next() { unsigned res = *iter; ++iter; return res; }
};

struct Interpreter : public opcode::Visitor
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

    /// Bitmap iteration
    Bitmap bitmap;

    /**
     * Number of extra bits inserted by the current C04yyy modifier (0 for no
     * C04yyy operator in use)
     */
    int c04_bits;
    /// Meaning of C04yyy field according to code table B31021
    int c04_meaning;

    // True if a Data Present Bitmap is expected
    bool want_bitmap;

    unsigned read_pos;

    Interpreter(const Bulletin& in, bulletin::DDSExecutor& out)
        : in(in), out(out),
          c_scale_change(0), c_width_change(0), c_string_len_override(0),
          c04_bits(0), c04_meaning(63), want_bitmap(false)
    {
    }

    void start()
    {
        c_scale_change = 0;
        c_width_change = 0;
        c_string_len_override = 0;
        bitmap.reset();
        c04_bits = 0;
        c04_meaning = 63;
        want_bitmap = false;
        read_pos = 0;
    }

    Varinfo get_varinfo(Varcode code);

    void b_variable(Varcode code)
    {
        Varinfo info = get_varinfo(code);
        // Choose which value we should encode
        if (WR_VAR_F(code) == 0 && WR_VAR_X(code) == 33 && !bitmap.eob())
        {
            // Attribute of the variable pointed by the bitmap
            unsigned target = bitmap.next();
            TRACE("Encode attribute %01d%02d%03d subset pos %u\n",
                    WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code), target);
            out.encode_attr(info, target, code);
        } else {
            // Proper variable
            TRACE("Encode variable %01d%02d%03d\n",
                    WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var));
            if (c04_bits > 0)
                out.encode_associated_field(c04_bits, c04_meaning);
            out.encode_var(info);
            ++read_pos;
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
            ++read_pos;
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
            ++read_pos;
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
        if (bitmap.bitmap == NULL)
            error_consistency::throwf("found C23255 with no active bitmap");
        if (bitmap.eob())
            error_consistency::throwf("found C23255 while at the end of active bitmap");
        unsigned target = bitmap.next();
        // Use the details of the corrisponding variable for decoding
        Varinfo info = in.subsets[0][target].info();
        // Encode the value
        out.encode_attr(info, target, info->var);
    }

    /* If using delayed replication and count is not -1, use count for the delayed
     * replication factor; else, look for a delayed replication factor among the
     * input variables */
    void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
    {
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
            if (count == 0 && delayed_code == 0)
                delayed_code = WR_VAR(0, 31, 12);
            const Var* bitmap_var = out.do_bitmap(code, delayed_code, ops);
            bitmap.init(*bitmap_var, *out.current_subset, read_pos);
            if (delayed_code)
                ++read_pos;
            want_bitmap = false;
        } else {
            if (count == 0)
            {
                Varinfo info = btable->query(delayed_code ? delayed_code : WR_VAR(0, 31, 12));
                Var var = out.encode_semantic_var(info);
                count = var.enqi();
                ++read_pos;
            }
            TRACE("encode_r_data %d items %d times%s\n", group, count, delayed_code ? " (delayed)" : "");
            IFTRACE {
                TRACE("Repeat opcodes: ");
                ops.print(stderr);
                TRACE("\n");
            }

            // encode_data_section on it `count' times
            out.push_repetition(group, count);
            for (int i = 0; i < count; ++i)
            {
                out.start_repetition();
                ops.visit(*this);
            }
            out.pop_repetition();
        }
    }
    virtual void d_group_begin(Varcode code)
    {
        out.push_dcode(code);
    }
    virtual void d_group_end(Varcode code)
    {
        out.pop_dcode();
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
        out.start_subset(i, subsets[i]);
        e.start();
        Opcodes(datadesc).visit(e);
        TRACE("run_dds: done encoding subset %u\n", i);
    }
}

} // wreport namespace

/* vim:set ts=4 sw=4: */
