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

struct Interpreter
{
    /// Input message data
    const Bulletin& in;
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

    // True if the bulletin is a CREX bulletin
    bool is_crex;

    Interpreter(const Bulletin& in, bulletin::DDSExecutor& out)
        : in(in), out(out),
          c_scale_change(0), c_width_change(0), c_string_len_override(0),
          bitmap_to_encode(0), bitmap_use_cur(0), bitmap_subset_cur(0),
          c04_bits(0), c04_meaning(63)
    {
        is_crex = dynamic_cast<const CrexBulletin*>(&in) != NULL;
    }

    Varinfo get_varinfo(Varcode code);

    void bitmap_next();

    unsigned do_b_data(const Opcodes& ops, unsigned& var_pos);
    unsigned do_r_data(const Opcodes& ops, unsigned& var_pos, const Var* bitmap=NULL);
    unsigned do_c_data(const Opcodes& ops, unsigned& var_pos);
    unsigned do_bitmap(const Opcodes& ops, unsigned& var_pos);
    void do_data_section(const Opcodes& ops, unsigned& var_pos);

    // Run the interpreter, sending commands to the executor
    void run();
};

Varinfo Interpreter::get_varinfo(Varcode code)
{
    Varinfo peek = in.btable->query(code);

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
    return in.btable->query_altered(code, scale, bit_len);
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

unsigned Interpreter::do_b_data(const Opcodes& ops, unsigned& var_pos)
{
    Varinfo info = get_varinfo(ops.head());

    // Choose which value we should encode
    if (WR_VAR_F(ops.head()) == 0 && WR_VAR_X(ops.head()) == 33
            && bitmap_to_encode != NULL && (unsigned)bitmap_use_cur < bitmap_to_encode->info()->len)
    {
        // Attribute of the variable pointed by the bitmap
        TRACE("Encode attribute %01d%02d%03d %d/%d subset pos %d\n",
                WR_VAR_F(ops.head()), WR_VAR_X(ops.head()), WR_VAR_Y(ops.head()),
                bitmap_use_cur, bitmap_to_encode->info()->len,
                bitmap_subset_cur);
        out.encode_attr(info, bitmap_subset_cur, ops.head());
        bitmap_next();
    } else {
        // Proper variable
        TRACE("Encode variable %01d%02d%03d var pos %d\n",
                WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var),
                var_pos);
        out.encode_var(info, var_pos);
        ++var_pos;
    }

    return 1;
}

/* If using delayed replication and count is not -1, use count for the delayed
 * replication factor; else, look for a delayed replication factor among the
 * input variables */
unsigned Interpreter::do_r_data(const Opcodes& ops, unsigned& var_pos, const Var* bitmap)
{
    unsigned used = 1;
    int group = WR_VAR_X(ops.head());
    int count = WR_VAR_Y(ops.head());

    IFTRACE{
        TRACE("bufr_message_encode_r_data %01d%02d%03d %d %d: items: ",
                WR_VAR_F(ops.head()), WR_VAR_X(ops.head()), WR_VAR_Y(ops.head()), group, count);
        ops.print(stderr);
        TRACE("\n");
    }

    if (count == 0)
    {
        // Delayed replication

        // Note: CREX messages do NOT have the repetition count opcode in their
        // data descriptor section, so we need to act accordingly here

        // Get encoding informations for this repetition count
        Varinfo info = in.btable->query(is_crex ? WR_VAR(0, 31, 12) : ops[used]);

        if (bitmap == NULL)
        {
            /* Look for a delayed replication factor in the input vars */

            /* Get the repetition count */
            count = out.encode_repetition_count(info, var_pos);
            ++var_pos;

            TRACE("delayed replicator count read as %d\n", count);
        } else {
            count = out.encode_bitmap_repetition_count(info, *bitmap);

            TRACE("delayed replicator count passed by caller as %d\n", count);
        }

        /* Move past the node with the repetition count */
        if (!is_crex)
            ++used;

        TRACE("encode_r_data %d items %d times (delayed)\n", group, count);
    } else
        TRACE("encode_r_data %d items %d times\n", group, count);

    if (bitmap)
    {
        // Encode the bitmap here directly
        if (ops[used] != WR_VAR(0, 31, 31))
            error_consistency::throwf("bitmap data descriptor is %d%02d%03d instead of B31031",
                    WR_VAR_F(ops[used]), WR_VAR_X(ops[used]), WR_VAR_Y(ops[used]));

        out.encode_bitmap(*bitmap);
        ++var_pos;
        TRACE("Encoded %d bitmap entries\n", bitmap->info()->len);
    } else {
        // Extract the first `group' nodes, to handle here
        Opcodes group_ops = ops.sub(used, group);
        IFTRACE {
            TRACE("Repeat opcodes: ");
            group_ops.print(stderr);
        }

        // encode_data_section on it `count' times
        out.push_repetition(group, count);
        for (int i = 0; i < count; ++i)
        {
            out.start_repetition();
            do_data_section(group_ops, var_pos);
        }
        out.pop_repetition();
    }

    return used + group;
}

void Interpreter::run()
{
    /* Encode all the subsets, uncompressed */
    for (unsigned i = 0; i < in.subsets.size(); ++i)
    {
        /* Encode the data of this subset */
        out.start_subset(i);
        unsigned var_pos = 0;
        do_data_section(Opcodes(in.datadesc), var_pos);
    }
}


