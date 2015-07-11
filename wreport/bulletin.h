#ifndef WREPORT_BULLETIN_H
#define WREPORT_BULLETIN_H

#include <wreport/var.h>
#include <wreport/subset.h>
#include <wreport/opcodes.h>
#include <wreport/tables.h>
#include <vector>
#include <memory>

/**
 * Main namespace
 */
namespace wreport {
struct DTable;

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
     * If not available, it is empty.
     */
    std::string fname;

    /**
     * File offset of the start of the message.
     *
     * If available, it will be used to generate better error messages.
     *
     * If not available, it is 0.
     */
    off_t offset = 0;

    /// Data category (BUFR or CREX Table A)
    uint8_t data_category = 0xff;

    /// International data sub-category (see Common Code table C-13)
    uint8_t data_subcategory = 0xff;

    /**
     * Local data sub-category, defined locally by automatic data-processing
     * (ADP) centres.
     *
     * Note: the local data sub-category is maintained for backwards
     * compatibility with BUFR editions 0-3, since many ADP centres have made
     * extensive use of such values in the past. The international data
     * sub-category introduced with BUFR edition 4 is intended to provide a
     * mechanism for better understanding of the overall nature and intent of
     * messages exchanged between ADP centres. These two values (i.e. local
     * sub-category and international sub-category) are intended to be
     * supplementary to one another, so both may be used within a particular
     * BUFR message.
     */
    uint8_t data_subcategory_local = 0xff;

    /**
     * Identification of originating/generating centre (see Common Code table
     * C-11)
     */
    uint16_t originating_centre = 0xffff;

    /**
     * Identification of originating/generating sub-centre (allocated by
     * originating/generating centre - see Common Code table C-12)
     */
    uint16_t originating_subcentre = 0xffff;

    /**
     * Update sequence number (zero for original messages and for messages
     * containing only delayed reports; incremented for the other updates)
     */
    uint8_t update_sequence_number = 0;

    /**
     * Most typical time for the BUFR message contents.
     *
     * When accuracy of the time does not define a time unit, then the value
     * for this unit shall be set to zero (e.g. for a SYNOP observation at 09
     * UTC, minute = 0, second = 0).
     * @{ */
    uint16_t rep_year = 0;
    uint8_t rep_month = 0;
    uint8_t rep_day = 0;
    uint8_t rep_hour = 0;
    uint8_t rep_minute = 0;
    uint8_t rep_second = 0;
    /** @} */

    /// Varcode and opcode tables used for encoding or decoding
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
    virtual void encode(std::string& buf) const = 0;

    /// Dump the contents of this bulletin
    void print(FILE* out) const;

    /// Dump the contents of this bulletin, in a more structured way
    void print_structured(FILE* out) const;

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
    /// BUFR edition number
    uint8_t edition_number = 4;

    /**
     * BUFR Master table number.
     *
     * A master table may be defined for a scientific discipline other than meteorology.
     * The current list of master tables, along with their associated values in
     * octet 4, is as follows:
     *
     * \l 0: Meteorology maintained by the World Meteorological Organization (WMO)
     * \l 10: Oceanography maintained by the Intergovernmental Oceanographic Commission (IOC) of UNESCO
     */
    uint8_t master_table_number = 0;

    /**
     * Version number of BUFR master table used.
     *
     * See WMO Manual on Codes, Binary codes, FM94-XIV BUFR, Section 1
     * Identification section, note 5, or FB95-XIV CREX, Specification of
     * sections, note 3, for a list.
     */
    uint8_t master_table_version_number = 19;

    /**
     * Version number of local table used to augment the master table.
     *
     * Local tables shall define those parts of the master table which are
     * reserved for local use, thus version numbers of local tables may be
     * changed at will by the originating centre. If no local table is used,
     * the version number of the local table shall be encoded as 0.
     */
    uint8_t master_table_version_number_local = 0;

    /// Whether the message is compressed
    bool compression;

    /**
     * Raw optional section of the message.
     *
     * It is empty if the message does not contain an optional section.
     */
    std::string optional_section;

    /**
     * Options used to customise encoding or decoding.
     *
     * It is NULL by default, in which case default options are used.
     *
     * To configure it, set it to point to a BufrCodecOptions structure with
     * the parameters you need. The caller is responsible for the memory
     * management of the BufrCodecOptions structure.
     */
    const BufrCodecOptions* codec_options = nullptr;


	virtual ~BufrBulletin();

    void clear();
    const char* encoding_name() const throw () override { return "BUFR"; }
    void load_tables() override;
    void decode_header(const std::string& raw, const char* fname="(memory)", size_t offset=0) override;
    void decode(const std::string& raw, const char* fname="(memory)", size_t offset=0) override;
    void encode(std::string& buf) const override;
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
    static std::unique_ptr<BufrBulletin> create();

protected:
    BufrBulletin();
};

/**
 * CREX bulletin implementation
 */
struct CrexBulletin : public Bulletin
{
    /// CREX Edition number
    uint8_t edition_number = 2;

    /**
     * CREX Master table number.
     *
     * A master table may be defined for a scientific discipline other than meteorology.
     * The current list of master tables, along with their associated values in
     * octet 4, is as follows:
     *
     * \l 0: Meteorology maintained by the World Meteorological Organization (WMO)
     * \l 10: Oceanography maintained by the Intergovernmental Oceanographic Commission (IOC) of UNESCO
     */
    uint8_t master_table_number = 0;

    /**
     * CREX master table version number.
     *
     * See WMO Manual on Codes, FB95-XIV CREX, Specification of sections, note
     * 3, for a list.
     */
    uint8_t master_table_version_number = 19;

    /**
     * BUFR master table version number.
     *
     * See WMO Manual on Codes, Binary codes, FM94-XIV BUFR, Section 1
     * Identification section, note 5, for a list.
     *
     * FIXME: I could not find any reference to why CREX edition 2 has a
     * separate field for BUFR master table version number but not for BUFR
     * master table version, or why it needs to reference BUFR master tables at
     * all.
     */
    uint8_t master_table_version_number_bufr = 19;

    /**
     * Version number of local table used to augment the master table.
     *
     * Local tables shall define those parts of the master table which are
     * reserved for local use, thus version numbers of local tables may be
     * changed at will by the originating centre. If no local table is used,
     * the version number of the local table shall be encoded as 0.
     */
    uint8_t master_table_version_number_local = 0;

    /// True if the CREX message uses the check digit feature
    bool has_check_digit = false;


    void clear();
    const char* encoding_name() const throw () override { return "CREX"; }
    void load_tables() override;
    void decode_header(const std::string& raw, const char* fname="(memory)", size_t offset=0) override;
    void decode(const std::string& raw, const char* fname="(memory)", size_t offset=0) override;
    void encode(std::string& buf) const override;
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
