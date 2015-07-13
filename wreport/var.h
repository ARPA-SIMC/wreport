#ifndef WREPORT_VAR_H
#define WREPORT_VAR_H

#include <wreport/error.h>
#include <wreport/varinfo.h>
#include <cstdio>
#include <string>
#include <memory>

struct lua_State;

namespace wreport {

/**
 * A physical variable
 *
 * A wreport::Var contains:
 * \li a wreport::Varinfo describing the variable
 * \li a value, that can be integer, floating point, string or opaque binary
 *     data as specified by the Varinfo
 * \li zero or more attributes, represented by other wreport::Var objects
 */
class Var
{
protected:
	/// Metadata about the variable
	Varinfo m_info;

    /// True if the variable is set, false otherwise
    bool m_isset;

    /**
     * Value of the variable
     *
     * For numeric values, it is the value encoded to an integer decimal string
     * according to m_info.
     *
     * For string values, it is the 0-terminated string.
     *
     * For binary values, it is a raw buffer where the first m_info->bit_len
     * bits are the binary value, and the rest is set to 0.
     */
    union {
        int32_t i;
        double d;
        char* c;
    } m_value;

	/// Attribute list (ordered by Varcode)
	Var* m_attrs;

    /// Make sure that m_value is allocated. It does nothing if it already is.
    void allocate();

    /// Copy the value from var. var is assumed to have the same varinfo as us.
    void copy_value(const Var& var);
    /// Move the value from var. var is assumed to have the same varinfo as us. var is left unset.
    void move_value(Var& var);
    void assign_i_checked(int32_t val);
    void assign_d_checked(double val);
    void assign_b_checked(uint8_t* val, unsigned size);
    void assign_c_checked(const char* val, unsigned size);

public:
	/// Create a new Var, with undefined value
	Var(Varinfo info);

	/// Create a new Var, with integer value
	Var(Varinfo info, int val);

	/// Create a new Var, with double value
	Var(Varinfo info, double val);

	/// Create a new Var, with character value
	Var(Varinfo info, const char* val);

    /// Create a new Var, with character value
    Var(Varinfo info, const std::string& val);

    /**
     * Create a new Var with the value from another one.
     *
     * Conversions are applied if necessary, attributes are not copied.
     *
     * @param info
     *   The wreport::Varinfo describing the variable to create
     * @param var
     *   The variable with the value to use
     */
    Var(Varinfo info, const Var& var);

	/// Copy constructor
	Var(const Var& var);

    /**
     * Move constructor.
     *
     * After movement, \a var will still a valid variable, but it will be unset
     * and without attributes.
     */
    Var(Var&& var);

    ~Var();

    /// Assignment
    Var& operator=(const Var& var);

    /**
     * Move assignment
     *
     * After movement, \a var will still a valid variable, but it will be unset
     * and without attributes.
     */
    Var& operator=(Var&& var);

    bool operator==(const Var& var) const;
    bool operator!=(const Var& var) const { return !operator==(var); }

    /**
     * Test if the values are the same, regardless of variable codes or
     * attributes
     */
    bool value_equals(const Var& var) const;

    /// Retrieve the Varcode for a variable
    Varcode code() const throw () { return m_info->code; }

    /// Get informations about the variable
    Varinfo info() const throw () { return m_info; }

    /// @returns true if the variable is defined, else false
    bool isset() const throw () { return m_isset; }


	/// Get the value as an integer
	int enqi() const;

	/// Get the value as a double
	double enqd() const;

	/// Get the value as a string
	const char* enqc() const;

    /// Get the value as a std::string
    std::string enqs() const;

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

    /// Set the value from a string or opaque binary value
    void setc(const char* val);

    /// Set the value from a string or opaque binary value
    void sets(const std::string& val);

    /// Set from a value formatted with the format() method
    void setf(const char* val);

    /**
     * Set the value from a string value, truncating it if it is too long.
     *
     * If a value is truncated, the last character is set to '>' to mark the
     * truncation.
     */
    void setc_truncate(const char* val);

    /**
     * Set the value from another variable, performing conversions if
     * needed. The attributes of \a src will be ignored.
     */
    void setval(const Var& src);

    /**
     * Replace all attributes in this variable with all the attributes from \a
     * src
     */
    void setattrs(const Var& src);

	/**
	 * Shortcuts (use with care, as the semanthics are slightly different
	 * depending on the type)
	 * @{
	 */
	void set(int val) { seti(val); }
	void set(double val) { setd(val); }
	void set(const char* val) { setc(val); }
	void set(const std::string& val) { setc(val.c_str()); }
	void set(const Var& var) { setval(var); setattrs(var); }
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
     *   The attribute to add. Its value will be moved inside the destination
     *   attribute, and attr will be unset.
     */
    void seta(Var&& attr);

	/**
	 * Set an attribute of the variable.  An existing attribute with the same
	 * wreport::Varcode will be replaced.
	 *
	 * @param attr
	 *   The attribute to add.  It will be used directly, and var will take care of
	 *   its memory management.
	 */
	void seta(std::unique_ptr<Var>&& attr);

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
    void print_without_attrs(FILE* out, const char* end="\n") const;

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
template<> inline std::string Var::enq() const { return enqs(); }


}

#endif