unsigned Interpreter::do_bitmap(const Opcodes& ops, unsigned& var_pos)
{
    unsigned used = 0;

    bitmap_to_encode = out.get_bitmap(var_pos);
    ++used;
    bitmap_use_cur = -1;
    bitmap_subset_cur = -1;

    IFTRACE{
        TRACE("Encoding data present bitmap:");
        bitmap_to_encode->print(stderr);
    }

    /* Encode the bitmap */
    used += do_r_data(ops.sub(used), var_pos, bitmap_to_encode);

    /* Point to the first attribute to encode */
    bitmap_next();

    /*
        TRACE("Decoded bitmap count %d: ", bitmap_count);
        for (size_t i = 0; i < dba_var_info(bitmap)->len; ++i)
        TRACE("%c", dba_var_value(bitmap)[i]);
        TRACE("\n");
        */
    return used;
}

unsigned Interpreter::do_c_data(const Opcodes& ops, unsigned& var_pos)
{
    Varcode code = ops.head();
    unsigned used = 1;

    TRACE("C DATA %01d%02d%03d\n", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));

    switch (WR_VAR_X(code))
    {
        case 1:
            // Change data width
            c_width_change = WR_VAR_Y(code) ? WR_VAR_Y(code) - 128 : 0;
            TRACE("Set width change to %d\n", c_width_change);
            break;
        case 2:
            // Change data scale
            c_scale_change = WR_VAR_Y(code) ? WR_VAR_Y(code) - 128 : 0;
            TRACE("Set scale change to %d\n", c_scale_change);
            break;
        case 4:
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
                Varinfo info = in.btable->query(WR_VAR(0, 31, 21));

                // Encode B31021
                c04_meaning = out.encode_repetition_count(info, var_pos);

                ++var_pos;
                ++used;
            }
            c04_bits = WR_VAR_Y(code);
            break;
        case 5:
            // Encode character data
            out.encode_char_data(code, var_pos);
            ++var_pos;
            break;
        case 6:
            // Length of next local descriptor
            if (WR_VAR_Y(code) > 32)
                error_unimplemented::throwf("C06 modifier found for %d bits but only at most 32 are supported", WR_VAR_Y(code));
            if (WR_VAR_Y(code))
            {
                bool skip = true;
                if (in.btable->contains(ops[1]))
                {
                    Varinfo info = get_varinfo(ops[1]);
                    if (info->bit_len == WR_VAR_Y(code))
                    {
                        // If we can resolve the descriptor and the size is the
                        // same, attempt decoding
                        used += do_b_data(ops.sub(1), var_pos);
                        skip = false;
                    }
                }
                if (skip)
                {
                    // Encode all bits all missing
                    out.encode_padding(WR_VAR_Y(code), true);
                    used += 1;
                }
            }
            break;
        case 8: {
            // Override length of character data
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
            // Quality information
            if (WR_VAR_Y(code) == 0)
            {
                used = do_bitmap(ops, var_pos);
            } else
                error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                            WR_VAR_F(code),
                            WR_VAR_X(code),
                            WR_VAR_Y(code));
            break;
        case 23:
            // Substituted values
            if (WR_VAR_Y(code) == 0)
            {
                used = do_bitmap(ops, var_pos);
            } else if (WR_VAR_Y(code) == 255) {
                if (bitmap_to_encode == NULL)
                    error_consistency::throwf("found C23255 with no active bitmap");
                if ((unsigned)bitmap_use_cur >= bitmap_to_encode->info()->len)
                    error_consistency::throwf("found C23255 while at the end of active bitmap");

                // Use the details of the corrisponding variable for decoding
                Varinfo info = in.subsets[0][bitmap_subset_cur].info();
                // Encode the value
                out.encode_attr(info, bitmap_subset_cur, info->var);
                bitmap_next();
            } else
                error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                        WR_VAR_F(code),
                        WR_VAR_X(code),
                        WR_VAR_Y(code));
            break;
        case 24:
            // First order statistical values
            if (WR_VAR_Y(code) == 0)
            {
                used += do_r_data(ops.sub(1), var_pos);
            } else
                error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                            WR_VAR_F(code),
                            WR_VAR_X(code),
                            WR_VAR_Y(code));
            break;
        default:
            error_unimplemented::throwf("C modifier %d%02d%03d is not yet supported",
                        WR_VAR_F(code),
                        WR_VAR_X(code),
                        WR_VAR_Y(code));
    }
    return used;
}


void Interpreter::do_data_section(const Opcodes& ops, unsigned& var_pos)
{
    TRACE("bufr_message_encode_data_section: START\n");

    for (unsigned i = 0; i < ops.size(); )
    {
        IFTRACE{
            TRACE("bufr_message_encode_data_section TODO: ");
            ops.sub(i).print(stderr);
            TRACE("\n");
        }

        switch (WR_VAR_F(ops[i]))
        {
            case 0: i += do_b_data(ops.sub(i), var_pos); break;
            case 1: i += do_r_data(ops.sub(i), var_pos); break;
            case 2: i += do_c_data(ops.sub(i), var_pos); break;
            case 3:
            {
                out.push_dcode(ops[i]);
                Opcodes exp = in.dtable->query(ops[i]);
                do_data_section(exp, var_pos);
                out.pop_dcode();
                ++i;
                break;
            }
            default:
                error_consistency::throwf(
                        "variable %01d%02d%03d cannot be handled",
                            WR_VAR_F(ops[i]),
                            WR_VAR_X(ops[i]),
                            WR_VAR_Y(ops[i]));
        }
    }
}

} // Unnamed namespace

void Bulletin::run_dds(bulletin::DDSExecutor& out) const
{
    Interpreter e(*this, out);
    e.run();
}

} // wreport namespace

/* vim:set ts=4 sw=4: */
