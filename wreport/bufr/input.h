#ifndef WREPORT_BUFR_INPUT_H
#define WREPORT_BUFR_INPUT_H

#include <functional>
#include <string>
#include <wreport/bulletin.h>
#include <wreport/error.h>
#include <wreport/var.h>

namespace wreport {
struct Bulletin;

namespace bulletin {
struct AssociatedField;
}

namespace bufr {

struct DispatchToSubsets
{
    Bulletin& out;
    unsigned subset_count;
    DispatchToSubsets(Bulletin& out, unsigned subset_count)
        : out(out), subset_count(subset_count)
    {
    }

    void add_missing(Varinfo info)
    {
        for (unsigned i = 0; i < subset_count; ++i)
            out.subsets[i].store_variable_undef(info);
    }
    void add_same(const Var& var)
    {
        for (unsigned i = 0; i < subset_count; ++i)
            out.subsets[i].store_variable(Var(var));
    }
    void add_var(unsigned subset, Var&& var)
    {
        out.subsets[subset].store_variable(var);
    }
};

/**
 * Binary buffer with bit-level read operations
 */
class Input
{
protected:
    /**
     * Scan length of section \a sec_no, filling in the start of the next
     * section in sec[sec_no + 1]
     */
    void scan_section_length(unsigned sec_no);

public:
    /// Input buffer
    const uint8_t* data;

    /// Input buffer size
    size_t data_len;

    /**
     * Input file name (optional).
     *
     * If available, it will be used to generate better error messages.
     *
     * If not available, it is NULL.
     */
    const char* fname = nullptr;

    /**
     * File offset of the start of the message.
     *
     * If available, it will be used to generate better error messages.
     *
     * If not available, it is 0.
     */
    size_t start_offset = 0;

    /// Offset of the byte we are currently decoding
    unsigned s4_cursor = 0;

    /// Byte we are currently decoding
    uint8_t pbyte = 0;

    /// Bits left in pbyte to decode
    int pbyte_len = 0;

    /// Offsets of the start of BUFR sections
    unsigned sec[6];

    /**
     * Wrap a string iinto a Input
     *
     * @param in
     *   String with the data to read
     */
    explicit Input(const std::string& in);

    /**
     * Scan the message filling in the sec[] array of start offsets of sections
     * 0 and 1.
     *
     * We cannot scan past section 1 until we decode it, because we need to
     * know if section 2 is present or not
     */
    void scan_lead_sections();

    /**
     * Scan the message filling in the sec[] array of section start offsets of
     * all sections from 2 on.
     *
     * It also initialises points s4_cursor to the begin of the data in section
     * 4.
     *
     * @param has_optional
     *   True if the optional section is present, false if it should be
     *   skipped.
     */
    void scan_other_sections(bool has_optional);

    /// Return the current decoding byte offset
    unsigned offset() const { return s4_cursor; }

    /// Return the number of bits left in the message to be decoded
    unsigned bits_left() const
    {
        return static_cast<unsigned>((data_len - s4_cursor) * 8 + pbyte_len);
    }

    /// Read a byte value at offset \a pos
    inline unsigned read_byte(unsigned pos) const
    {
        return (unsigned)data[pos];
    }

    /// Read a byte value at offset \a pos inside section \a section
    inline unsigned read_byte(unsigned section, unsigned pos) const
    {
        return (unsigned)data[sec[section] + pos];
    }

    /// Read a big endian integer value \a byte_len bytes long, at offset \a pos
    unsigned read_number(unsigned pos, unsigned byte_len) const
    {
        unsigned res = 0;
        for (unsigned i = 0; i < byte_len; ++i)
        {
            res <<= 8;
            res |= data[pos + i];
        }
        return res;
    }

    /**
     * Read a big endian integer value \a byte_len bytes long, at offset \a pos
     * inside section \a section
     */
    inline unsigned read_number(unsigned section, unsigned pos,
                                unsigned byte_len) const
    {
        return read_number(sec[section] + pos, byte_len);
    }

