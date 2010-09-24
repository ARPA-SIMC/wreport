/*
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#ifndef WREPORT_DTABLE_H
#define WREPORT_DTABLE_H

#include <wreport/opcode.h>
#include <string>
#include <vector>

namespace wreport {

/** @file
 * @ingroup bufrex
 * Implement fast access to information about WMO expansion tables D.
 */

namespace dtable {
/**
 * D-table entry
 */
struct Entry
{
	/// Varcode to be expanded
	Varcode code;
	/// Position in the main table where the expansion begins
	unsigned begin;
	/// Position in the main table one past where the expansion ends
	unsigned end;

	Entry(Varcode code, unsigned begin, unsigned end)
		: code(code), begin(begin), end(end) {}
};
}

/**
 * D-table with Dxxyyy aggregate code expansions
 */
struct DTable
{
protected:
	/// Table ID
	std::string m_id;

public:
	/**
	 * One single table with the concatenation of all the expansion
	 * varcodes
	 */
	std::vector<Varcode> varcodes;

	/**
	 * Expansion entries with pointers inside \a varcodes
	 */
	std::vector<dtable::Entry> entries;

	DTable();
	~DTable();

	/// Table ID
	const std::string& id() const throw () { return m_id; }

	/// True if the table has been loaded
	bool loaded() const throw () { return !m_id.empty(); }

	/**
	 * Load a table
	 *
	 * @param idfile
	 *   pair of (table id, pathname of the table on disk)
	 */
	void load(const std::pair<std::string, std::string>& idfile);

	/**
	 * Query the DTable
	 *
	 * @param var
	 *   entry code (i.e. DXXYYY as a wreport::Varcode WR_VAR(3, xx, yyy).
	 * @return
	 *   the bufrex_opcode chain that contains the expansion elements
	 *   (must be deallocated by the caller using bufrex_opcode_delete)
	 */
	Opcodes query(Varcode var) const;

	/**
	 * Return a DTable by id, loading it if necessary
	 *
	 * Once loaded, the table will be cached in memory for reuse, and
	 * further calls to get() will return the cached version.
	 *
	 * The cached tables are never deallocated, so the returned pointer is
	 * valid through the whole lifetime of the program.
	 *
	 * @param id
	 *   ID of the DTable data to access
	 */
	static const DTable* get(const char* id);

	/**
	 * Same as get(), but explicitly specifies the pathname.
	 *
	 * @param idfile
	 *   pair of (table id, pathname of the table on disk)
	 */
	static const DTable* get(const std::pair<std::string, std::string>& idfile);
};

}

#endif
/* vim:set ts=4 sw=4: */
