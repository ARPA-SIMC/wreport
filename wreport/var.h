/*
 * wreport/var - Store a value and its informations
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

#ifndef WREPORT_VAR_H
#define WREPORT_VAR_H

/** @file
 * @ingroup core
 * Implement wreport::Var, an encapsulation of a measured variable.
 */


#include <wreport/error.h>
#include <wreport/varinfo.h>
#include <cstdio>
#include <string>
#include <memory>

struct lua_State;

namespace wreport {

/**
 * Holds a wreport variable
 *
 * A wreport::Var contains:
 * \li a wreport::Varcode identifying what is measured.  See @ref vartable.h
 * \li a measured value, that can be an integer, double or string depending on
 *     the wreport::Varcode
 * \li zero or more attributes, in turn represented by wreport::Var structures
 */
class Var
{
protected:
	/// Metadata about the variable
	Varinfo m_info;

	/// Value of the variable
	char* m_value;

	/// Attribute list (ordered by Varcode)
	Var* m_attrs;

public:
#if 0
	/// Create a new Var, from the local B table, with undefined value
	Var(Varcode code);

	/// Create a new Var, from the local B table, with integer value
	Var(Varcode code, int val);

	/// Create a new Var, from the local B table, with double value
	Var(Varcode code, double val);

	/// Create a new Var, from the local B table, with string value
	Var(Varcode code, const char* val);
#endif

	/// Create a new Var, with undefined value
	Var(Varinfo info);

	/// Create a new Var, with integer value
	Var(Varinfo info, int val);

	/// Create a new Var, with double value
	Var(Varinfo info, double val);

	/// Create a new Var, with character value
	Var(Varinfo info, const char* val);

	/// Copy constructor
	Var(const Var& var);

    /// Copy constructor
    Var(const Var& var, bool with_attrs);

	/**
	 * Create a new Var with the value from another one
	 *
	 * Conversions are applied if necessary
	 *
	 * @param info
	 *   The wreport::Varinfo describing the variable to create
	 * @param var
	 *   The variable with the value to use
	 */
	Var(Varinfo info, const Var& var);

	~Var();
	
	/// Assignment
	Var& operator=(const Var& var);

	/// Equality
	bool operator==(const Var& var) const;

	/// Equality
	bool operator!=(const Var& var) const { return !operator==(var); }

	/// Retrieve the Varcode for a variable
	Varcode code() const throw ();

	/// Get informations about the variable
	Varinfo info() const throw ();

	/// Retrieve the internal string representation of the value for a variable.
	const char* value() const throw ();

	/// @returns true if the variable is defined, else false
	bool isset() const throw ();

	/// Get the value as an integer
	int enqi() const;

	/// Get the value as a double
	double enqd() const;

	/// Get the value as a string
	const char* enqc() const;

	/// Templated version of enq
	template<typename T>
	T enq() const
	{
		throw error_unimplemented("getting value of unsupported type");
	}

	/**
	 * Return the variable value, or the given default value if the variable is
	 * not set
	 */
	template<typename T>
	T enq(T default_value) const
	{
		if (!isset()) return default_value;
		return enq<T>();
	}

	/// Set the value from an integer value
	void seti(int val);

	/// Set the value from a double value
	void setd(double val);

	/// Set the value from a string value
	void setc(const char* val);

    /**
     * Set the raw, binary value from a string value.
     *
     * This is similar to setc(), but it always copies as many bytes as the
     * variable is long, including null bytes.
     */
    void set_binary(const unsigned char* val);

    /**
     * Set the value from a string value, truncating \a val if it is too long
     *
     * If a value is truncated, the last character is set to '>' to mark the
     * truncation.
     */
    void setc_truncate(const char* val);

    /// Set from a value formatted with the format() method
    void set_from_formatted(const char* val);

	/**
	 * Shortcuts (use with care, as the semanthics are slightly different
	 * depending on the type)
	 * @{
	 */
	void set(int val) { seti(val); }
	void set(double val) { setd(val); }
	void set(const char* val) { setc(val); }
	void set(const std::string& val) { setc(val.c_str()); }
	void set(const Var& var) { copy_val(var); }
	/// @}

