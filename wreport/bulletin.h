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
#include <wreport/opcode.h>
#include <wreport/tables.h>
#include <vector>
#include <memory>

/**
 * Main namespace
 */
namespace wreport {
struct DTable;

namespace bulletin {
struct Visitor;
struct Parser;
struct BufrInput;
struct LocalVartable;
}

/**
 * Storage for the decoded data of a BUFR or CREX message.
 *
 * A Bulletin roughly reflects the structure of a BUFR or CREX message: it
 * contains metadata, a sequence of wreport::Varcode with the contents of a
 * Data Descriptor Section, and one or more wreport::Subset with the decoded
 * values.
 *
 * Subsets are essentially sequences of wreport::Var objects, and therefore
 * contain the values together with the full range of variable information,
 * including type, measurement units and number of significant digits.
 *
 * Extra values like quality control statistics or replaced values are
 * represented as 'attributes' to the wreport::Var objects.
 */
struct Bulletin
{
    /**
     * Input file name (optional).
     *
     * If available, it will be used to generate better error messages.
     *
     * If not available, it is NULL.
     */
    const char* fname;

    /**
     * File offset of the start of the message.
     *
     * If available, it will be used to generate better error messages.
     *
     * If not available, it is 0.
     */
    size_t offset;

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

    Tables tables;

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
    virtual void encode(std::string& buf) = 0;

    /**
     * Walk the structure of the data descriptor section sending events to an
     * opcode::Explorer
     */
    void visit_datadesc(bulletin::Visitor& e);

    /**
     * Run the Data Descriptor Section interpreter, sending commands to \a
     * executor
     */
    void visit(bulletin::Parser& out);

    /// Dump the contents of this bulletin
    void print(FILE* out);

    /// Dump the contents of this bulletin, in a more structured way
    void print_structured(FILE* out);

    /// Print format-specific details
    virtual void print_details(FILE* out) const;

    /**
     * Pretty-print the data descriptor section
     *
     * @param out
     *   Output stream to use
     * @param indent
     *   Indent all output by this amount of spaces
     */
    void print_datadesc(FILE* out, unsigned indent=0);

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
 * Options used to configure BUFR decoding.
 */
struct BufrCodecOptions
{
    /**
     * By default (false) undefined attributes are not added to variables, and
     * there is no difference between an undefined or a missing attribute.
     *
     * If this is set to true, undefined attributes are added to variables, so
     * that it is possible to tell between a variable with no attributes and a
     * variable for which the bulletin provides attributes but they have an
     * missing value.
     */
    bool decode_adds_undef_attrs;

    /**
     * Create a BufrCodecOptions
     *
     * Options may be added at any time to future versions of the structure. To
     * reduce the likelyhook of breaking ABI, construction on stack is discouraged
     * in favour of an allocator function.
     */
    static std::unique_ptr<BufrCodecOptions> create();

protected:
    BufrCodecOptions();
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


	virtual ~BufrBulletin();

    void clear();
    const char* encoding_name() const throw () override { return "BUFR"; }
    void load_tables() override;
    void decode_header(const std::string& raw, const char* fname="(memory)", size_t offset=0) override;
    void decode(const std::string& raw, const char* fname="(memory)", size_t offset=0) override;
    void encode(std::string& buf) override;
    void print_details(FILE* out) const override;
    unsigned diff_details(const Bulletin& msg) const override;

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

    /**
     * To prevent breaking ABI if new members are added to bulletins, direct
     * construction is discouraged in favour of an allocator function
     */
    static std::unique_ptr<BufrBulletin> create();

protected:
    BufrBulletin();
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
	bool has_check_digit;

    void clear();
    const char* encoding_name() const throw () override { return "CREX"; }
    void load_tables() override;
    void decode_header(const std::string& raw, const char* fname="(memory)", size_t offset=0) override;
    void decode(const std::string& raw, const char* fname="(memory)", size_t offset=0) override;
    void encode(std::string& buf) override;
    void print_details(FILE* out) const override;
    unsigned diff_details(const Bulletin& msg) const override;

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

    /**
     * To prevent breaking ABI if new members are added to bulletins, direct
     * construction is discouraged in favour of an allocator function
     */
    static std::unique_ptr<CrexBulletin> create();

protected:
    CrexBulletin();
};

/**
 * The bulletin namespace contains bulletin implementation details, internals
 * and utility functions.
 */
namespace bulletin {
}

}

/* vim:set ts=4 sw=4: */
#endif
