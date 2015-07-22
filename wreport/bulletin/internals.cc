#include "internals.h"
#include "var.h"
#include "subset.h"
#include "bulletin.h"
#include "notes.h"
#include <cmath>

// #define TRACE_INTERPRETER

#ifdef TRACE_INTERPRETER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {
namespace bulletin {

UncompressedEncoder::UncompressedEncoder(const Bulletin& bulletin, unsigned subset_no)
    : Interpreter(bulletin.tables, bulletin.datadesc), current_subset(bulletin.subset(subset_no))
{
}

UncompressedEncoder::~UncompressedEncoder()
{
}

const Var& UncompressedEncoder::peek_var()
{
    return get_var(current_var);
}

const Var& UncompressedEncoder::get_var()
{
    return get_var(current_var++);
}

const Var& UncompressedEncoder::get_var(unsigned pos) const
{
    unsigned max_var = current_subset.size();
    if (pos >= max_var)
        error_consistency::throwf("cannot return variable #%u out of a maximum of %u", pos, max_var);
    return current_subset[pos];
}

void UncompressedEncoder::define_bitmap(unsigned bitmap_size)
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    bitmaps.define(var, current_subset, current_var);
}


UncompressedDecoder::UncompressedDecoder(Bulletin& bulletin, unsigned subset_no)
    : Interpreter(bulletin.tables, bulletin.datadesc), output_subset(bulletin.obtain_subset(subset_no))
{
}

UncompressedDecoder::~UncompressedDecoder()
{
}


CompressedDecoder::CompressedDecoder(Bulletin& bulletin)
    : Interpreter(bulletin.tables, bulletin.datadesc), output_bulletin(bulletin)
{
    TRACE("parser: start on compressed bulletin\n");
}

CompressedDecoder::~CompressedDecoder() {}

}
}
