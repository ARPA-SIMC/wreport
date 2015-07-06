#include "interpreter.h"
#include "wreport/error.h"
#include "wreport/notes.h"
#include "wreport/dtable.h"
#include "wreport/vartable.h"
#include "wreport/tables.h"

namespace wreport {
namespace bulletin {

void Interpreter::run()
{
    for (unsigned i = 0; i < opcodes.size(); ++i)
    {
        Varcode cur = opcodes[i];
        switch (WR_VAR_F(cur))
        {
            case 0: visitor.b_variable(cur); break;
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
                visitor.r_replication(cur, rep_code, ops);
                i += WR_VAR_X(cur);
                break;
            }
            case 2:
                // Generic notification
                visitor.c_modifier(cur);
                // Specific notification
                switch (WR_VAR_X(cur))
                {
                    case 1:
                        visitor.c_change_data_width(cur, WR_VAR_Y(cur) ? WR_VAR_Y(cur) - 128 : 0);
                        break;
                    case 2:
                        visitor.c_change_data_scale(cur, WR_VAR_Y(cur) ? WR_VAR_Y(cur) - 128 : 0);
                        break;
                    case 4: {
                        Varcode sig_code = 0;
                        if (WR_VAR_Y(cur))
                        {
                            sig_code = opcodes[i + 1];
                            ++i;
                        }
                        visitor.c_associated_field(cur, sig_code, WR_VAR_Y(cur));
                        break;
                    }
                    case 5:
                        visitor.c_char_data(cur);
                        break;
                    case 6:
                        visitor.c_local_descriptor(cur, opcodes[i + 1], WR_VAR_Y(cur));
                        ++i;
                        break;
                    case 7:
                        visitor.c_increase_scale_ref_width(cur, WR_VAR_Y(cur));
                        break;
                    case 8:
                        visitor.c_char_data_override(cur, WR_VAR_Y(cur));
                        break;
                    case 22:
                        visitor.c_quality_information_bitmap(cur);
                        break;
                    case 23:
                        // Substituted values
                        switch (WR_VAR_Y(cur))
                        {
                            case 0:
                                visitor.c_substituted_value_bitmap(cur);
                                break;
                            case 255:
                                visitor.c_substituted_value(cur);
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
                                visitor.c_reuse_last_bitmap(true);
                                break;
                            case 255: // cancels reuse of the last defined bitmap
                                visitor.c_reuse_last_bitmap(false);
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
                visitor.d_group_begin(cur);
                Interpreter interpreter(tables, tables.dtable->query(cur), visitor);
                interpreter.run();
                visitor.d_group_end(cur);
                break;
            }
            default:
                error_consistency::throwf("cannot handle opcode %01d%02d%03d",
                    WR_VAR_F(cur), WR_VAR_X(cur), WR_VAR_Y(cur));
        }
    }
}


Visitor::Visitor(const Tables& tables) : tables(&tables) {}
Visitor::~Visitor() {}
void Visitor::b_variable(Varcode code) {}
void Visitor::c_modifier(Varcode code) {}
void Visitor::c_change_data_width(Varcode code, int change) {}
void Visitor::c_change_data_scale(Varcode code, int change) {}
void Visitor::c_increase_scale_ref_width(Varcode code, int change) {}
void Visitor::c_associated_field(Varcode code, Varcode sig_code, unsigned nbits) {}
void Visitor::c_char_data(Varcode code) {}
void Visitor::c_char_data_override(Varcode code, unsigned new_length) {}
void Visitor::c_quality_information_bitmap(Varcode code) {}
void Visitor::c_substituted_value_bitmap(Varcode code) {}
void Visitor::c_substituted_value(Varcode code) {}
void Visitor::c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits) {}
void Visitor::c_reuse_last_bitmap(Varcode code) {}
void Visitor::r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops) {}
void Visitor::d_group_begin(Varcode code) {}
void Visitor::d_group_end(Varcode code) {}


Printer::Printer(const Tables& tables)
    : Visitor(tables), out(stdout), btable(0), indent(0), indent_step(2)
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
    if (btable)
    {
        if (btable->contains(code))
        {
            Varinfo info = btable->query(code);
            fprintf(out, " %s[%s]", info->desc, info->unit);
        } else
            fprintf(out, " (missing in B table %s)", btable->pathname().c_str());
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
    Interpreter interpreter(*tables, ops, *this);
    interpreter.run();
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
