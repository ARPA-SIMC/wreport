#include "interpreter.h"
#include "wreport/error.h"
#include "wreport/notes.h"
#include "wreport/dtable.h"
#include "wreport/vartable.h"
#include "wreport/tables.h"
#include "wreport/var.h"

// #define TRACE_INTERPRETER

#ifdef TRACE_INTERPRETER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

namespace wreport {
namespace bulletin {

void DDSInterpreter::run()
{
    Opcodes opcodes = opcode_stack.top();
    while (!opcodes.empty())
    {
        Varcode cur = opcodes.pop_left();
        switch (WR_VAR_F(cur))
        {
            case 0: b_variable(cur); break;
            case 1: {
                // Replicate the next X elements Y times
                Varcode delayed_replication_code = 0;
                unsigned count = WR_VAR_Y(cur);
                if (count == 0 && !opcodes.empty())
                {
                    // Delayed replication, if replicator is there. In case of
                    // CREX, delayed replicator codes are implicit
                    Varcode next_code = opcodes[0];
                    if (WR_VAR_F(next_code) == 0 && WR_VAR_X(next_code) == 31)
                        delayed_replication_code = opcodes.pop_left();
                }

                // CREX has an implicit delayed replication code: use that if
                // none was defined.
                if (count == 0 && !delayed_replication_code)
                    delayed_replication_code = WR_VAR(0, 31, 12);

                if (bitmaps.pending_definitions)
                {
                    unsigned opcode_count = WR_VAR_X(cur);

                    // Sanity checks
                    if (opcode_count != 1)
                        error_consistency::throwf("bitmap section replicates %u descriptors instead of one", opcode_count);

                    Opcodes replicated = opcodes.pop_left(1);

                    if (replicated[0] != WR_VAR(0, 31, 31))
                        error_consistency::throwf("bitmap element descriptor is %01d%02d%03d instead of B31031", WR_VAR_FXY(replicated[0]));

                    if (!count)
                    {
                        Varinfo rep_info = tables.btable->query(delayed_replication_code);
                        count = define_bitmap_delayed_replication_factor(rep_info);
                        ++bitmaps.next_bitmap_anchor_point;
                    }

                    r_bitmap(count);
                }
                else
                {
                    r_replication(cur, delayed_replication_code, opcodes.pop_left(WR_VAR_X(cur)));
                }
                break;
            }
            case 2:
                // Generic notification
                c_modifier(cur, opcodes);
                break;
            case 3:
            {
                d_group_begin(cur);
                opcode_stack.push(tables.dtable->query(cur));
                run();
                opcode_stack.pop();
                d_group_end(cur);
                break;
            }
            default:
                error_consistency::throwf("cannot handle opcode %01d%02d%03d", WR_VAR_FXY(cur));
        }
    }
}

Varinfo DDSInterpreter::get_varinfo(Varcode code)
{
    Varinfo peek = tables.btable->query(code);

    if (!c_scale_change && !c_width_change && !c_string_len_override && !c_scale_ref_width_increase)
        return peek;

    int scale = peek->scale;
    if (c_scale_change)
    {
        TRACE("get_varinfo:applying %d scale change\n", c_scale_change);
        scale += c_scale_change;
    }

    int bit_len = peek->bit_len;
    if (peek->type == Vartype::String && c_string_len_override)
    {
        TRACE("get_varinfo:overriding string to %d bytes\n", c_string_len_override);
        bit_len = c_string_len_override * 8;
    }
    else if (c_width_change)
    {
        TRACE("get_varinfo:applying %d width change\n", c_width_change);
        bit_len += c_width_change;
    }

    if (c_scale_ref_width_increase)
    {
        TRACE("get_varinfo:applying %d increase of scale, ref, width\n", c_scale_ref_width_increase);
        // TODO: misses reference value adjustment
        scale += c_scale_ref_width_increase;
        bit_len += (10 * c_scale_ref_width_increase + 2) / 3;
        // c_ref *= 10**code
    }

    TRACE("get_info:requesting alteration scale:%d, bit_len:%d\n", scale, bit_len);
    return tables.btable->query_altered(code, scale, bit_len);
}

void DDSInterpreter::b_variable(Varcode code)
{
    Varinfo info = get_varinfo(code);
    // Choose which value we should encode
    if (WR_VAR_F(code) == 0 && WR_VAR_X(code) == 33 && bitmaps.active())
    {
        // Attribute of the variable pointed by the bitmap
        unsigned pos = bitmaps.next();
        TRACE("b_variable attribute %01d%02d%03d subset pos %u\n", WR_VAR_FXY(code), pos);
        define_attribute(info, pos);
    } else {
        // Proper variable
        TRACE("b_variable variable %01d%02d%03d\n",
                WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var));
        define_variable(info);
        ++bitmaps.next_bitmap_anchor_point;
    }
}

void DDSInterpreter::c_modifier(Varcode code, Opcodes& next)
{
    TRACE("C DATA %01d%02d%03d\n", WR_VAR_FXY(code));
    switch (WR_VAR_X(code))
    {
        case 1: {
            /*
             * Change data width: add Y-128 bits to the data width given for
             * each data element in table B, other than string variables, and
             * flag tables.
             */
            int change = WR_VAR_Y(code) ? WR_VAR_Y(code) - 128 : 0;
            TRACE("Set width change from %d to %d\n", c_width_change, change);
            c_width_change = change;
            break;
        }
        case 2: {
            /*
             * Change scale Add Y - 128 to Scale in Table B for elements that
             * are not code or flag tables.
             */
            int change = WR_VAR_Y(code) ? WR_VAR_Y(code) - 128 : 0;
            TRACE("Set scale change from %d to %d\n", c_scale_change, change);
            c_scale_change = change;

            break;
        }
        case 4: {
            /*
             * Add associated field.
             *
             * Precede each data element with Y bits of information. This
             * operation associates a data field (e.g. quality control
             * information) of Y bits with each data element.
             *
             * The Add Associated Field operator, whenever used, must be
             * immediately followed by the Class 31 Data description operator
             * qualifier 0 31 021 to indicate the meaning of the associated
             * fields.
             */
            unsigned nbits = WR_VAR_Y(code);
            TRACE("Set C04 bits to %d\n", nbits);
            // FIXME: nested C04 modifiers are not currently implemented
            if (nbits && associated_field.bit_count)
                throw error_unimplemented("nested C04 modifiers are not yet implemented");
            if (nbits > 32)
                error_unimplemented::throwf("C04 modifier wants %d bits but only at most 32 are supported", nbits);
            if (nbits)
            {
                Varcode sig_code = next.pop_left();
                if (sig_code != WR_VAR(0, 31, 21))
                    error_consistency::throwf("C04%03i modifier is followed by data descriptor %01d%02d%03d instead of B31021",
                            nbits, WR_VAR_FXY(sig_code));

                // Get encoding informations for this associated_field_significance
                Varinfo info = tables.btable->query(WR_VAR(0, 31, 21));

                // Get the value for B31021, defaulting to 63 if missing
                associated_field.significance = define_associated_field_significance(info);
                ++bitmaps.next_bitmap_anchor_point;
            }
            associated_field.bit_count = nbits;
            break;
        }
        case 5:
            /*
             * Signify character
             *
             * Y characters (CCITT International Alphabet No. 5) are inserted
             * as a data field of Y * 8 bits in length
             */
            define_raw_character_data(code);
            break;
        case 6: {
            /*
             * Signify data width for the immediately following local
             * descriptor.
             *
             * Y bits of data are described by the immediately following
             * descriptor.
             */
            Varcode desc_code = next.pop_left();
            // Length of next local descriptor
            if (unsigned nbits = WR_VAR_Y(code))
            {
                bool skip = true;
                if (tables.btable->contains(desc_code))
                {
                    Varinfo info = get_varinfo(desc_code);
                    if (info->bit_len == nbits)
                    {
                        // If we can resolve the descriptor and the size is the
                        // same, attempt decoding
                        define_variable(info);
                        skip = false;
                    }
                }
                if (skip)
                {
                    Varinfo info = tables.get_unknown(desc_code, nbits);
                    define_variable(info);
                }
                ++bitmaps.next_bitmap_anchor_point;
            }
            break;
        }
        case 7: {
            /*
             * Increase scale, reference value and data width.
             *
             * For Table B elements, which are not CCITTIA5, code or flag tables:
             *  1. Add Y to the existing scale factor
             *  2. Multiply the existing reference value by 10^Y
             *  3. Calculate ( ( 10 * Y ) + 2 ) / 3 , disregard any fractional
             *     remainder and add the result to the existing bit width.
             */
            int change = WR_VAR_Y(code);
            TRACE("Increase scale, reference value and data width by %d\n", change);
            c_scale_ref_width_increase = change;
            break;
        }
        case 8: {
            /*
             * Change width of CCITTIA5 field.
             *
             * Y characters (representing Y * 8 bits in length) replace the
             * specified data width given for each CCITTIA5 element in Table B.
             */
            int change = WR_VAR_Y(code);
            IFTRACE {
                if (change)
                    TRACE("decode_c_data:character size overridden to %d chars for all fields\n", change);
                else
                    TRACE("decode_c_data:character size overridde end\n");
            }
            c_string_len_override = change;
            break;
        }
        case 22:
            /*
             * Quality information follows.
             *
             * The values of class 33 elements which follow relate to the data
             * defined by the data present bit-map
             */
            if (WR_VAR_Y(code) != 0)
                error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                            WR_VAR_F(code),
                            WR_VAR_X(code),
                            WR_VAR_Y(code));
            bitmaps.pending_definitions = code;
            break;
        case 23:
            switch (WR_VAR_Y(code))
            {
                case 0:
                    /*
                     * Substituted values operator.
                     *
                     * The substituted values which follow relate to the data
                     * defined by the data present bit-map
                     */
                    bitmaps.pending_definitions = code;
                    break;
                case 255:
                    /*
                     * Substituted values marker operator.
                     *
                     * This operator shall signify a data item containing a
                     * substituted value; the element descriptor for the
                     * substituted value is obtained by the application of the
                     * data present bit-map associated with the substituted
                     * values operator
                     */
                    if (!bitmaps.active())
                        error_consistency::throwf("found C23255 while there is no active bitmap");
                    define_substituted_value(bitmaps.next());
                    break;
                default:
                    error_consistency::throwf("C modifier %d%02d%03d not yet supported", WR_VAR_FXY(code));
            }
            break;
        case 37:
            // Use defined data present bitmap
            switch (WR_VAR_Y(code))
            {
                case 0: // Reuse last defined bitmap
                    c_reuse_last_bitmap(true);
                    break;
                case 255: // cancels reuse of the last defined bitmap
                    c_reuse_last_bitmap(false);
                    break;
                default:
                    error_consistency::throwf("C modifier %d%02d%03d uses unsupported y=%03d",
                            WR_VAR_FXY(code), WR_VAR_Y(code));
                    break;
            }
            break;
            /*
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
            */
        default:
            notes::logf("ignoring unsupported C modifier %01d%02d%03d", WR_VAR_FXY(code));
            break;
            /*
            error_unimplemented::throwf("C modifier %d%02d%03d is not yet supported",
                        WR_VAR_F(code),
                        WR_VAR_X(code),
                        WR_VAR_Y(code));
            */
    }
}

