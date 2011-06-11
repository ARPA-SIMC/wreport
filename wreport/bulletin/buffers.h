/*
 * wreport/bulletin/buffers - Low-level I/O operations
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

#ifndef WREPORT_BULLETIN_BUFFERS_H
#define WREPORT_BULLETIN_BUFFERS_H

#include <wreport/error.h>
#include <string>
#include <stdint.h>

namespace wreport {
namespace bulletin {

class BufrInput
{
protected:
    /**
     * Scan length of section \a sec_no, filling in the start of the next
     * section in sec[sec_no + 1]
     */
    void scan_section_length(unsigned sec_no);

public:
    /// Input buffer
    const unsigned char* data;

    /// Input buffer size
    size_t data_len;

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
    size_t start_offset;

    /// Offset of the byte we are currently decoding
    unsigned s4_cursor;

    /// Byte we are currently decoding
    unsigned char pbyte;

    /// Bits left in pbyte to decode
    int pbyte_len;

    /// Offsets of the start of BUFR sections
    unsigned sec[6];


    BufrInput(const std::string& in);

    /// Start decoding a different buffer
    void reset(const std::string& in);

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
    int offset() const;

    /// Return the number of bits left in the message to be decoded
    int bits_left() const;

    /**
     * Read a byte value at offset \a pos
     */
    inline unsigned read_byte(unsigned pos) const
    {
        return (unsigned)data[pos];
    }

    /**
     * Read a byte value at offset \a pos inside section \a section
     */
    inline unsigned read_byte(unsigned section, unsigned pos) const
    {
        return (unsigned)data[sec[section] + pos];
    }

    /**
     * Read a big endian integer value \a byte_len bytes long, at offset \a pos
     */
    unsigned read_number(unsigned pos, unsigned byte_len) const;

    /**
     * Read a big endian integer value \a byte_len bytes long, at offset \a pos
     * inside section \a section
     */
    inline unsigned read_number(unsigned section, unsigned pos, unsigned byte_len) const
    {
        return read_number(sec[section] + pos, byte_len);
    }

    /**
     * Get the integer value of the next 'n' bits from the decode input
     * n must be <= 32.
     */
    uint32_t get_bits(unsigned n);

    /// Dump to stderr 'count' bits of 'buf', starting at the 'ofs-th' bit
    void debug_dump_next_bits(int count) const;

    /// Throw an error_parse at the current decoding location
    void parse_error(const char* fmt, ...) const WREPORT_THROWF_ATTRS(2, 3);

    /// Throw an error_parse at the given decoding location
    void parse_error(unsigned pos, const char* fmt, ...) const WREPORT_THROWF_ATTRS(3, 4);

    /// Throw an error_parse at the given decoding location inside the given section
    void parse_error(unsigned section, unsigned pos, const char* fmt, ...) const WREPORT_THROWF_ATTRS(4, 5);

    /**
     * Check that the input buffer contains at least \a datalen characters
     * after offset \a pos; throw error_parse otherwise.
     *
     * @param expected
     *    name of what we are about to decode, used for generating nice error
     *    messages
     */
    void check_available_data(unsigned pos, size_t datalen, const char* expected);

    /**
     * Check that the input buffer contains at least \a datalen characters
     * after offset \a pos in section \a section; throw error_parse otherwise.
     *
     * @param expected
     *    name of what we are about to decode, used for generating nice error
     *    messages
     */
    void check_available_data(unsigned section, unsigned pos, size_t datalen, const char* expected);

    /**
     * Read the size of the given section based 
     */
    //void read_section_size(int num);
};

struct BufrOutput
{
};

struct CrexInput
{
    /// Input buffer
    const char* data;

    /// Input buffer size
    size_t data_len;

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

    /**
     * Character offsets of the starts of CREX sections
     */
    unsigned sec[5];

    /// Cursor inside in.data() used for decoding
    const char* cur;

    CrexInput(const std::string& in);

    /// Return true if the cursor is at the end of the buffer
    bool eof() const;

    /// Return the number of remaining characters until the end of the buffer
    unsigned remaining() const;

    /// Throw an error_parse at the current decoding location
    void parse_error(const char* fmt, ...) const WREPORT_THROWF_ATTRS(2, 3);

    /**
     * Check if the decoding cursor has reached the end of buffer, throw
     * error_parse otherwise.
     *
     * @param expected
     *    name of what we are about to decode, used for generating nice error
     *    messages
     */
    void check_eof(const char* expected) const;

    /**
     * Check that the input buffer contains at least \a datalen characters
     * after the cursor, throw error_parse otherwise.
     *
     * @param expected
     *    name of what we are about to decode, used for generating nice error
     *    messages
     */
    void check_available_data(unsigned datalen, const char* expected) const;

    /// Move the cursor to the next non-space character
    void skip_spaces();

    /// Skip \a datalen characters and all spaces after them
    void skip_data_and_spaces(unsigned datalen);

    /**
     * Mark the start of the given section.
     *
     * This sets sec[num] to the position pointed by the current cursor, and
     * throws error_parse if the section starts at the end of the buffer.
     *
     * There is no need to mark the start of section 0, as it is always at
     * offset 0.
     */
    void mark_section_start(unsigned num);

    /**
     * Read a word into \a buf.
     *
     * A word ends at the first whitespace or at the end of buffer.
     *
     * Whitespace after the word is skipped.
     *
     * @param buf
     *   The buffer that will hold the word. It will always be zero-terminated.
     * @param len
     *   Length of the buffer. Since the buffer will always be zero-terminated,
     *   the word will be at most \a len-1 characters long.
     */
    void read_word(char* buf, size_t len);

    /// Dump to stderr the contents of the next bit of buffer
    void debug_dump_next(const char* desc) const;
};

struct CrexOutput
{
};

}
}

#endif
