#ifndef WREPORT_INTERNALS_VARTABLE_H
#define WREPORT_INTERNALS_VARTABLE_H

#include <filesystem>
#include <string>
#include <wreport/fwd.h>
#include <wreport/varinfo.h>
#include <wreport/vartable.h>

namespace wreport::vartable {

struct Entry
{
    /**
     * Master Varinfo structure for this entry.
     *
     * A point to this will be given out and shared by all the code that needs
     * to refer to informations about this variable.
     */
    _Varinfo varinfo;

    /**
     * Altered versions of this Varinfo.
     *
     * BUFR messages can trasmit variables encoded with variations of standard
     * BUFR/CREX B table entries, by overriding reference codes or bit lengths.
     *
     * Altered versions of a Varinfo are stored in this chain. The first
     * element of the chain is always the original Varinfo defined in the B
     * table.
     */
    mutable Entry* alterations = nullptr;

    Entry() = default;

    /**
     * Build an altered entry created for BUFR table C modifiers
     */
    Entry(const Entry& other, int new_scale, unsigned new_bit_len,
          int new_bit_ref);

    /**
     * Search for this alteration in the alteration chain.
     *
     * Returns nullptr if it was not found
     */
    const Entry* get_alteration(int new_scale, unsigned new_bit_len,
                                int new_bit_ref) const;
};

/// Base Vartable implementation
class Base : public Vartable
{
protected:
    /// Pathname to the file from which this vartable has been loaded
    std::filesystem::path m_pathname;

public:
    /**
     * Entries in this Vartable.
     *
     * The entries are sorted by varcode, so that we can look them up by binary
     * search.
     *
     * Since we are handing out pointers to _Varinfo structures inside the
     * vector, those pointers will be invalidated if a vector reallocation gets
     * triggered. This means that once the table has been loaded, it size cannot
     * be changed anymore.
     */
    std::vector<Entry> entries;

    explicit Base(const std::filesystem::path& pathname);

    std::string pathname() const override { return m_pathname; }

    std::filesystem::path path() const override { return m_pathname; }

    _Varinfo* obtain(unsigned line_no, Varcode code);
    const Entry* query_entry(Varcode code) const;
    Varinfo query(Varcode code) const override;
    bool contains(Varcode code) const override;
    Varinfo query_altered(Varcode code, int new_scale, unsigned new_bit_len,
                          int new_bit_ref) const override;
    bool iterate(std::function<bool(Varinfo)> dest) const override;
};

struct Bufr : public Base
{
    /// Create and load a BUFR B table
    explicit Bufr(const std::filesystem::path& pathname);
};

struct Crex : public Base
{
    /// Create and load a CREX B table
    explicit Crex(const std::filesystem::path& pathname);
};

} // namespace wreport::vartable

#endif