void DDSInterpreter::c_reuse_last_bitmap(Varcode code) {}

void DDSInterpreter::r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
{
    // unsigned group = WR_VAR_X(code);
    unsigned count = WR_VAR_Y(code);

    IFTRACE{
        TRACE("visitor r_replication %01d%02d%03d, %u times, %u opcodes: ",
                WR_VAR_F(delayed_code), WR_VAR_X(delayed_code), WR_VAR_Y(delayed_code), count, WR_VAR_X(code));
        ops.print(stderr);
        TRACE("\n");
    }

    /* If using delayed replication and count is not 0, use count for the
     * delayed replication factor; else, look for a delayed replication
     * factor among the input variables */
    if (count == 0)
    {
        Varinfo info = tables.btable->query(delayed_code);
        count = define_delayed_replication_factor(info);
        ++bitmaps.next_bitmap_anchor_point;
    }
    IFTRACE {
        TRACE("visitor r_replication %d items %d times%s\n", WR_VAR_X(code), count, delayed_code ? " (delayed)" : "");
        TRACE("Repeat opcodes: ");
        ops.print(stderr);
        TRACE("\n");
    }

    // encode_data_section on it `count' times
    for (unsigned i = 0; i < count; ++i)
    {
        opcode_stack.push(ops);
        run();
        opcode_stack.pop();
    }
}

