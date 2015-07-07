#ifndef WREPORT_BULLETIN_BITMAPS_H
#define WREPORT_BULLETIN_BITMAPS_H

#include <vector>

namespace wreport {
struct Var;
struct Subset;

namespace bulletin {

/// Associate a Data Present Bitmap to decoded variables in a subset
struct Bitmap
{
    /// Bitmap being iterated
    Var* bitmap = nullptr;

    /**
     * Arrays of variable indices corresponding to positions in the bitmap
     * where data is present
     */
    std::vector<unsigned> refs;

    /**
     * Iterator over refs
     *
     * Since refs is filled while going backwards over the subset, iteration is
     * done via a reverse_iterator.
     */
    std::vector<unsigned>::const_reverse_iterator iter;

    /**
     * Anchor point of the first bitmap found since the last reset().
     *
     * From the specs it looks like bitmaps refer to all data that precedes the
     * C operator that defines or uses them, but from the data samples that we
     * have it look like when multiple bitmaps are present, they always refer
     * to the same set of variables.
     *
     * For this reason we remember the first anchor point that we see and
     * always refer the other bitmaps that we see to it.
     */
    unsigned old_anchor = 0;

    Bitmap();
    Bitmap(const Bitmap&) = delete;
    ~Bitmap();
    Bitmap& operator=(const Bitmap&) = delete;

    /**
     * Resets the object. To be called at start of decoding, to discard all
     * previous leftover context, if any.
     */
    void reset();

    /**
     * Initialise the bitmap handler
     *
     * @param bitmap
     *   The bitmap
     * @param subset
     *   The subset to which the bitmap refers
     * @param anchor
     *   The index to the first element after the end of the bitmap (usually
     *   the C operator that defines or uses the bitmap)
     */
    void init(const Var& bitmap, const Subset& subset, unsigned anchor);

    /**
     * True if there is no bitmap or if the bitmap has been iterated until the
     * end
     */
    bool eob() const;

    /**
     * Return the next variable offset for which the bitmap reports that data
     * is present
     */
    unsigned next();
};

}
}
#endif
