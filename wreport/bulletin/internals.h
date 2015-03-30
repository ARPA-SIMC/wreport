/*
 * wreport/bulletin/internals - Bulletin implementation helpers
 *
 * Copyright (C) 2005--2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#ifndef WREPORT_BULLETIN_INTERNALS_H
#define WREPORT_BULLETIN_INTERNALS_H

#include <wreport/varinfo.h>
#include <wreport/opcode.h>
#include <vector>
#include <memory>

namespace wreport {
struct Var;
struct Subset;
struct Bulletin;

namespace bulletin {

/**
 * Associate a Data Present Bitmap to decoded variables in a subset
 */
struct Bitmap
{
    /// Bitmap being iterated
    const Var* bitmap;

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
    unsigned old_anchor;

    Bitmap();
    ~Bitmap();

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

struct AssociatedField
{
    /// B table used to generate associated field attributes
    const Vartable* btable;

    /**
     * If true, fields with a missing values will be returned as 0. If it is
     * false, fields with missing values will be returned as undefined
     * variables.
     */
    bool skip_missing;

    /**
     * Number of extra bits inserted by the current C04yyy modifier (0 for no
     * C04yyy operator in use)
     */
    unsigned bit_count;

    /// Significance of C04yyy field according to code table B31021
    unsigned significance;

    AssociatedField();
    ~AssociatedField();

    /**
     * Resets the object. To be called at start of decoding, to discard all
     * previous leftover context, if any.
     */
    void reset(const Vartable& btable);

    /**
     * Create a Var that can be used as an attribute for the currently defined
     * associated field and the given value.
     *
     * A return value of nullptr means "no field to associate".
     *
     */
    std::auto_ptr<Var> make_attribute(unsigned value) const;

    /**
     * Get the attribute of var corresponding to this associated field
     * significance.
     */
    const Var* get_attribute(const Var& var) const;
};

/**
 * Abstract interface for classes that can be used as targets for the Bulletin
 * Data Descriptor Section interpreters.
 */
struct Visitor : public opcode::Visitor
{
    /// B table used to resolve variable information
    const Vartable* btable;

    /// Current subset (used to refer to past variables)
    const Subset* current_subset;

    /// Bitmap iteration
    Bitmap bitmap;

    /// Current associated field state
    AssociatedField associated_field;

    /// Current value of scale change from C modifier
    int c_scale_change;

    /// Current value of width change from C modifier
    int c_width_change;

    /**
     * Current value of string length override from C08 modifiers (0 for no
     * override)
     */
    int c_string_len_override;

    /// Increase of scale, reference value and data width
    int c_scale_ref_width_increase;

    /// Nonzero if a Data Present Bitmap is expected
    Varcode want_bitmap;

    /**
     * Number of data items processed so far.
     *
     * This is used to generate reference to past decoded data, used when
     * associating attributes to variables.
     */
    unsigned data_pos;


    Visitor();
    virtual ~Visitor();

    /**
     * Return the Varinfo describing the variable \a code, possibly altered
     * taking into account current C modifiers
     */
    Varinfo get_varinfo(Varcode code);

    /// Notify the start of a subset
    virtual void do_start_subset(unsigned subset_no, const Subset& current_subset);

    /**
     * Notify the beginning of one instance of an R group
     *
     * @param idx
     *  The repetition sequence number (0 at the first iteration, 1 at the
     *  second, and so on)
     */
    virtual void do_start_repetition(unsigned idx);

    /**
     * Request processing, according to \a info, of the attribute \a attr_code
     * of the variable in position \a var_pos in the current subset.
     */
    virtual void do_attr(Varinfo info, unsigned var_pos, Varcode attr_code) = 0;

    /**
     * Request processing, according to \a info, of a data variable.
     *
     * associated_field should be consulted to see if there are also associated
     * fields that need processing.
     */
    virtual void do_var(Varinfo info) = 0;

    /**
     * Request processing, according to \a info, of a data variabile that is
     * significant for controlling the encoding process.
     *
     * This means that the variable has always the same value on all datasets
     * (in case of compressed datasets), and that the interpreter needs to know
     * its value.
     *
     * @returns a copy of the variable
     */
    virtual const Var& do_semantic_var(Varinfo info) = 0;

    /**
     * Request processing of a data present bitmap.
     *
     * @param code
     *   The C modifier code that defines the bitmap
     * @param rep_code
     *   The R replicator that defines the bitmap
     * @param delayed_code
     *   The B delayed replicator that defines the bitmap length (it is 0 if
     *   the length is encoded in the YYY part of rep_code
     * @param ops
     *   The replicated opcodes that define the bitmap
     * @returns
     *   The bitmap that has been processed.
     */
    virtual const Var& do_bitmap(Varcode code, Varcode rep_code, Varcode delayed_code, const Opcodes& ops) = 0;

    /**
     * Request processing of C05yyy character data
     */
    virtual void do_char_data(Varcode code) = 0;

    //@{
    /// opcode::Visitor methods implementation
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
    virtual void c_increase_scale_ref_width(Varcode code, int change);
    //@}
};

/**
 * Common bulletin::Visitor base for visitors that modify the bulletin.
 *
 * This assumes a fully decoded bulletin.
 */
struct BaseVisitor : public Visitor
{
    /// Bulletin being visited
    Bulletin& bulletin;
    /// Index of the subset being visited
    unsigned current_subset_no;
    /// Index of the next variable to be visited
    unsigned current_var;

    /// Create visitor for the given bulletin
    BaseVisitor(Bulletin& bulletin);

    /// Get the next variable
    Var& get_var();
    /// Get the variable at the given position
    Var& get_var(unsigned var_pos) const;

    virtual void do_start_subset(unsigned subset_no, const Subset& current_subset);
    virtual const Var& do_bitmap(Varcode code, Varcode rep_code, Varcode delayed_code, const Opcodes& ops);
};

/**
 * Common bulletin::Visitor base for visitors that do not modify the bulletin.
 *
 * This assumes a fully decoded bulletin.
 */
struct ConstBaseVisitor : public Visitor
{
    /// Bulletin being visited
    const Bulletin& bulletin;
    /// Index of the subset being visited
    unsigned current_subset_no;
    /// Index of the next variable to be visited
    unsigned current_var;

    /// Create visitor for the given bulletin
    ConstBaseVisitor(const Bulletin& bulletin);

    /// Get the next variable
    const Var& get_var();
    /// Get the variable at the given position
    const Var& get_var(unsigned var_pos) const;

    virtual void do_start_subset(unsigned subset_no, const Subset& current_subset);
    virtual const Var& do_bitmap(Varcode code, Varcode rep_code, Varcode delayed_code, const Opcodes& ops);
};

}
}

#endif
