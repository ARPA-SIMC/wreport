/*
 * wreport/bulletin - Decoded weather bulletin
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

#ifndef WREPORT_BULLETIN_H
#define WREPORT_BULLETIN_H

/** @file
 * @ingroup bufrex
 * Intermediate representation of parsed values, ordered according to a BUFR or
 * CREX message template.
 */

#include <wreport/var.h>
#include <wreport/subset.h>
#include <vector>
#include <memory>

namespace wreport {

namespace bulletin {
struct Visitor;
struct BufrInput;
}

struct DTable;

/**
 * Storage for the decoded data of a BUFR or CREX message.
 */
struct Bulletin
{
	/** Message category */
	int type;
	/** International message subcategory */
	int subtype;
	/** Local message subcategory */
	int localsubtype;

	/** Edition number */
	int edition;

    /** Master table number */
    int master_table_number;

	/** Representative datetime for this data
	 * @{ */
	int rep_year;	/**< Year */
	int rep_month;	/**< Month */
	int rep_day;	/**< Day */
	int rep_hour;	/**< Hour */
	int rep_minute;	/**< Minute */
	int rep_second;	/**< Second */
	/** @} */

	/** vartable used to lookup B table codes */
	const Vartable* btable;
	/** dtable used to lookup D table codes */
	const DTable* dtable;

	/** Parsed data descriptor section */
	std::vector<Varcode> datadesc;

	/** Decoded variables */
	std::vector<Subset> subsets;

	Bulletin();
	virtual ~Bulletin();

	/// Reset the bulletin
	virtual void clear();

	/** Type of source/target encoding */
	virtual const char* encoding_name() const throw () = 0;

	/**
	 * Get a Subset from the message.
	 *
	 * The subset will be created if it does not exist, and it will be
	 * memory managed by the Bulletin.
	 *
	 * @param subsection
	 *   The subsection index (starting from 0)
	 */
	Subset& obtain_subset(unsigned subsection);

	/**
	 * Get a Subset from the message.
	 *
	 * An exception will be thrown if the subset does not exist
	 *
	 * @param subsection
	 *   The subsection index (starting from 0)
	 */
	const Subset& subset(unsigned subsection) const;

	/// Load a new set of tables to use for encoding this message
	virtual void load_tables() = 0;

	/**
	 * Parse only the header of an encoded message
	 *
	 * @param buf
	 *   The buffer to decode
	 * @param fname
	 *   The file name to use for error messages
	 * @param offset
	 *   The offset inside the file of the start of the bulletin, used for
	 *   error messages
	 */
	virtual void decode_header(const std::string& buf, const char* fname="(memory)", size_t offset=0) = 0;

	/**
	 * Parse an encoded message
	 *
	 * @param buf
	 *   The buffer to decode
	 * @param fname
	 *   The file name to use for error messages
	 * @param offset
	 *   The offset inside the file of the start of the bulletin, used for
	 *   error messages
	 */
	virtual void decode(const std::string& buf, const char* fname="(memory)", size_t offset=0) = 0;

	/**
	 * Encode the message
	 */
	virtual void encode(std::string& buf) const = 0;

    /**
     * Walk the structure of the data descriptor section sending events to an
     * opcode::Explorer
     */
    void visit_datadesc(opcode::Visitor& e) const;

    /**
     * Run the Data Descriptor Section interpreter, sending commands to \a
     * executor
     */
    void visit(bulletin::Visitor& out) const;

	/**
	 * Dump the contents of this bulletin
	 */
	void print(FILE* out) const;

    /**
     * Dump the contents of this bulletin, in a more structured way
     */
    void print_structured(FILE* out) const;

    /// Print format-specific details
    virtual void print_details(FILE* out) const;

    /**
     * Pretty-print the data descriptor section
     *
     * @param indent
     *   Indent all output by this amount of spaces
     */
    void print_datadesc(FILE* out, unsigned indent=0) const;

    /**
     * Compute the differences between two bulletins
     *
     * Details of the differences found will be formatted using the notes
     * system (@see notes.h).
     *
     * @param msg
     *   The bulletin to compare with this one
     * @returns
     *   The number of differences found
     */
    virtual unsigned diff(const Bulletin& msg) const;

    /// Diff format-specific details
    virtual unsigned diff_details(const Bulletin& msg) const;
};


/**
 * Options used to configure BUFR decoding
 */
struct BufrCodecOptions
{
    bool decode_adds_undef_attrs;

