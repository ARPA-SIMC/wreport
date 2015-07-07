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
#include <wreport/bulletin/interpreter.h>
#include <vector>
#include <memory>
#include <cmath>

namespace wreport {
struct Var;
struct Subset;
struct Bulletin;

namespace bulletin {

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
    std::unique_ptr<Var> make_attribute(unsigned value) const;

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
struct Parser : public bulletin::DDSInterpreter
{
    /// Current subset (used to refer to past variables)
    const Subset* current_subset;

    /// Current associated field state
    AssociatedField associated_field;


    Parser(const Tables& tables, const Opcodes& opcodes);
    virtual ~Parser();

    /**
     * Return the Varinfo describing the variable \a code, possibly altered
     * taking into account current C modifiers
     */
    Varinfo get_varinfo(Varcode code);

    /// Notify the start of a subset
    virtual void do_start_subset(unsigned subset_no, const Subset& current_subset);

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
     * Request processing of C05yyy character data
     */
    virtual void do_char_data(Varcode code) = 0;

    //@{
    /// bulletin::Visitor methods implementation
    void b_variable(Varcode code) override;
    void c_associated_field(Varcode code, Varcode sig_code, unsigned nbits) override;
    void c_char_data(Varcode code) override;
    void c_substituted_value(Varcode code) override;
    void c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits) override;
    //@}
};

/**
 * Common bulletin::Parser base for visitors that modify the bulletin.
 *
 * This assumes a fully decoded bulletin.
 */
struct BaseParser : public Parser
{
    /// Bulletin being visited
    Bulletin& bulletin;
    /// Index of the subset being visited
    unsigned current_subset_no;
    /// Index of the next variable to be visited
    unsigned current_var;

    /// Create visitor for the given bulletin
    BaseParser(Bulletin& bulletin);

    /// Get the next variable
    Var& get_var();
    /// Get the variable at the given position
    Var& get_var(unsigned var_pos) const;

    void do_start_subset(unsigned subset_no, const Subset& current_subset) override;
    void define_bitmap(Varcode code, Varcode rep_code, Varcode delayed_code, const Opcodes& ops) override;
};

}
}

#endif