void DDSInterpreter::r_bitmap(unsigned bitmap_size)
{
    define_bitmap(bitmap_size);
    bitmaps.pending_definitions = 0;
}

void DDSInterpreter::d_group_begin(Varcode code) {}
void DDSInterpreter::d_group_end(Varcode code) {}

void DDSInterpreter::define_bitmap(unsigned bitmap_size)
{
    throw error_unimplemented("define_bitmap is not implemented in this interpreter");
}

unsigned DDSInterpreter::define_delayed_replication_factor(Varinfo info)
{
    throw error_unimplemented("define_delayed_replication_factor is not implemented in this interpreter");
}

unsigned DDSInterpreter::define_bitmap_delayed_replication_factor(Varinfo info)
{
    throw error_unimplemented("define_bitmap_delayed_replication_factor is not implemented in this interpreter");
}

unsigned DDSInterpreter::define_associated_field_significance(Varinfo info)
{
    throw error_unimplemented("define_associated_field_significance is not implemented in this interpreter");
}

void DDSInterpreter::define_variable(Varinfo info)
{
    throw error_unimplemented("define_variable is not implemented in this interpreter");
}

void DDSInterpreter::define_substituted_value(unsigned pos)
{
    throw error_unimplemented("define_substituted_variable is not implemented in this interpreter");
}

