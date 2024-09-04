#ifndef WREPORT_DTABLE_H
#define WREPORT_DTABLE_H

#include <wreport/opcodes.h>
#include <filesystem>
#include <string>
#include <vector>

namespace wreport {

/**
 * BUFR/CREX table D with Dxxyyy aggregate code expansions
 */
class DTable
{
public:
    virtual ~DTable();

    /// Return the pathname of the file from which this table has been loaded
    [[deprecated("Use path() instead")]] virtual std::string pathname() const = 0;

    /**
     * Query the DTable
     *
     * @param var
     *   entry code (i.e. DXXYYY as a wreport::Varcode WR_VAR(3, xx, yyy).
     * @return
     *   the bufrex_opcode chain that contains the expansion elements
     *   (must be deallocated by the caller using bufrex_opcode_delete)
     */
    virtual Opcodes query(Varcode var) const = 0;

    /// Return the pathname of the file from which this table has been loaded
    virtual std::filesystem::path path() const = 0;

    /**
     * Return a BUFR D table, by file name.
     *
     * Once loaded, the table will be cached in memory for reuse, and
     * further calls to load_bufr() will return the cached version.
     */
    static const DTable* load_bufr(const std::string& pathname);

    /**
     * Return a CREX D table, by file name.
     *
     * Once loaded, the table will be cached in memory for reuse, and
     * further calls to load_crex() will return the cached version.
     */
    static const DTable* load_crex(const std::string& pathname);
};


}

#endif
