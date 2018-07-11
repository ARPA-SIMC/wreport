#include "bitmaps.h"
#include "wreport/var.h"
#include "wreport/subset.h"

namespace wreport {
namespace bulletin {

Bitmap::Bitmap(const Var& bitmap, const Subset& subset)
    : Bitmap(bitmap, subset, subset.size())
{
}

Bitmap::Bitmap(const Var& bitmap, const Subset& subset, unsigned anchor)
    : bitmap(bitmap)
{
//    /**
//     * Anchor point of the first bitmap found since the last reset().
//     *
//     * From the specs it looks like bitmaps refer to all data that precedes the
//     * C operator that defines or uses them, but from the data samples that we
//     * have it look like when multiple bitmaps are present, they always refer
//     * to the same set of variables.
//     *
//     * For this reason we remember the first anchor point that we see and
//     * always refer the other bitmaps that we see to it.
//     */
//  FIXME: we do not seem to currently do that and all seems fine; do we
//  actually have samples where this matters?

    unsigned b_cur = bitmap.info()->len;
    unsigned s_cur = anchor;
    if (b_cur == 0) throw error_consistency("data present bitmap has length 0");
    if (s_cur == 0) throw error_consistency("data present bitmap is anchored at start of subset");

    while (true)
    {
        --b_cur;
        --s_cur;
        while (WR_VAR_F(subset[s_cur].code()) != 0)
        {
            if (s_cur == 0) throw error_consistency("bitmap refers to variables before the start of the subset");
            --s_cur;
        }

        if (bitmap.enqc()[b_cur] == '+')
            refs.push_back(s_cur);

        if (b_cur == 0)
            break;
        if (s_cur == 0)
            throw error_consistency("bitmap refers to variables before the start of the subset");
    }

    iter = refs.rbegin();
}

Bitmap::~Bitmap()
{
}

bool Bitmap::eob() const { return iter == refs.rend(); }
unsigned Bitmap::next() { unsigned res = *iter; ++iter; return res; }

void Bitmap::reuse()
{
    iter = refs.rbegin();
}


Bitmaps::~Bitmaps()
{
    delete current;
    delete last;
}

unsigned Bitmaps::next()
{
    if (!current)
        throw error_consistency("bitmap iteration requested when no bitmap is currently active");
    unsigned res = current->next();
    if (current->eob())
    {
        delete last;
        last = current;
        current = nullptr;
    }
    return res;
}

void Bitmaps::define(const Var& bitmap, const Subset& subset)
{
    define(bitmap, subset, subset.size());
}

void Bitmaps::define(const Var& bitmap, const Subset& subset, unsigned anchor_point)
{
    delete current;
    current = new Bitmap(bitmap, subset, anchor_point);
}

void Bitmaps::reuse_last()
{
// Only throw an error when the bitmap is actually used
//    if (!last)
//        throw error_consistency("cannot reuse bitmap, because no bitmap is currently defined");
    delete current;
    current = last;
    last = nullptr;
    if (current) current->reuse();
}

void Bitmaps::discard_last()
{
    delete last;
    last = nullptr;
}

}
}
