#include "wreport/bulletin/dds-printer.h"

using namespace std;

namespace wreport {
namespace bulletin {

DDSPrinter::DDSPrinter(const Bulletin& b, FILE* out, unsigned subset_idx)
    : UncompressedEncoder(b, subset_idx), out(out), subset_no(subset_idx)
{
}

DDSPrinter::~DDSPrinter() {}

void DDSPrinter::print_context(Varinfo info, unsigned var_pos)
{
    print_context(info->code, var_pos);
}

void DDSPrinter::print_context(Varcode code, unsigned var_pos)
{
    fprintf(out, "%2u.%2u ", subset_no, var_pos);
    for (vector<Varcode>::const_iterator i = stack.begin();
            i != stack.end(); ++i)
        fprintf(out, "%01d%02d%03d/", WR_VAR_F(*i), WR_VAR_X(*i), WR_VAR_Y(*i));
    fprintf(out, "%01d%02d%03d: ", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));
}

void DDSPrinter::run_d_expansion(Varcode code)
{
    stack.push_back(code);
    UncompressedEncoder::run_d_expansion(code);
    stack.pop_back();
}

void DDSPrinter::r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
{
    stack.push_back(code);
    UncompressedEncoder::r_replication(code, delayed_code, ops);
    stack.pop_back();
}

void DDSPrinter::print_attr(Varinfo info, unsigned var_pos)
{
    print_context(info, var_pos);
    fprintf(out, "(attr)");

    const Var& var = get_var(var_pos);
    if (const Var* a = var.enqa(info->code))
        a->print(out);
    else
        fprintf(out, "(undef)");
}

void DDSPrinter::encode_associated_field(const Var& var)
{
    print_context(var.info(), current_var);

    const Var* att = associated_field.get_attribute(var);
    if (att)
        att->print(out);
    else
        fprintf(out, "associated field with significance %u is not present", associated_field.significance);
}

void DDSPrinter::encode_var(Varinfo info, const Var& var)
{
    print_context(info, current_var);
    var.print(out);
}

void DDSPrinter::define_bitmap(unsigned bitmap_size)
{
    UncompressedEncoder::define_bitmap(bitmap_size);
#if 0
    if (delayed_code)
    {
        Varinfo info = tables.btable->query(delayed_code);
        print_context(info, 0);
        Var var(info, (int)bitmaps.current->bitmap.info()->len);
        var.print(out);
    }
#endif
    print_context(bitmaps.current->bitmap.info(), 0);
    bitmaps.current->bitmap.print(out);
}

void DDSPrinter::define_raw_character_data(Varcode code)
{
    print_context(code, 0);

    const Var& var = get_var();
    var.print(out);
}

void DDSPrinter::define_substituted_value(unsigned pos)
{
    // Use the details of the corrisponding variable for decoding
    Varinfo info = current_subset[pos].info();
    print_attr(info, pos);
}

void DDSPrinter::define_attribute(Varinfo info, unsigned pos)
{
    print_attr(info, pos);
}

}
}
