#include "internals.h"
#include "wreport/var.h"
#include "wreport/subset.h"
#include "wreport/bulletin.h"
#include "wreport/notes.h"
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

void UncompressedEncoder::encode_var(Varinfo info, const Var& var)
{
    throw error_unimplemented("encode_var not implemented in this interpreter");
}

void UncompressedEncoder::encode_associated_field(const Var& var)
{
}

void UncompressedEncoder::define_variable(Varinfo info)
{
    encode_var(info, get_var());
}

void UncompressedEncoder::define_variable_with_associated_field(Varinfo info)
{
    const Var& var = get_var();
    encode_associated_field(var);
    encode_var(info, var);
}

unsigned UncompressedEncoder::define_delayed_replication_factor(Varinfo info)
{
    const Var& var = get_var();
    encode_var(info, var);
    return var.enqi();
}

unsigned UncompressedEncoder::define_associated_field_significance(Varinfo info)
{
    const Var& var = get_var();
    encode_var(info, var);
    return var.enq(63);
}

unsigned UncompressedEncoder::define_bitmap_delayed_replication_factor(Varinfo info)
{
    const Var& var = peek_var();
    Var rep_var(info, (int)var.info()->len);
    encode_var(info, rep_var);
    return var.info()->len;
}

}
}
