#include "bitmaps.h"
#include "wreport/var.h"
#include "wreport/subset.h"

namespace wreport {
namespace bulletin {

Bitmap::Bitmap()
{
    iter = refs.rend();
}

Bitmap::~Bitmap()
{
    delete bitmap;
}

void Bitmap::init(const Var& bitmap, const Subset& subset, unsigned anchor)
{
    delete this->bitmap;
    this->bitmap = new Var(bitmap);
    refs.clear();

    // From the specs it looks like bitmaps refer to all data that precedes
    // the C operator that defines or uses the bitmap, but from the data
    // samples that we have it look like when multiple bitmaps are present,
    // they always refer to the same set of variables. For this reason we
    // remember the first anchor point that we see and always refer the
    // other bitmaps that we see to it.
    if (old_anchor)
        anchor = old_anchor;
    else
        old_anchor = anchor;

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

bool Bitmap::eob() const { return iter == refs.rend(); }
unsigned Bitmap::next() { unsigned res = *iter; ++iter; return res; }


Bitmaps::~Bitmaps()
{
    delete current;
}

unsigned Bitmaps::next()
{
    if (!current)
        throw error_consistency("bitmap iteration requested when no bitmap is currently active");
    unsigned res = current->next();
    if (current->eob())
    {
        delete current;
        current = 0;
    }
    return res;
}

void Bitmaps::define(const Var& bitmap, const Subset& subset, unsigned anchor)
{
    delete current;
    current = new Bitmap;
    current->init(bitmap, subset, anchor);
}

}
}
