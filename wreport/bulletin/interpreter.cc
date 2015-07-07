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
    auto& opcodes = opcode_stack.top();
    for (unsigned i = 0; i < opcodes.size(); ++i)
    {
        Varcode cur = opcodes[i];
        switch (WR_VAR_F(cur))
        {
            case 0: b_variable(cur); break;
            case 1: {
                Varcode rep_code = 0;
                Varcode next_code = opcodes[i+1];
                if (WR_VAR_Y(cur) == 0)
                {
                    // Delayed replication, if replicator is there. In case of
                    // CREX, delayed replicator codes are implicit
                    if (WR_VAR_F(next_code) == 0 && WR_VAR_X(next_code) == 31)
                    {
                        rep_code = opcodes[i+1];
                        ++i;
                    }
                }
                Opcodes ops = opcodes.sub(i + 1, WR_VAR_X(cur));
                r_replication(cur, rep_code, ops);
                i += WR_VAR_X(cur);
                break;
            }
            case 2:
                // Generic notification
                c_modifier(cur);
                // Specific notification
                switch (WR_VAR_X(cur))
                {
                    case 1:
                        c_change_data_width(cur, WR_VAR_Y(cur) ? WR_VAR_Y(cur) - 128 : 0);
                        break;
                    case 2:
                        c_change_data_scale(cur, WR_VAR_Y(cur) ? WR_VAR_Y(cur) - 128 : 0);
                        break;
                    case 4: {
                        Varcode sig_code = 0;
                        if (WR_VAR_Y(cur))
                        {
                            sig_code = opcodes[i + 1];
                            ++i;
                        }
                        c_associated_field(cur, sig_code, WR_VAR_Y(cur));
                        break;
                    }
                    case 5:
                        c_char_data(cur);
                        break;
                    case 6:
                        c_local_descriptor(cur, opcodes[i + 1], WR_VAR_Y(cur));
                        ++i;
                        break;
                    case 7:
                        c_increase_scale_ref_width(cur, WR_VAR_Y(cur));
                        break;
                    case 8:
                        c_char_data_override(cur, WR_VAR_Y(cur));
                        break;
                    case 22:
                        c_quality_information_bitmap(cur);
                        break;
                    case 23:
                        // Substituted values
                        switch (WR_VAR_Y(cur))
                        {
                            case 0:
                                c_substituted_value_bitmap(cur);
                                break;
                            case 255:
                                c_substituted_value(cur);
                                break;
                            default:
                                error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                                        WR_VAR_F(cur),
                                        WR_VAR_X(cur),
                                        WR_VAR_Y(cur));
                        }
                        break;
                    case 37:
                        // Use defined data present bitmap
                        switch (WR_VAR_Y(cur))
                        {
                            case 0: // Reuse last defined bitmap
                                c_reuse_last_bitmap(true);
                                break;
                            case 255: // cancels reuse of the last defined bitmap
                                c_reuse_last_bitmap(false);
                                break;
                            default:
                                error_consistency::throwf("C modifier %d%02d%03d uses unsupported y=%03d",
                                        WR_VAR_F(cur), WR_VAR_X(cur), WR_VAR_Y(cur), WR_VAR_Y(cur));
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
                        notes::logf("ignoring unsupported C modifier %01d%02d%03d",
                                    WR_VAR_F(cur), WR_VAR_X(cur), WR_VAR_Y(cur));
                        break;
                        /*
                        error_unimplemented::throwf("C modifier %d%02d%03d is not yet supported",
                                    WR_VAR_F(cur),
                                    WR_VAR_X(cur),
                                    WR_VAR_Y(cur));
                        */
                }
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
                error_consistency::throwf("cannot handle opcode %01d%02d%03d",
                    WR_VAR_F(cur), WR_VAR_X(cur), WR_VAR_Y(cur));
        }
    }
}


void DDSInterpreter::b_variable(Varcode code) {}

void DDSInterpreter::c_modifier(Varcode code)
{
    TRACE("C DATA %01d%02d%03d\n", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));
}

void DDSInterpreter::c_change_data_width(Varcode code, int change)
{
    TRACE("Set width change from %d to %d\n", c_width_change, change);
    c_width_change = change;
}

void DDSInterpreter::c_change_data_scale(Varcode code, int change)
{
    TRACE("Set scale change from %d to %d\n", c_scale_change, change);
    c_scale_change = change;
}

