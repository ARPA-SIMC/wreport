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

#ifndef WREPORT_OPCODE_H
#define WREPORT_OPCODE_H

/** @file
 * @ingroup wreport
 * Implementation of opcode chains, that are used to drive the encoding and
 * decoding process.
 */

#include <wreport/varinfo.h>
#include <vector>

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
	/// Reference to the vector with all the expanded varcodes
	const std::vector<Varcode>& vals;
	/// First element of the varcode sequence in Opcodes::vals
	unsigned begin;
	/// One-past-the-last element of the varcode sequence in Opcodes::vals
	unsigned end;

	/// Sequence spanning the whole vector
	Opcodes(const std::vector<Varcode>& vals) : vals(vals), begin(0), end(vals.size()) {}
	/// Sequence from begin (inclusive) to end (excluded)
	Opcodes(const std::vector<Varcode>& vals, unsigned begin, unsigned end)
		: vals(vals), begin(begin), end(end) {}
	/// Copy constructor
	Opcodes(const Opcodes& o) : vals(o.vals), begin(o.begin), end(o.end) {}

	/**
	 * Assignment only works if the Opcodes share the same vector.
	 *
	 * @warning: for efficiency reasons, we do not check for it
	 */
	Opcodes& operator=(const Opcodes& o)
	{
		begin = o.begin;
		end = o.end;
		return *this;
	}

	/// Return the i-th varcode in the chain
	Varcode operator[](unsigned i) const
	{
		if (begin + i > end)
			return 0;
		else
			return vals[begin + i];
	}

	/// Number of items in this opcode list
	unsigned size() const { return end - begin; }

	/// True if there are no opcodes
	bool empty() const { return begin == end; }

	/// First opcode in the list (0 if the list is empty)
	Varcode head() const
	{
		if (begin == end)
			return 0;
		return vals[begin];
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
			return Opcodes(vals, begin+1, end);
	}

	/// Return the opcodes from \a skip until the end
	Opcodes sub(unsigned skip) const
	{
		if (begin + skip > end)
			return Opcodes(vals, end, end);
		else
			return Opcodes(vals, begin + skip, end);
	}

	/// Return \a len opcodes starting from \a skip
	Opcodes sub(unsigned skip, unsigned len) const
	{
		if (begin + skip > end)
			return Opcodes(vals, end, end);
		else if (begin + skip + len > end)
			return Opcodes(vals, begin + skip, end);
		else
			return Opcodes(vals, begin + skip, begin + skip + len);
	}

	/**
	 * Print the contents of this opcode list
	 *
	 * This function is used mainly for debugging purposes.
	 *
	 * @param outstream
	 *   The output stream (a FILE* variable) to print to.  void* is used to avoid
	 *   including stdio.h just for this debugging function.
	 */
	void print(void* outstream) const;
};

}

#endif
