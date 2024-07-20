#ifndef WREPORT_BUFFERS_BUFR_H
#define WREPORT_BUFFERS_BUFR_H

#include <wreport/error.h>
#include <wreport/var.h>
#include <string>
#include <functional>
#include <cstdint>

namespace wreport {
struct Var;

namespace buffers {

/**
 * Binary buffer with bit-level append operations
 */
struct BufrOutput
{
    /// Output buffer to which we append encoded data
    std::string& out;

    /// Byte to which we are appending bits to encode
    uint8_t pbyte;

    /// Number of bits already encoded in pbyte
    int pbyte_len;

    /**
     * Wrap a string into a BufrOutput
     *
     * @param out
     *   String to append data to
     */
    BufrOutput(std::string& out);

    /**
     * Append n bits from 'val'.  n must be <= 32.
     */
    void add_bits(uint32_t val, int n);

    /**
     * Append a string \a len bits long to the output buffer as it is,
     * ignoring partially encoded bits
     */
    void raw_append(const char* str, size_t len)
    {
        out.append(str, len);
    }

    [[deprecated("Use the version with size_t len")]] void raw_append(const char* str, int len)
    {
        out.append(str, len);
    }


    /// Append a 16 bits integer
    void append_short(unsigned short val)
    {
        add_bits(val, 16);
    }

    /// Append an 8 bits integer
    void append_byte(unsigned char val)
    {
        add_bits(val, 8);
    }

    /// Append a missing value \a len_bits long
    void append_missing(unsigned len_bits)
    {
        add_bits(0xffffffff, len_bits);
    }

    /// Append a string variable
    void append_string(const Var& var, unsigned len_bits);

    /// Append a string \a len_bits bits long
    void append_string(const char* val, unsigned len_bits);

    /// Append a binary value \a len_bits bits long
    void append_binary(const unsigned char* val, unsigned len_bits);

    /// Append \a var encoded according to \a info
    void append_var(Varinfo info, const Var& var);

    /// Append a missing value according to \a info
    void append_missing(Varinfo info);

    /**
     * Write all bits left to the buffer, padding the last partial byte with
     * zeros if needed to make it even
     */
    void flush();
};


}
}

#endif