    BufrCodecOptions()
        : decode_adds_undef_attrs(false)
    {
    }
};

/**
 * BUFR bulletin implementation
 */
struct BufrBulletin : public Bulletin
{
	/** BUFR-specific encoding options */

	/** Common Code table C-1 identifying the originating centre */
	int centre;
	/** Centre-specific subcentre code */
	int subcentre;
	/** Version number of master tables used */
	int master_table;
	/** Version number of local tables used to augment the master table */
	int local_table;

	/** 1 if the BUFR message uses compression, else 0 */
	int compression;
	/** Update sequence number from octet 7 in section 1*/
	int update_sequence_number;
	/** 0 if the BUFR message does not contain an optional section, else
	 *  its length in bytes */
	int optional_section_length;
	/** Raw contents of the optional section */
	char* optional_section;

    /**
     * Raw details about the message that has been decoded.
     *
     * It is only filled in by a decoding operation: in all other cases it is
     * NULL.
     */
    bulletin::BufrInput* raw_details;

    /**
     * Options used to customise encoding or decoding.
     *
     * It is NULL by default, in which case default options are used.
     *
     * To configure it, set it to point to a BufrCodecOptions structure with
     * the parameters you need. The caller is responsible for the memory
     * management of the BufrCodecOptions structure.
     */
    const BufrCodecOptions* codec_options;


	BufrBulletin();
	virtual ~BufrBulletin();

	void clear();
	virtual const char* encoding_name() const throw () { return "BUFR"; }
	virtual void load_tables();
	virtual void decode_header(const std::string& raw, const char* fname="(memory)", size_t offset=0);
	virtual void decode(const std::string& raw, const char* fname="(memory)", size_t offset=0);
	virtual void encode(std::string& buf) const;
	virtual void print_details(FILE* out) const;
    virtual unsigned diff_details(const Bulletin& msg) const;

    /**
     * Create or reset the raw_details structure for this bulletin.
     *
     * This is only invoked during decoding.
     */
    bulletin::BufrInput& reset_raw_details(const std::string& buf);

	/**
	 * Read an encoded BUFR message from a stream
	 *
	 * @param in
	 *   The stream to read from
	 * @param buf
	 *   The buffer where the data will be written
	 * @param fname
	 *   File name to use in error messages
	 * @retval offset
	 *   The offset in the file of the beginning of the BUFR data
	 * @returns
	 *   true if a message was found, false on EOF
	 */
	static bool read(FILE* in, std::string& buf, const char* fname = 0, long* offset = 0);

	/**
	 * Write an encoded BUFR message to a stream
	 *
	 * @param buf
	 *   The buffer with the data to write
	 * @param out
	 *   The stream to write to
	 * @param fname
	 *   File name to use in error messages
	 */
	static void write(const std::string& buf, FILE* out, const char* fname = 0);
};

/**
 * CREX bulletin implementation
 */
struct CrexBulletin : public Bulletin
{
	/** CREX-specific encoding options */

	/** Table version number */
	int table;
	/** True if the CREX message uses the check digit feature */
	int has_check_digit;

	void clear();
	virtual const char* encoding_name() const throw () { return "CREX"; }
	virtual void load_tables();
	virtual void decode_header(const std::string& raw, const char* fname="(memory)", size_t offset=0);
	virtual void decode(const std::string& raw, const char* fname="(memory)", size_t offset=0);
	virtual void encode(std::string& buf) const;
	virtual void print_details(FILE* out) const;
    virtual unsigned diff_details(const Bulletin& msg) const;

	/**
	 * Read an encoded BUFR message from a stream
	 *
	 * @param in
	 *   The stream to read from
	 * @param buf
	 *   The buffer where the data will be written
	 * @param fname
	 *   File name to use in error messages
	 * @retval offset
	 *   The offset in the file of the beginning of the BUFR data
	 * @returns
	 *   true if a message was found, false on EOF
	 */
	static bool read(FILE* in, std::string& buf, const char* fname = 0, long* offset = 0);

	/**
	 * Write an encoded BUFR message to a stream
	 *
	 * @param buf
	 *   The buffer with the data to write
	 * @param out
	 *   The stream to write to
	 * @param fname
	 *   File name to use in error messages
	 */
	static void write(const std::string& buf, FILE* out, const char* fname = 0);
};


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

/* vim:set ts=4 sw=4: */
#endif
