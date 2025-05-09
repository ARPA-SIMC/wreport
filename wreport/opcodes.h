#ifndef WREPORT_OPCODE_H
#define WREPORT_OPCODE_H

#include <cstdio>
#include <vector>
#include <wreport/error.h>
#include <wreport/varinfo.h>

namespace wreport {

/**
 * Sequence of opcodes, as a slice of a Varcode vector.
 *
 * This is used for BUFR and CREX encoding and decoding.
 *
 * It can be considered as a sort of subroutine to be interpreted by the
 * encoders/decoders.
 */
struct Opcodes
{
    /// First element of the varcode sequence
    const Varcode* begin;
    /// One-past-the-last element of the varcode sequence
    const Varcode* end;

    /// Sequence spanning the whole vector
    Opcodes(const std::vector<Varcode>& vals)
        : begin(vals.data()), end(begin + vals.size())
    {
    }
    /// Sequence from begin (inclusive) to end (excluded)
    Opcodes(const Varcode* begin, const Varcode* end) : begin(begin), end(end)
    {
    }

    Opcodes(const Opcodes& o)            = default;
    Opcodes& operator=(const Opcodes& o) = default;

    /// Return the i-th varcode in the chain
    Varcode operator[](unsigned i) const
    {
        if (begin + i > end)
            return 0;
        else
            return begin[i];
    }

    /// Number of items in this opcode list
    unsigned size() const { return static_cast<unsigned>(end - begin); }

    /// True if there are no opcodes
    bool empty() const { return begin == end; }

    /**
     * Return the first element and advance begin to the next one.
     *
     * If the sequence is empty, throw an exception
     */
    Varcode pop_left()
    {
        if (empty())
            throw error_consistency(
                "cannot do pop_left on an empty opcode sequence");
        return *begin++;
    }

    /**
     * Return the first \a count elements and advance begin to the first opcode
     * after the sequence.
     *
     * If the sequence has less that \a count elements, throw an exception
     */
    Opcodes pop_left(unsigned count)
    {
        if (size() < count)
            error_consistency::throwf("cannot do pop_left(%u) on an opcode "
                                      "sequence of only %u elements",
                                      count, size());
        Opcodes res(begin, begin + count);
        begin += count;
        return res;
    }

    /// First opcode in the list (0 if the list is empty)
    Varcode head() const
    {
        if (begin == end)
            return 0;
        return *begin;
    }

    /**
     * List of all opcodes after the first one
     *
     * If the list is empty, return the empty list
     */
    Opcodes next() const
    {
        if (begin == end)
            return *this;
        else
            return Opcodes(begin + 1, end);
    }

    /// Return the opcodes from \a skip until the end
    Opcodes sub(unsigned skip) const
    {
        if (begin + skip > end)
            return Opcodes(end, end);
        else
            return Opcodes(begin + skip, end);
    }

    /// Return \a len opcodes starting from \a skip
    Opcodes sub(unsigned skip, unsigned len) const
    {
        if (begin + skip > end)
            return Opcodes(end, end);
        else if (begin + skip + len >= end)
            return Opcodes(begin + skip, end);
        else
            return Opcodes(begin + skip, begin + skip + len);
    }

    /// Print the contents of this opcode list
    void print(FILE* out) const;
};

} // namespace wreport
#endif