    /**
     * Get the integer value of the next 'n' bits from the decode input
     * n must be <= 32.
     */
    uint32_t get_bits(unsigned n)
    {
        uint32_t result = 0;

        if (s4_cursor == data_len)
            parse_error(
                "end of buffer while looking for %u bits of bit-packed data",
                n);

        // TODO: review and benchmark and possibly simplify
        // (a possible alternative approach is to keep a current bitmask that
        // starts at 0x80 and is shifted right by 1 at each read until it
        // reaches 0, and get rid of pbyte_len)
        for (unsigned i = 0; i < n; i++)
        {
            if (pbyte_len == 0)
            {
                pbyte_len = 8;
                pbyte     = data[s4_cursor++];
            }
            result <<= 1;
            if (pbyte & 0x80)
                result |= 1;
            pbyte <<= 1;
            pbyte_len--;
        }

        return result;
    }

    /**
     * Skip the next n bits
     */
    void skip_bits(unsigned n)
    {
        if (s4_cursor == data_len)
            parse_error(
                "end of buffer while looking for %u bits of bit-packed data",
                n);

        for (unsigned i = 0; i < n; i++)
        {
            if (pbyte_len == 0)
            {
                pbyte_len = 8;
                pbyte     = data[s4_cursor++];
            }
            pbyte <<= 1;
            pbyte_len--;
        }
    }

    /// Dump to stderr 'count' bits of 'buf', starting at the 'ofs-th' bit
    void debug_dump_next_bits(const char* desc, unsigned count,
                              const std::vector<unsigned>& groups = {}) const;

    /**
     * Match the given pattern as regexp on the still unread input bitstream,
     * with bits converted to a string of '0' and '1'
     */
    void debug_find_sequence(const char* pattern) const;

    /// Throw an error_parse at the current decoding location
    void parse_error(const char* fmt, ...) const WREPORT_THROWF_ATTRS(2, 3);

    /// Throw an error_parse at the given decoding location
    void parse_error(unsigned pos, const char* fmt, ...) const
        WREPORT_THROWF_ATTRS(3, 4);

    /// Throw an error_parse at the given decoding location inside the given
    /// section
    void parse_error(unsigned section, unsigned pos, const char* fmt, ...) const
        WREPORT_THROWF_ATTRS(4, 5);

    /**
     * Check that the input buffer contains at least \a datalen characters
     * after offset \a pos; throw error_parse otherwise.
     *
     * @param pos
     *   Starting offset of the required data
     * @param datalen
     *   Required amount of data expected starting from \a pos
     * @param expected
     *    name of what we are about to decode, used for generating nice error
     *    messages
     */
    void check_available_data(unsigned pos, size_t datalen,
                              const char* expected);

    /**
     * Check that the input buffer contains at least \a datalen characters
     * after offset \a pos in section \a section; throw error_parse otherwise.
     *
     * @param section
     *   Number of the section to check
     * @param pos
     *   Starting offset inside the section of the required data
     * @param datalen
     *   Required amount of data expected starting from \a pos
     * @param expected
     *   Name of what we are about to decode, used for generating nice error
     *   messages
     */
    void check_available_message_data(unsigned section, unsigned pos,
                                      size_t datalen, const char* expected);

    /**
     * Check that the given section in the input buffer contains at least \a
     * datalen characters after offset \a pos; throw error_parse otherwise.
     *
     * @param section
     *   Number of the section to check
     * @param pos
     *   Starting offset inside the section of the required data
     * @param datalen
     *   Required amount of data expected starting from \a pos
     * @param expected
     *   Name of what we are about to decode, used for generating nice error
     *   messages
     */
    void check_available_section_data(unsigned section, unsigned pos,
                                      size_t datalen, const char* expected);

    /**
     * Decode a compressed number as described by dest.info(), ad set it as
     * value for \a dest.
     *
     * @param dest
     *   Variable which holds the decoding information and that will hold the
     *   decoded value
     * @param base
     *   The base value for the compressed number
     * @param diffbits
     *   The number of bits used to encode the difference from \a base
     */
    void decode_compressed_number(Var& dest, uint32_t base, unsigned diffbits);

    /**
     * Decode a number as described by dest.info(), and set it as value for \a
     * dest.
     *
     * @param dest
     *   Variable which holds the decoding information and that will hold the
     *   decoded value
     */
    void decode_number(Var& dest);

