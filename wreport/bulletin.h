/*
 * wreport/bulletin - Archive for punctual meteorological data
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
struct DDSExecutor;
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
     * Run the Data Descriptor Section interpreter, sending commands to \a
     * executor
     */
    void run_dds(bulletin::DDSExecutor& out) const;

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

	/** Master table (00 for standard WMO FM95 CREX tables) */
	int master_table;
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
 * Abstract interface for classes that can be used as targets for the Bulletin
 * Data Descriptor Section interpreters.
 */
struct DDSExecutor
{
    virtual ~DDSExecutor();

    /// Notify the start of a subset
    virtual void start_subset(unsigned subset_no) = 0;

    /// Notify that we start decoding a R group
    virtual void push_repetition(unsigned length, unsigned count);

    /// Notify the beginning of one instance of an R group
    virtual void start_repetition();

    /// Notify that we ended decoding a R group
    virtual void pop_repetition();

    /// Notify that we start decoding a D group
    virtual void push_dcode(Varcode code);

    /// Notify that we ended decoding a D group
    virtual void pop_dcode();

    /// Return the size of the current subset
    virtual unsigned subset_size() = 0;

    /**
     * Return true if the variable at \a var_pos is special
     * (WR_VAR_F(varcode) != 0)
     */
    virtual bool is_special_var(unsigned var_pos) = 0;

    /**
     * Request encoding, according to \a info, of attribute \a attr_code of
     * variable in position \a var_pos in the current subset.
     */
    virtual void encode_attr(Varinfo info, unsigned var_pos, Varcode attr_code) = 0;

    /**
     * Request encoding, according to \a info, of variable in position \a
     * var_pos in the current subset.
     */
    virtual void encode_var(Varinfo info, unsigned var_pos) = 0;

    /**
     * Request encoding, according to \a info, of repetition count in position
     * \a var_pos in the current subset.
     *
     * @return the value of the repetition count.
     */
    virtual unsigned encode_repetition_count(Varinfo info, unsigned var_pos) = 0;

    /**
     * Request encoding, according to \a info, of repetition count
     * corresponding to the length of \a bitmap.
     *
     * @return the value of the repetition count.
     */
    virtual unsigned encode_bitmap_repetition_count(Varinfo info, const Var& bitmap) = 0;

    /**
     * Request encoding of \a bitmap
     */
    virtual void encode_bitmap(const Var& bitmap) = 0;

    /**
     * Request encoding of C05yyy character data
     */
    virtual void encode_char_data(Varcode code, unsigned var_pos) = 0;

    /**
     * Get the bitmap at position \a var_pos
     */
    virtual const Var* get_bitmap(unsigned var_pos) = 0;
};

struct BaseDDSExecutor : public DDSExecutor
{
    Bulletin& bulletin;
    Subset* current_subset;
    unsigned current_subset_no;

    BaseDDSExecutor(Bulletin& bulletin);

    const Var& get_var(unsigned var_pos) const;

    virtual void start_subset(unsigned subset_no);
    virtual unsigned subset_size();
    virtual bool is_special_var(unsigned var_pos);
    virtual const Var* get_bitmap(unsigned var_pos);
};

struct ConstBaseDDSExecutor : public DDSExecutor
{
    const Bulletin& bulletin;
    const Subset* current_subset;
    unsigned current_subset_no;

    ConstBaseDDSExecutor(const Bulletin& bulletin);

    const Var& get_var(unsigned var_pos) const;

    virtual void start_subset(unsigned subset_no);
    virtual unsigned subset_size();
    virtual bool is_special_var(unsigned var_pos);
    virtual const Var* get_bitmap(unsigned var_pos);
};

}



}

/* vim:set ts=4 sw=4: */
#endif
