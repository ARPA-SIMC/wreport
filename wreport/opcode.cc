#include "config.h"

#include "opcode.h"
#include "vartable.h"
#include "dtable.h"
#include "error.h"
#include "notes.h"
#include <stdio.h>

using namespace std;

namespace wreport {

void Opcodes::print(FILE* out) const
{
    if (begin == end)
        fprintf(out, "(empty)");
    else
        for (unsigned i = begin; i < end; ++i)
            fprintf(out, "%d%02d%03d ", WR_VAR_F(vals[i]), WR_VAR_X(vals[i]), WR_VAR_Y(vals[i]));
}

#if 0
void Opcodes::visit(opcode::Visitor& e, const DTable& dtable) const
{
    e.dtable = &dtable;
    visit(e);
}

void Opcodes::visit(opcode::Visitor& e) const
{
}
#endif

}
