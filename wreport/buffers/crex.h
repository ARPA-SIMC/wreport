#ifndef WREPORT_BUFFERS_CREX_H
#define WREPORT_BUFFERS_CREX_H

#include <wreport/error.h>
#include <wreport/varinfo.h>
#include <string>

namespace wreport {
struct Var;

namespace buffers {

/**
 * Text input buffer
 */
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

    /// True if the CREX message uses the check digit feature
    int has_check_digit;

    /// Value of the next expected check digit
    int expected_check_digit;


    /**
     * Wrap a string into a CrexInput
     *
     * @param in
     *   The string with the data to read
     */
    CrexInput(const std::string& in, const char* fname, size_t offset);

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
     *   name of what we are about to decode, used for generating nice error
     *   messages
     */
    void check_eof(const char* expected) const;

    /**
     * Check that the input buffer contains at least \a datalen characters
     * after the cursor, throw error_parse otherwise.
     *
     * @param datalen
     *   number of bytes expected to still be available at the current location
     * @param expected
     *   name of what we are about to decode, used for generating nice error
     *   messages
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

    /**
     * Parse a data value from the input buffer
     *
     * @param len
     *   Length in characters of the value to parse
     * @param is_signed
     *   True if the value can be preceded by a + or - sign
     * @retval d_start
     *   Start of the parsed token
     * @retval d_end
     *   End of the parsed token
     */
    void parse_value(int len, int is_signed, const char** d_start, const char** d_end);

    /// Dump to stderr the contents of the next bit of buffer
    void debug_dump_next(const char* desc) const;
};

/**
 * Text output buffer
 */
struct CrexOutput
{
    /// String we append to
    std::string& buf;

    /// True if the CREX message uses the check digit feature
    int has_check_digit;

    /// Value of the next expected check digit
    int expected_check_digit;


    /**
     * Wrap a string with a CrexOutput
     *
     * @param buf
     *   The string to append to
     */
    CrexOutput(std::string& buf);

    /// Append a string
    void raw_append(const char* str, int len);

    /// Append a printf-formatted string
    void raw_appendf(const char* fmt, ...) __attribute__ ((format(printf, 2, 3)));

    /// Generate and append a check digit
    void encode_check_digit();

    /// Append a missing variable encoded according to \a info
    void append_missing(Varinfo info);

    /// Append a variable encoded according to \a info
    void append_var(Varinfo info, const Var& var);
};

}
}

#endif
