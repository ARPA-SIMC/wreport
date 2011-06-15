/*
 * wreport/bulletin/internals - Bulletin implementation helpers
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

#ifndef WREPORT_BULLETIN_INTERNALS_H
#define WREPORT_BULLETIN_INTERNALS_H

#include <wreport/varinfo.h>
#include <wreport/opcode.h>
#include <vector>

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
    const Var* bitmap;
    std::vector<unsigned> refs;
    std::vector<unsigned>::const_reverse_iterator iter;
    unsigned old_anchor;

    Bitmap();
    ~Bitmap();

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

    bool eob() const;
    unsigned next();
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

    /// Current value of scale change from C modifier
    int c_scale_change;

    /// Current value of width change from C modifier
    int c_width_change;

    /**
     * Current value of string length override from C08 modifiers (0 for no
     * override)
     */
    int c_string_len_override;

    /**
     * Number of extra bits inserted by the current C04yyy modifier (0 for no
     * C04yyy operator in use)
     */
    int c04_bits;

    /// Meaning of C04yyy field according to code table B31021
    int c04_meaning;

    /// True if a Data Present Bitmap is expected
    bool want_bitmap;

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

    /// Notify the beginning of one instance of an R group
    virtual void do_start_repetition(unsigned idx);

    /**
     * Request processing of \a bit_count bits of associated field with the
     * given \a significance
     */
    virtual void do_associated_field(unsigned bit_count, unsigned significance) = 0;

    /**
     * Request processing, according to \a info, of the attribute \a attr_code
     * of the variable in position \a var_pos in the current subset.
     */
    virtual void do_attr(Varinfo info, unsigned var_pos, Varcode attr_code) = 0;

    /**
     * Request processing, according to \a info, of a data variable.
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
    virtual Var do_semantic_var(Varinfo info) = 0;

    /**
     * Request processing of a data present bitmap.
     *
     * Returns a pointer to the bitmap that has been processed.
     */
    virtual const Var* do_bitmap(Varcode code, Varcode delayed_code, const Opcodes& ops) = 0;

    /**
     * Request processing of C05yyy character data
     */
    virtual void do_char_data(Varcode code) = 0;

    // opcode::Visitor method implementation
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
};

struct BaseVisitor : public Visitor
{
    Bulletin& bulletin;
    unsigned current_subset_no;
    unsigned current_var;

    BaseVisitor(Bulletin& bulletin);

    Var& get_var();
    Var& get_var(unsigned var_pos) const;

    virtual void do_start_subset(unsigned subset_no, const Subset& current_subset);
    virtual const Var* do_bitmap(Varcode code, Varcode delayed_code, const Opcodes& ops);
};

struct ConstBaseVisitor : public Visitor
{
    const Bulletin& bulletin;
    unsigned current_subset_no;
    unsigned current_var;

    ConstBaseVisitor(const Bulletin& bulletin);

    const Var& get_var();
    const Var& get_var(unsigned var_pos) const;

    virtual void do_start_subset(unsigned subset_no, const Subset& current_subset);
    virtual const Var* do_bitmap(Varcode code, Varcode delayed_code, const Opcodes& ops);
};

}
}

#endif