	/// Unset the value
	void unset();

	/// Remove all attributes
	void clear_attrs();

	/**
	 * Query variable attributes
	 *
	 * @param code
	 *   The wreport::Varcode of the attribute requested.  See @ref vartable.h
	 * @returns attr
	 *   A pointer to the attribute if it exists, else NULL.  The pointer points to
	 *   the internal representation and must not be deallocated by the caller.
	 */
	const Var* enqa(Varcode code) const;

    /**
     * Query variable attribute according to significance given in CODE TABLE
     * 031021
     */
    const Var* enqa_by_associated_field_significance(unsigned significance) const;

	/**
	 * Set an attribute of the variable.  An existing attribute with the same
	 * wreport::Varcode will be replaced.
	 *
	 * @param attr
	 *   The attribute to add.  It will be copied inside var, and memory management
	 *   will still be in charge of the caller.
	 */
	void seta(const Var& attr);

	/**
	 * Set an attribute of the variable.  An existing attribute with the same
	 * wreport::Varcode will be replaced.
	 *
	 * @param attr
	 *   The attribute to add.  It will be used directly, and var will take care of
	 *   its memory management.
	 */
	void seta(std::auto_ptr<Var> attr);

	/// Remove the attribute with the given code
	void unseta(Varcode code);

	/**
	 * Get the next attribute in the attribute list
	 *
	 * Example attribute iteration:
	 *
	 * for (const Var* a = var.next_attr(); a != NULL; a = a->next_attr())
	 * 	// Do something with a
	 */
	const Var* next_attr() const;

	/**
	 * Set the value from another variable, performing conversions if
	 * needed.
	 *
	 * The attributes of \a src will also be copied
	 */
	void copy_val(const Var& src);

    /**
     * Set the value from another variable, performing conversions if
     * needed.
     *
     * The attributes of \a src will NOT be copied
     */
    void copy_val_only(const Var& src);

	/**
	 * Copy all the attributes from another variable
	 *
	 * @param src
	 *   The variable with the attributes to copy.
	 */
	void copy_attrs(const Var& src);

    /**
     * Copy all the attributes from another variable, unless they are set to an
     * undefined value
     *
     * @param src
     *   The variable with the attributes to copy.
     */
    void copy_attrs_if_defined(const Var& src);

	/**
	 * Create a formatted string representation of the variable value
	 *
	 * @param ifundef
	 *   String to use if the variable is undefiend
	 */
	std::string format(const char* ifundef = "(undef)") const;

	/**
	 * Print the variable to an output stream
	 *
	 * @param out
	 *   The output stream to use for printing
	 */
	void print(FILE* out) const;

    /**
     * Print the variable to an output stream
     *
     * @param out
     *   The output stream to use for printing
     */
    void print(std::ostream& out) const;

	/**
	 * Print the variable to an output stream, without its attributes
	 *
	 * @param out
	 *   The output stream to use for printing
	 */
	void print_without_attrs(FILE* out) const;

    /**
     * Print the variable to an output stream, without its attributes
     *
     * @param out
     *   The output stream to use for printing
     */
    void print_without_attrs(std::ostream& out) const;

    /**
     * Compare two Var and return the number of differences.
     *
     * Details of the differences found will be formatted using the notes
     * system (@see notes.h).
     *
     * @param var
     *   The variable to compare with this one
     * @returns
     *   The number of differences found and reported
     */
    unsigned diff(const Var& var) const;


	/**
	 * Push the variable as an object in the lua stack
	 */
	void lua_push(struct lua_State* L);

	/**
	 * Check that the element at \a idx is a Var
	 *
	 * @return the Var element, or NULL if the check failed
	 */
	static Var* lua_check(struct lua_State* L, int idx);
};

template<> inline int Var::enq() const { return enqi(); }
template<> inline float Var::enq() const { return (float)enqd(); }
template<> inline double Var::enq() const { return enqd(); }
template<> inline const char* Var::enq() const { return enqc(); }
template<> inline std::string Var::enq() const { return enqc(); }


}

#endif
/* vim:set ts=4 sw=4: */