void DDSInterpreter::c_increase_scale_ref_width(Varcode code, int change)
{
    TRACE("Increase scale, reference value and data width by %d\n", change);
    c_scale_ref_width_increase = change;
}

void DDSInterpreter::c_associated_field(Varcode code, Varcode sig_code, unsigned nbits) {}
void DDSInterpreter::c_char_data(Varcode code) {}

void DDSInterpreter::c_char_data_override(Varcode code, unsigned new_length)
{
    IFTRACE {
        if (new_length)
            TRACE("decode_c_data:character size overridden to %d chars for all fields\n", new_length);
        else
            TRACE("decode_c_data:character size overridde end\n");
    }
    c_string_len_override = new_length;
}

void DDSInterpreter::c_quality_information_bitmap(Varcode code)
{
    // Quality information
    if (WR_VAR_Y(code) != 0)
        error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                    WR_VAR_F(code),
                    WR_VAR_X(code),
                    WR_VAR_Y(code));
    want_bitmap = code;
}

void DDSInterpreter::c_substituted_value_bitmap(Varcode code)
{
    want_bitmap = code;
}

void DDSInterpreter::c_substituted_value(Varcode code) {}
void DDSInterpreter::c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits) {}
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

    if (want_bitmap)
    {
        if (count == 0 && delayed_code == 0)
            delayed_code = WR_VAR(0, 31, 12);
        define_bitmap(want_bitmap, code, delayed_code, ops);
        // TODO: bitmap.init(bitmap_var, *current_subset, data_pos);
        if (delayed_code)
            ++data_pos;
        want_bitmap = 0;
    } else {
        /* If using delayed replication and count is not 0, use count for the
         * delayed replication factor; else, look for a delayed replication
         * factor among the input variables */
        if (count == 0)
        {
            Varinfo info = tables.btable->query(delayed_code ? delayed_code : WR_VAR(0, 31, 12));
            const Var& var = define_semantic_var(info);
            if (var.code() == WR_VAR(0, 31, 0))
            {
                count = var.isset() ? 0 : 1;
            } else {
                count = var.enqi();
            }
            ++data_pos;
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
}


void DDSInterpreter::d_group_begin(Varcode code) {}
void DDSInterpreter::d_group_end(Varcode code) {}

void DDSInterpreter::define_bitmap(Varcode code, Varcode rep_code, Varcode delayed_code, const Opcodes& ops)
{
    throw error_unimplemented("define_bitmap is not implemented in this interpreter");
}

const Var& DDSInterpreter::define_semantic_var(Varinfo info)
{
    throw error_unimplemented("define_semantic_var is not implemented in this interpreter");
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

void Printer::c_modifier(Varcode code)
{
    print_lead(code);
    fputs(" (C modifier)\n", out);
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

void Printer::c_change_data_width(Varcode code, int change)
{
    print_lead(code);
    fprintf(out, " change data width to %d\n", change);
}
void Printer::c_change_data_scale(Varcode code, int change)
{
    print_lead(code);
    fprintf(out, " change data scale to %d\n", change);
}
void Printer::c_associated_field(Varcode code, Varcode sig_code, unsigned nbits)
{
    print_lead(code);
    fprintf(out, " %d bits of associated field, significance code %d%02d%03d\n",
           nbits, WR_VAR_F(sig_code), WR_VAR_X(sig_code), WR_VAR_Y(sig_code));
}
void Printer::c_char_data(Varcode code)
{
    print_lead(code);
    fputs(" character data\n", out);
}
void Printer::c_char_data_override(Varcode code, unsigned new_length)
{
    print_lead(code);
    fprintf(out, " override character data length to %d\n", new_length);
}
void Printer::c_quality_information_bitmap(Varcode code)
{
    print_lead(code);
    fputs(" quality information with bitmap\n", out);
}
void Printer::c_substituted_value_bitmap(Varcode code)
{
    print_lead(code);
    fputs(" substituted values bitmap\n", out);
}
void Printer::c_substituted_value(Varcode code)
{
    print_lead(code);
    fputs(" one substituted value\n", out);
}
void Printer::c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits)
{
    print_lead(code);
    fprintf(out, " local descriptor %d%02d%03d %d bits long\n",
            WR_VAR_F(desc_code), WR_VAR_X(desc_code), WR_VAR_Y(desc_code), nbits);
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

}
}
