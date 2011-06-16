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
#include <cstdio>

namespace wreport {

namespace opcode {
struct Visitor;
}

struct Vartable;
struct DTable;

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
     * Walk the structure of the opcodes sending events to an opcode::Visitor.
     *
     * Initialise e.dtable with \a dtable.
     */
    void visit(opcode::Visitor& e, const DTable& dtable) const;

    /**
     * Walk the structure of the opcodes sending events to an opcode::Visitor
     *
     * Assume that e.dtable is already initialised.
     */
    void visit(opcode::Visitor& e) const;

    /// Print the contents of this opcode list
    void print(FILE* out) const;
};

namespace opcode
{

/**
 * Visitor-style interface for scanning the contents of a data descriptor
 * section.
 *
 * This supports scanning the DDS without looking at the data, so it cannot be
 * used for encoding/decoding, as it cannot access the data that controls
 * decoding such as delayed replicator factors or data descriptor bitmaps.
 *
 * All interface methods have a default implementations that do nothing, so you
 * can override only what you need.
 */
struct Visitor
{
    /**
     * D table to use to expand D groups.
     *
     * This must be provided by the caller
     */
    const DTable* dtable;

    Visitor();
    virtual ~Visitor();

    /**
     * Notify of a B variable entry
     *
     * @param code
     *   The B variable code
     */
    virtual void b_variable(Varcode code);

    /**
     * Notify of a C modifier
     *
     * Whenever the modifier is a supported one, this is followed by an
     * invocation of one of the specific c_* methods.
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_modifier(Varcode code);

    /**
     * Notify a change of data width
     *
     * @param code
     *   The C modifier code
     * @param change
     *   The width change (positive or negative)
     */
    virtual void c_change_data_width(Varcode code, int change);

    /**
     * Notify a change of data scale
     *
     * @param code
     *   The C modifier code
     * @param change
     *   The scale change (positive or negative)
     */
    virtual void c_change_data_scale(Varcode code, int change);

    /**
     * Notify the declaration of an associated field for the next values.
     *
     * @param code
     *   The C modifier code
     * @param sig_code
     *   The B code of the associated field significance opcode (or 0 to mark
     *   the end of the associated field encoding)
     * @param nbits
     *   The number of bits used for the associated field.
     */
    virtual void c_associated_field(Varcode code, Varcode sig_code, unsigned nbits);

    /**
     * Notify raw character data encoded via a C modifier
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_char_data(Varcode code);

    /**
     * Notify an override of character data length
     *
     * @param code
     *   The C modifier code
     * @param new_length
     *   New length of all following character data (or 0 to reset to default)
     */
    virtual void c_char_data_override(Varcode code, unsigned new_length);

    /**
     * Notify a bitmap for quality information data
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_quality_information_bitmap(Varcode code);

    /**
     * Notify a bitmap for substituted values
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_substituted_value_bitmap(Varcode code);

    /**
     * Notify a substituted value
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_substituted_value(Varcode code);

    /**
     * Notify the length of the following local descriptor
     *
     * @param code
     *   The C modifier code
     * @param desc_code
     *   Local descriptor for which the length is provided
     * @param nbits
     *   Bit size of the data described by \a desc_code
     */
    virtual void c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits);

    /**
     * Notify a replicated section
     * 
     * @param code
     *   The R replication code
     * @param delayed_code
     *   The delayed replication B code, or 0 if delayed replication is not
     *   used
     * @param ops
     *   The replicated operators
     */
    virtual void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops);

    /**
     * Notify the start of a D group
     *
     * @param code
     *   The D code that is being expanded
     */
    virtual void d_group_begin(Varcode code);

    /**
     * Notify the end of a D group
     *
     * @param code
     *   The D code that has just been expanded
     */
    virtual void d_group_end(Varcode code);
};

/**
 * opcode::Visitor that pretty-prints the opcodes using indentation to show
 * structure
 */
class Printer : public Visitor
{
protected:
    /**
     * Print line lead (indentation and formatted code)
     *
     * @param code
     *   Code to format in the line lead
     */
    void print_lead(Varcode code);

public:
    /**
     * Output stream.
     *
     * It defaults to stdout, but it can be set to any FILE* stream
     */
    FILE* out;

    /**
     * Table used to get variable descriptions (optional).
     *
     * It defaults to NULL, but if it is set, the output will contain
     * descriptions of B variable entries
     */
    const Vartable* btable;

    /**
     * Current indent level
     *
     * It defaults to 0 in a newly created Printer. You can set it to some
     * other value to indent all the output by the given amount of spaces
     */
    unsigned indent;

    /// How many spaces in an indentation level
    unsigned indent_step;

    Printer();
    virtual void b_variable(Varcode code);
    virtual void c_modifier(Varcode code);
    virtual void c_change_data_width(Varcode code, int change);
    virtual void c_change_data_scale(Varcode code, int change);
    virtual void c_associated_field(Varcode code, Varcode sig_code, unsigned nbits);
    virtual void c_char_data(Varcode code);
    virtual void c_char_data_override(Varcode code, unsigned new_length);
    virtual void c_quality_information_bitmap(Varcode code);
    virtual void c_substituted_value_bitmap(Varcode code);
    virtual void c_substituted_value(Varcode code);
    virtual void c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits);
    virtual void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops);
    virtual void d_group_begin(Varcode code);
    virtual void d_group_end(Varcode code);
};

}

}

#endif