    /**
     * Decode the base value for a variable in a compressed BUFR
     */
    bool decode_compressed_base(Varinfo info, uint32_t& base,
                                uint32_t& diffbits);

    /**
     * Decode a number as described by \a info from a compressed bufr with
     * \a subsets subsets, and send the resulting variables to \a dest
     */
    void decode_compressed_number(Varinfo info, unsigned subsets,
                                  std::function<void(unsigned, Var&&)> dest);

    void decode_string(Varinfo info, unsigned subsets, DispatchToSubsets& dest);

    void decode_compressed_number(Varinfo info, unsigned subsets,
                                  DispatchToSubsets& dest);

    /**
     * Decode a number as described by \a info from a compressed bufr with
     * \a subsets subsets, and send the resulting variables to \a dest
     */
    void decode_compressed_number_af(Varinfo info,
                                     const bulletin::AssociatedField& afield,
                                     unsigned subsets,
                                     std::function<void(unsigned, Var&&)> dest);

    /**
     * Decode a number as described by dest.info(), and set it as value for \a
     * dest. The number is decoded for \a subsets compressed datasets, and an
     * exception is thrown if the values differ.
     *
     * @param dest
     *   Variable which holds the decoding information and that will hold the
     *   decoded value
     * @param subsets
     *   Number of subsets in the compressed data section
     */
    void decode_compressed_semantic_number(Var& dest, unsigned subsets);

    /**
     * Read a string from the data section
     *
     * @param bit_len
     *   Number of bits (not bytes) to read. It is normally a multiple of 8,
     *   and when it is not, the last character will contain the partial byte
     *   read.
     * @retval str
     *   Buffer where the string is written. Must be big enough to contain the
     *   longest string described by info, plus 2 bytes
     * @retval len
     *   The string length
     * @return
     *   true if we decoded a real string, false if we decoded a missing string
     *   value
     */
    bool decode_string(unsigned bit_len, char* str, size_t& len);

    /**
     * Decode a string as described by dest.info(), ad set it as value for \a
     * dest.
     *
     * It is assumed that \a dest is not set, therefore in case we decode a
     * missing value, \a dest will not be touched.
     *
     * @param dest
     *   Variable which holds the decoding information and that will hold the
     *   decoded value
     */
    void decode_string(Var& dest);

    /**
     * Decode a string as described by dest.info(), and set it as value for \a
     * dest. The string is decoded for \a subsets compressed datasets, and an
     * exception is thrown if the values differ.
     *
     * @param dest
     *   Variable which holds the decoding information and that will hold the
     *   decoded value
     * @param subsets
     *   Number of subsets in the compressed data section
     */
    void decode_string(Var& dest, unsigned subsets);

    /**
     * Decode a string as described by \a info from a compressed bufr with \a
     * subsets subsets, and send the resulting variables to \a dest
     */
    void decode_string(Varinfo info, unsigned subsets,
                       std::function<void(unsigned, Var&&)> dest);

    /**
     * Decode a generic binary value as-is, as described by dest.info(), ad set
     * it as value for \a dest.
     *
     * It is assumed that \a dest is not set, therefore in case we decode a
     * missing value, \a dest will not be touched.
     *
     * @param dest
     *   Variable which holds the decoding information and that will hold the
     *   decoded value
     */
    void decode_binary(Var& dest);

    /**
     * Decode an uncompressed bitmap of \a size bits.
     *
     * The result will be a string \a size bytes long, with a '+' where the
     * bitmap reports that data is present, and a '-' where the bitmap reports
     * that data is not present.
     */
    std::string decode_uncompressed_bitmap(unsigned size);

    /**
     * Decode a "compressed" bitmap of \a size bits.
     *
     * The result will be a string \a size bytes long, with a '+' where the
     * bitmap reports that data is present, and a '-' where the bitmap reports
     * that data is not present.
     *
     * It would be more correct to say that it decodes a bitmap from a
     * compressed BUFR message, because bitmaps in compressed messages are
     * actually encoded with 7 bits per bit instead of one, because after each
     * bit they need to send 6 bits saying that it will be followed by 0 bits
     * of difference values.
     */
    std::string decode_compressed_bitmap(unsigned size);
};

} // namespace bufr
} // namespace wreport
#endif
