#include "opcode.h"

using namespace std;

namespace wreport {

void Opcodes::print(FILE* out) const
{
    if (begin == end)
        fprintf(out, "(empty)");
    else
        for (const Varcode* i = begin; i < end; ++i)
            fprintf(out, "%d%02d%03d ", WR_VAR_FXY(*i));
}

}
