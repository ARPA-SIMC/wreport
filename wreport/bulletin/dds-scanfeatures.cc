#include "dds-scanfeatures.h"

namespace wreport {
namespace bulletin {

ScanFeatures::ScanFeatures(const Tables& tables, const Opcodes& opcodes)
    : Interpreter(tables, opcodes)
{
}

void ScanFeatures::c_modifier(Varcode code, Opcodes& next)
{
    switch (WR_VAR_X(code))
    {
        case 6:
            if (next.empty())
                features.insert(varcode_format(code));
            else
                features.insert(varcode_format(code) + ":" +
                                varcode_format(next[0]));
            break;
        default: features.insert(varcode_format(code)); break;
    }
    Interpreter::c_modifier(code, next);
}

void ScanFeatures::r_replication(Varcode code, Varcode delayed_code,
                                 const Opcodes& ops)
{
    if (delayed_code)
        features.insert("Rxx000:" + varcode_format(delayed_code));
    else
        features.insert("Rxxyyy");
    opcode_stack.push(ops);
    run();
    opcode_stack.pop();
}

void ScanFeatures::define_variable(Varinfo info) {}

void ScanFeatures::define_bitmap(unsigned bitmap_size) {}

unsigned ScanFeatures::define_associated_field_significance(Varinfo info)
{
    return 63;
}

unsigned ScanFeatures::define_bitmap_delayed_replication_factor(Varinfo info)
{
    return 0;
}

} // namespace bulletin
} // namespace wreport
