/*
 * wreport/subset - Data subset for BUFR and CREX messages
 *
 * Copyright (C) 2005--2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#ifndef WREPORT_SUBSET_H
#define WREPORT_SUBSET_H

/** @file
 * @ingroup bufrex
 * Handling of a BUFR/CREX data subset as a list of decoded variables.
 */

#include <wreport/var.h>
#include <wreport/vartable.h>
#include <vector>

namespace wreport {
struct Tables;

/**
 * Represent a BUFR/CREX data subset as a list of decoded variables
 */
struct Subset : public std::vector<Var>
{
    // Tables used for creating variables in this subset
    const Tables* tables;

    /**
     * Create a new BUFR/CREX subset.
     *
     * @param btable
     *   Reference to the B table to use to create variables.
     */
    Subset(const Tables& tables);
    Subset(const Subset& subset) = default;
    Subset(Subset&& subset)
        : std::vector<Var>(move(subset)), tables(subset.tables)
    {
    }
    ~Subset();
    Subset& operator=(const Subset&) = default;
    Subset& operator=(Subset&& s)
    {
        if (this == &s) return *this;
        std::vector<Var>::operator=(s);
        tables = s.tables;
        return *this;
    }

	/// Store a decoded variable in the message, to be encoded later.
	void store_variable(const Var& var);

	/**
	 * Store a new variable in the message, copying it from an already existing
	 * variable.
	 *
	 * @param code
	 *   The Varcode of the variable to add.  See @ref varinfo.h
	 * @param var
	 *   The variable holding the value for the variable to add.
	 */
	void store_variable(Varcode code, const Var& var);

	/**
	 * Store a new variable in the message, providing its value as an int
	 *
	 * @param code
	 *   The Varcode of the variable to add.  See @ref vartable.h
	 * @param val
	 *   The value for the variable
	 */
	void store_variable_i(Varcode code, int val);

	/**
	 * Store a new variable in the message, providing its value as a double
	 *
	 * @param code
	 *   The Varcode of the variable to add.  See @ref vartable.h
	 * @param val
	 *   The value for the variable
	 */
	void store_variable_d(Varcode code, double val);

	/**
	 * Store a new variable in the message, providing its value as a string
	 *
	 * @param code
	 *   The Varcode of the variable to add.  See @ref vartable.h
	 * @param val
	 *   The value for the variable
	 */
	void store_variable_c(Varcode code, const char* val);

	/**
	 * Store a new, undefined variable in the message
	 *
	 * @param code
	 *   The Varcode of the variable to add.  See @ref vartable.h
	 */
	void store_variable_undef(Varcode code);

	/**
	 * Compute and append a data present bitmap
	 *
	 * @param ccode
	 *   The C code that uses this bitmap
	 * @param size
	 *   The size of the bitmap
	 * @param attr
	 *   The code of the attribute that the bitmap will represent.  See @ref vartable.h
	 * @return
	 *   The number of attributes that will be encoded (for which the dpb has '+')
	 */
	int append_dpb(Varcode ccode, unsigned size, Varcode attr);

	/**
	 * Append a fixed-size data present bitmap with all zeros
	 *
	 * @param ccode
	 *   The C code that uses this bitmap
	 * @param size
	 *   The size of the bitmap
	 */
	void append_fixed_dpb(Varcode ccode, int size);

	/// Dump the contents of this subset
	void print(FILE* out) const;

    /**
     * Compute the differences between two wreport subsets
     *
     * Details of the differences found will be formatted using the notes
     * system (@see notes.h).
     *
     * @param s2
     *   The subset to compare with this one
     * @returns
     *   The number of differences found
     */
    unsigned diff(const Subset& s2) const;

protected:
	/// Append a C operator with a \a count long bitmap
	void append_c_with_dpb(Varcode ccode, int count, const char* bitmap);
};

}

/* vim:set ts=4 sw=4: */
#endif
