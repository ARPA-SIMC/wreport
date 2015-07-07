#ifndef WREPORT_BULLETIN_BITMAPS_H
#define WREPORT_BULLETIN_BITMAPS_H

#include <wreport/var.h>
#include <vector>

namespace wreport {
struct Var;
struct Subset;

namespace bulletin {

/// Associate a Data Present Bitmap to decoded variables in a subset
struct Bitmap
{
    /// Bitmap being iterated
    Var bitmap;

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
     * Create a new bitmap
     *
     * @param bitmap
     *   The bitmap variable
     * @param subset
     *   The subset to which the bitmap refers
     * @param anchor
     *   The index to the first element after the end of the bitmap (usually
     *   the C operator that defines or uses the bitmap)
     */
    Bitmap(const Var& bitmap, const Subset& subset, unsigned anchor);
    Bitmap(const Bitmap&) = delete;
    ~Bitmap();
    Bitmap& operator=(const Bitmap&) = delete;

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

struct Bitmaps
{
    /// Nonzero if a Data Present Bitmap is expected
    Varcode pending_definitions = 0;

    /// Currently active bitmap
    Bitmap* current = nullptr;

    Bitmaps() {}
    Bitmaps(const Bitmaps&) = delete;
    ~Bitmaps();
    Bitmaps& operator=(const Bitmaps&) = delete;

    void define(const Var& bitmap, const Subset& subset, unsigned anchor);

    /**
     * Return the next variable offset for which the bitmap reports that data
     * is present
     */
    unsigned next();

    /// Return true if there is an active bitmap
    bool active() const { return (bool)current; }
};

}
}
#endif