void DDSInterpreter::define_attribute(Varinfo info, unsigned pos)
{
    throw error_unimplemented("define_attribute is not implemented in this interpreter");
}

void DDSInterpreter::define_raw_character_data(Varcode code)
{
    throw error_unimplemented("define_raw_character_data is not implemented in this interpreter");
}


Printer::Printer(const Tables& tables, const Opcodes& opcodes)
    : DDSInterpreter(tables, opcodes), out(stdout), indent(0), indent_step(2)
{
}

void Printer::print_lead(Varcode code)
{
    fprintf(out, "%*s%d%02d%03d",
            indent, "", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));
}

void Printer::b_variable(Varcode code)
{
    print_lead(code);
    if (tables.btable)
    {
        if (tables.btable->contains(code))
        {
            Varinfo info = tables.btable->query(code);
            fprintf(out, " %s[%s]", info->desc, info->unit);
        } else
            fprintf(out, " (missing in B table %s)", tables.btable->pathname().c_str());
    }
    putc('\n', out);
}

void Printer::c_modifier(Varcode code, Opcodes& next)
{
    print_lead(code);
    switch (WR_VAR_X(code))
    {
        case 1:
            fprintf(out, " change data width to %d\n", WR_VAR_Y(code) ? WR_VAR_Y(code) - 128 : 0);
            break;
        case 2:
            fprintf(out, " change data scale to %d\n", WR_VAR_Y(code) ? WR_VAR_Y(code) - 128 : 0);
            break;
        case 4:
            fprintf(out, " %d bits of associated field\n", WR_VAR_Y(code));
            break;
        case 5:
            fputs(" character data\n", out);
            break;
        case 6:
            if (next.empty())
                fprintf(out, " local descriptor (unknown) %d bits long\n", WR_VAR_Y(code));
            else
                fprintf(out, " local descriptor %d%02d%03d %d bits long\n", WR_VAR_FXY(next[0]), WR_VAR_Y(code));
            break;
        case 7:
            fprintf(out, " change data scale, reference value and data width by %d\n", WR_VAR_Y(code));
            break;
        case 8:
            fprintf(out, " change width of string fields to %d\n", WR_VAR_Y(code));
            break;
        case 22:
            fputs(" quality information with bitmap\n", out);
            break;
        case 23:
            switch (WR_VAR_Y(code))
            {
                case 0:
                    fputs(" substituted values bitmap\n", out);
                    break;
                case 255:
                    fputs(" one substituted value\n", out);
                    break;
                default:
                    error_consistency::throwf("C modifier %d%02d%03d not yet supported", WR_VAR_FXY(code));
            }
            break;
        default:
            fputs(" (C modifier)\n", out);
            break;
    }
    DDSInterpreter::c_modifier(code, next);
}

void Printer::r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
{
    print_lead(code);
    unsigned group = WR_VAR_X(code);
    unsigned count = WR_VAR_Y(code);
    fprintf(out, " replicate %u descriptors", group);
    if (count)
        fprintf(out, " %u times\n", count);
    else
        fprintf(out, " (delayed %d%02d%03d) times\n",
                WR_VAR_F(delayed_code), WR_VAR_X(delayed_code), WR_VAR_Y(delayed_code));
    indent += indent_step;
    opcode_stack.push(ops);
    run();
    opcode_stack.pop();
    indent -= indent_step;
}

void Printer::d_group_begin(Varcode code)
{
    print_lead(code);
    fputs(" (group)\n", out);

    indent += indent_step;
}

void Printer::d_group_end(Varcode code)
{
    indent -= indent_step;
}

void Printer::c_reuse_last_bitmap(Varcode code)
{
    print_lead(code);
    switch (WR_VAR_Y(code))
    {
        case 0:
            fprintf(out, " reuse last data present bitmap\n");
            break;
        case 255:
            fprintf(out, " cancel reuse of last data present bitmap\n");
            break;
        default:
            fprintf(out, " unknown B37%03d reuse of last data present bitmap code\n", WR_VAR_Y(code));
            break;
    }
}

void Printer::define_variable(Varinfo info)
{
}

}
}
