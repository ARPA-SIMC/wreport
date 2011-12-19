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

#include <config.h>

#include "buffers.h"
#include "conv.h"
#include "var.h"
#include "vasprintf.h"

#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

using namespace std;

namespace wreport {
namespace bulletin {

namespace {

const char* bufr_sec_names[] = {
    "Indicator section",
    "Identification section",
    "Optional section",
    "Data desription section",
    "Data section",
    "End section"
};

// Return a value with bitlen bits set to 1
static inline uint32_t all_ones(int bitlen)
{
    return ((1 << (bitlen - 1))-1) | (1 << (bitlen - 1));
}

}

BufrInput::BufrInput(const std::string& in)
{
    reset(in);
}

void BufrInput::reset(const std::string& in)
{
    data = (const unsigned char*)in.data();
    data_len = in.size();
    fname = NULL;
    start_offset = 0;
    s4_cursor = 0;
    pbyte = 0;
    pbyte_len = 0;
    for (unsigned i = 0; i < sizeof(sec)/sizeof(sec[0]); ++i)
        sec[i] = 0;
}

void BufrInput::scan_section_length(unsigned sec_no)
{
    if (sec[sec_no] + 3 > data_len)
        parse_error(sec[sec_no], "section %d (%s) is too short to hold the section size indicator",
                sec_no, bufr_sec_names[sec_no]);

    sec[sec_no + 1] = sec[sec_no] + read_number(sec_no, 0, 3);

    if (sec[sec_no + 1] > data_len)
        parse_error(sec[sec_no], "section %d (%s) claims to end past the end of the BUFR message",
                sec_no, bufr_sec_names[sec_no]);
}

void BufrInput::scan_lead_sections()
{
    check_available_data(sec[0], 8, "section 0 of BUFR message (indicator section)");
    sec[1] = sec[0] + 8;
    scan_section_length(1);
}

void BufrInput::scan_other_sections(bool has_optional)
{
    if (has_optional)
    {
        scan_section_length(2);
    } else
        sec[3] = sec[2];

    for (unsigned i = 3; i < 5; ++i)
        scan_section_length(i);

    s4_cursor = sec[4] + 4;
}

int BufrInput::offset() const { return s4_cursor; }
int BufrInput::bits_left() const { return (data_len - s4_cursor) * 8 + pbyte_len; }

unsigned BufrInput::read_number(unsigned pos, unsigned byte_len) const
{
    unsigned res = 0;
    for (unsigned i = 0; i < byte_len; ++i)
    {
        res <<= 8;
        res |= data[pos + i];
    }
    return res;
}

uint32_t BufrInput::get_bits(unsigned n)
{
    uint32_t result = 0;

    if (s4_cursor == data_len)
        parse_error("end of buffer while looking for %d bits of bit-packed data", n);

    for (unsigned i = 0; i < n; i++) 
    {
        if (pbyte_len == 0) 
        {
            pbyte_len = 8;
            pbyte = data[s4_cursor++];
        }
        result <<= 1;
        if (pbyte & 0x80)
            result |= 1;
        pbyte <<= 1;
        pbyte_len--;
    }

    return result;
}

void BufrInput::debug_dump_next_bits(int count) const
{
    size_t cursor = s4_cursor;
    int pbyte = this->pbyte;
    int pbyte_len = this->pbyte_len;
    int i;

    for (i = 0; i < count; ++i) 
    {
        if (cursor == data_len)
            break;
        if (pbyte_len == 0) 
        {
            pbyte_len = 8;
            pbyte = data[cursor++];
            putc(' ', stderr);
        }
        putc((pbyte & 0x80) ? '1' : '0', stderr);
        pbyte <<= 1;
        --pbyte_len;
    }
}

void BufrInput::parse_error(const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&message, fmt, ap);
    va_end(ap);

    asprintf(&context, "%s:%zd+%u: %s", fname, start_offset, s4_cursor, message);
    free(message);

    string msg(context);
    free(context);

    throw error_parse(msg);
}

void BufrInput::parse_error(unsigned pos, const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&message, fmt, ap);
    va_end(ap);

    asprintf(&context, "%s:%zd+%u: %s", fname, start_offset, pos, message);
    free(message);

    string msg(context);
    free(context);
    throw error_parse(msg);
}

void BufrInput::parse_error(unsigned section, unsigned pos, const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&message, fmt, ap);
    va_end(ap);

    asprintf(&context, "%s:%zd+%u: %s (%db inside section %s)",
        fname, start_offset, sec[section] + pos, message,
        pos, bufr_sec_names[section]);
    free(message);

    string msg(context);
    free(context);
    throw error_parse(msg);
}

void BufrInput::check_available_data(unsigned pos, size_t datalen, const char* expected)
{
    if (pos + datalen > data_len)
        parse_error(pos, "end of BUFR message while looking for %s", expected);
}

void BufrInput::check_available_data(unsigned section, unsigned pos, size_t datalen, const char* expected)
{
    // TODO: check that sec[section] + pos + datalen > sec[section + 1] instead?
    // TODO: in that case, make a fake section 6 which starts at the end of
    // TODO: BUFR data
    if (sec[section] + pos + datalen > data_len)
        parse_error(section, pos, "end of BUFR message while looking for %s", expected);
}

bool BufrInput::decode_string(unsigned bit_len, char* str, size_t& len)
{
    int toread = bit_len;
    bool missing = true;
    len = 0;

    while (toread > 0)
    {
        int count = toread > 8 ? 8 : toread;
        uint32_t bitval = get_bits(count);
        /* Check that the string is not all 0xff, meaning missing value */
        if (bitval != 0xff && bitval != 0)
            missing = false;
        str[len++] = bitval;
        toread -= count;
    }

    if (!missing)
    {
        str[len] = 0;

        /* Convert space-padding into zero-padding */
        for (; len > 1 && (str[len - 1] == 0 || isspace(str[len - 1]));
                --len)
            str[len - 1] = 0;
    }

    return !missing;
}

void BufrInput::decode_string(Var& dest)
{
    Varinfo info = dest.info();

    char str[info->bit_len / 8 + 2];
    size_t len;
    bool missing = !decode_string(info->bit_len, str, len);

    /* Store the variable that we found */
    // Set the variable value
    if (!missing)
        dest.setc(str);
}

void BufrInput::decode_binary(Var& dest)
{
    Varinfo info = dest.info();

    unsigned char buf[info->bit_len / 8 + 1];
    size_t len = 0;
    unsigned toread = info->bit_len;
    bool missing = true;

    while (toread > 0)
    {
        unsigned count = toread > 8 ? 8 : toread;
        uint32_t bitval = get_bits(count);
        /* Check that the string is not all 0xff, meaning missing value */
        if (bitval != 0xff)
            missing = false;
        buf[len++] = bitval;
        toread -= count;
    }

    /* Store the variable that we found */
    // Set the variable value
    if (!missing)
        dest.set_binary(buf);
}

void BufrInput::decode_number(Var& dest)
{
    Varinfo info = dest.info();

    uint32_t val = get_bits(info->bit_len);

    // TRACE("datasec:decode_b_num:reading %s (%s), size %d, scale %d, starting point %d\n", info->desc, info->bufr_unit, info->bit_len, info->scale, val);

    // Check if there are bits which are not 1 (that is, if the value is present)
    // In case of delayed replications, there is no missing value
    bool missing = false;
    if (WR_VAR_X(info->var) == 31 && WR_VAR_F(info->var) == 0)
        switch (WR_VAR_Y(info->var))
        {
            case 1:
            case 2:
            case 11:
            case 12:
                break;
            default:
                missing = (val == all_ones(info->bit_len));
        }
    else
        missing = (val == all_ones(info->bit_len));

    // TRACE("datasec:decode_b_num:len %d val %d info-len %d info-desc %s\n", info->bit_len, val, info->bit_len, info->desc);

    /* Store the variable that we found */
    if (missing)
    {
        /* Create the new Var */
        // TRACE("datasec:decode_b_num:decoded as missing\n");
        dest.unset();
    } else {
        double dval = info->bufr_decode_int(val);
        // TRACE("datasec:decode_b_num:decoded as %f %s\n", dval, info->bufr_unit);
        /* Convert to target unit */
        dval = convert_units(info->bufr_unit, info->unit, dval);
        // TRACE("datasec:decode_b_num:converted to %f %s\n", dval, info->unit);
        /* Create the new Var */
        dest.setd(dval);
    }
}

void BufrInput::decode_number(Var& dest, uint32_t base, unsigned diffbits)
{
    Varinfo info = dest.info();

    // Decode the difference value
    uint32_t diff = get_bits(diffbits);

    // Check if it's all 1s: in that case it's a missing value
    if (base == all_ones(info->bit_len) || diff == all_ones(diffbits))
    {
        /* Missing value */
        //TRACE("datasec:decode_b_num:decoded[%d] as missing\n", i);
        dest.unset();
    } else {
        /* Compute the value for this subset */
        uint32_t newval = base + diff;
        double dval = info->bufr_decode_int(newval);
        //TRACE("datasec:decode_b_num:decoded[%d] as %d+%d=%d->%f %s\n", i, val, diff, newval, dval, info->bufr_unit);

        /* Convert to target unit */
        dval = convert_units(info->bufr_unit, info->unit, dval);
        //TRACE("datasec:decode_b_num:converted to %f %s\n", dval, info->unit);

        /* Create the new Var */
        dest.setd(dval);
    }
}

void BufrInput::decode_number(Varinfo info, unsigned subsets, CompressedVarSink& dest)
{
    Var var(info);

    uint32_t base = get_bits(info->bit_len);

    //TRACE("datasec:decode_b_num:reading %s (%s), size %d, scale %d, starting point %d\n", info->desc, info->bufr_unit, info->bit_len, info->scale, base);

    /* Check if there are bits which are not 1 (that is, if the value is present) */
    bool missing = (base == all_ones(info->bit_len));

    /*bufr_decoder_debug(decoder, "  %s: %d%s\n", info.desc, base, info.type);*/
    //TRACE("datasec:decode_b_num:len %d base %d info-len %d info-desc %s\n", info->bit_len, base, info->bit_len, info->desc);

    /* Store the variable that we found */

    /* If compression is in use, then we just decoded the base value.  Now
     * we need to decode all the offsets */

    /* Decode the number of bits (encoded in 6 bits) that these difference
     * values occupy */
    uint32_t diffbits = get_bits(6);
    if (missing && diffbits != 0)
        error_consistency::throwf("When decoding compressed BUFR data, the difference bit length must be 0 (and not %d like in this case) when the base value is missing", diffbits);

    //TRACE("Compressed number, base value %d diff bits %d\n", base, diffbits);

    for (unsigned i = 0; i < subsets; ++i)
    {
        decode_number(var, base, diffbits);
        dest(var, i);
    }
}

void BufrInput::decode_number(Var& dest, unsigned subsets)
{
    Varinfo info = dest.info();

    uint32_t base = get_bits(info->bit_len);

    //TRACE("datasec:decode_b_num:reading %s (%s), size %d, scale %d, starting point %d\n", info->desc, info->bufr_unit, info->bit_len, info->scale, base);

    /* Check if there are bits which are not 1 (that is, if the value is present) */
    bool missing = (base == all_ones(info->bit_len));

    /*bufr_decoder_debug(decoder, "  %s: %d%s\n", info.desc, base, info.type);*/
    //TRACE("datasec:decode_b_num:len %d base %d info-len %d info-desc %s\n", info->bit_len, base, info->bit_len, info->desc);

    /* Store the variable that we found */

    /* If compression is in use, then we just decoded the base value.  Now
     * we need to decode all the offsets */

    /* Decode the number of bits (encoded in 6 bits) that these difference
     * values occupy */
    uint32_t diffbits = get_bits(6);
    if (missing && diffbits != 0)
        error_consistency::throwf("When decoding compressed BUFR data, the difference bit length must be 0 (and not %d like in this case) when the base value is missing", diffbits);

    //TRACE("Compressed number, base value %d diff bits %d\n", base, diffbits);

    // Decode the destination variable
    decode_number(dest, base, diffbits);

    // Decode all other versions and ensure they are the same
    Var copy(dest.info());
    for (unsigned i = 1; i < subsets; ++i)
    {
        // TODO: only compare the diffbits without needing to reconstruct the var
        decode_number(copy, base, diffbits);
        if (dest != copy)
        {
            string val1 = dest.format();
            string val2 = copy.format();
            error_consistency::throwf("When decoding %d%02d%03d from compressed BUFR data, decoded values differ (%s != %s) but should all be the same",
                   WR_VAR_F(dest.code()), WR_VAR_X(dest.code()), WR_VAR_Y(dest.code()),
                   val2.c_str(), val1.c_str());
        }
    }
}

void BufrInput::decode_string(Varinfo info, unsigned subsets, CompressedVarSink& dest)
{
    /* Read a string */
    Var var(info);
    //size_t len = 0;

    char str[info->bit_len / 8 + 2];
    size_t len;
    bool missing = !decode_string(info->bit_len, str, len);

    /* Store the variable that we found */

    /* If compression is in use, then we just decoded the base value.  Now
     * we need to decode all the offsets */

    /* Decode the number of bits (encoded in 6 bits) that these difference
     * values occupy */
    uint32_t diffbits = get_bits(6);

    //TRACE("datadesc:decode_b_string:compressed string, base:%.*s, diff bits %d\n", (int)len, str, diffbits);

    if (diffbits != 0)
    {
        /* For compressed strings, the reference value must be all zeros */
        for (size_t i = 0; i < len; ++i)
            if (str[i] != 0)
                error_unimplemented::throwf("compressed strings with %d bit deltas have non-zero reference value", diffbits);

        /* Let's also check that the number of
         * difference characters is the same length as
         * the reference string */
        if (diffbits > len)
            error_unimplemented::throwf("compressed strings with %zd characters have %d bit deltas (deltas should not be longer than field)", len, diffbits);

        for (unsigned i = 0; i < subsets; ++i)
        {
            // Set the variable value
            if (decode_string(diffbits * 8, str, len))
            {
                /* Compute the value for this subset */
                //TRACE("datadesc:decode_b_string:decoded[%d] as \"%s\"\n", i, str);
                var.setc(str);
            } else {
                /* Missing value */
                //TRACE("datadesc:decode_b_string:decoded[%d] as missing\n", i);
                var.unset();
            }

            /* Add it to this subset */
            dest(var, i);
        }
    } else {
        /* Add the string to all the subsets */
        for (unsigned i = 0; i < subsets; ++i)
        {
            // Set the variable value
            if (!missing) var.setc(str);
            dest(var, i);
        }
    }
}

void BufrInput::decode_string(Var& dest, unsigned subsets)
{
    Varinfo info = dest.info();

    char str[info->bit_len / 8 + 2];
    size_t len;
    bool missing = !decode_string(info->bit_len, str, len);

    /* Store the variable that we found */

    /* If compression is in use, then we just decoded the base value.  Now
     * we need to decode all the offsets */

    /* Decode the number of bits (encoded in 6 bits) that these difference
     * values occupy */
    uint32_t diffbits = get_bits(6);

    //TRACE("datadesc:decode_b_string:compressed string, base:%.*s, diff bits %d\n", (int)len, str, diffbits);

    if (diffbits == 0)
    {
        if (!missing)
            dest.setc(str);
    } else {
        /* For compressed strings, the reference value must be all zeros */
        for (size_t i = 0; i < len; ++i)
            if (str[i] != 0)
                error_unimplemented::throwf("compressed strings with %d bit deltas have non-zero reference value", diffbits);

        /* Let's also check that the number of
         * difference characters is the same length as
         * the reference string */
        if (diffbits > len)
            error_unimplemented::throwf("compressed strings with %zd characters have %d bit deltas (deltas should not be longer than field)", len, diffbits);

        // Set the variable value
        if (decode_string(diffbits * 8, str, len))
        {
            /* Compute the value for this subset */
            //TRACE("datadesc:decode_b_string:decoded[%d] as \"%s\"\n", i, str);
            dest.setc(str);
        } else {
            /* Missing value */
            //TRACE("datadesc:decode_b_string:decoded[%d] as missing\n", i);
            dest.unset();
        }

        Var copy(dest.info());
        for (unsigned i = 1; i < subsets; ++i)
        {
            // TODO: only compare the diffbits without needing to reconstruct the var

            // Set the variable value
            if (decode_string(diffbits * 8, str, len))
            {
                /* Compute the value for this subset */
                //TRACE("datadesc:decode_b_string:decoded[%d] as \"%s\"\n", i, str);
                copy.setc(str);
            } else {
                /* Missing value */
                //TRACE("datadesc:decode_b_string:decoded[%d] as missing\n", i);
                copy.unset();
            }

            if (dest != copy)
            {
                string val1 = dest.format();
                string val2 = copy.format();
                error_consistency::throwf("When decoding %d%02d%03d from compressed BUFR data, decoded values differ (%s != %s) but should all be the same",
                       WR_VAR_F(dest.code()), WR_VAR_X(dest.code()), WR_VAR_Y(dest.code()),
                       val2.c_str(), val1.c_str());
            }
        }
    }
}


BufrOutput::BufrOutput(std::string& out)
    : out(out), pbyte(0), pbyte_len(0)
{
}

void BufrOutput::add_bits(uint32_t val, int n)
{
    /* Mask for reading data out of val */
    uint32_t mask = 1 << (n - 1);
    int i;

    for (i = 0; i < n; i++) 
    {
        pbyte <<= 1;
        pbyte |= ((val & mask) != 0) ? 1 : 0;
        val <<= 1;
        pbyte_len++;

        if (pbyte_len == 8) 
            flush();
    }
#if 0
    IFTRACE {
        /* Prewrite it when tracing, to allow to dump the buffer as it's
         * written */
        while (e->out->len + 1 > e->out->alloclen)
            DBA_RUN_OR_RETURN(dba_rawmsg_expand_buffer(e->out));
        e->out->buf[e->out->len] = e->pbyte << (8 - e->pbyte_len);
    }
#endif
}

void BufrOutput::append_string(const Var& var, unsigned len_bits)
{
    append_string(var.value(), len_bits);
}

void BufrOutput::append_string(const char* val, unsigned len_bits)
{
    unsigned i, bi;
    bool eol = false;
    for (i = 0, bi = 0; bi < len_bits; ++i)
    {
        //TRACE("append_string:len: %d, i: %d, bi: %d, eol: %d\n", len_bits, i, bi, (int)eol);
        if (!eol && !val[i])
            eol = true;

        /* Strings are space-padded in BUFR */
        if (len_bits - bi >= 8)
        {
            append_byte(eol ? ' ' : val[i]);
            bi += 8;
        }
        else
        {
            /* Pad with zeros if writing strings with a number of bits
             * which is not multiple of 8.  It's not worth to implement
             * writing partial bytes at the moment and it's better to fail
             * gracefully, as my understanding is that this case should
             * never happen anyway. */
            add_bits(0, len_bits - bi);
            bi = len_bits;
        }
    }
}

void BufrOutput::append_binary(const unsigned char* val, unsigned len_bits)
{
    unsigned i, bi;
    for (i = 0, bi = 0; bi < len_bits; ++i)
    {
        /* Strings are space-padded in BUFR */
        if (len_bits - bi >= 8)
        {
            append_byte(val[i]);
            bi += 8;
        }
        else
        {
            add_bits(val[i], len_bits - bi);
            bi = len_bits;
        }
    }
}

void BufrOutput::append_var(Varinfo info, const Var& var)
{
    if (var.value() == NULL)
    {
        append_missing(info->bit_len);
    } else if (info->is_string()) {
        append_string(var.value(), info->bit_len);
    } else if (info->is_binary()) {
        append_binary((const unsigned char*)var.value(), info->bit_len);
    } else {
        unsigned ival = info->encode_bit_int(var.enqd());
        add_bits(ival, info->bit_len);
    }
}

void BufrOutput::append_missing(Varinfo info)
{
    append_missing(info->bit_len);
}

void BufrOutput::flush()
{
    if (pbyte_len == 0) return;

    while (pbyte_len < 8)
    {
        pbyte <<= 1;
        pbyte_len++;
    }

    out.append((const char*)&pbyte, 1);
    pbyte_len = 0;
    pbyte = 0;
}



CrexInput::CrexInput(const std::string& in)
    : data(in.c_str()), data_len(in.size()), fname(NULL), offset(0), cur(data), has_check_digit(false)
{
    for (int i = 0; i < 5; ++i)
        sec[i] = 0;
}

bool CrexInput::eof() const
{
    return cur >= data + data_len;
}

unsigned CrexInput::remaining() const
{
    return data + data_len - cur;
}

void CrexInput::parse_error(const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&message, fmt, ap);
    va_end(ap);

    asprintf(&context, "%s:%zd+%d: %s", fname, offset, (int)(cur - data), message);

    string msg(context);
    free(context);
    free(message);
    throw error_parse(msg);
}

void CrexInput::check_eof(const char* expected) const
{
    if (cur >= data + data_len)
        parse_error("end of CREX message while looking for %s", expected);
}

void CrexInput::check_available_data(unsigned datalen, const char* expected) const
{
    if (cur + datalen > data + data_len)
        parse_error("end of CREX message while looking for %s", expected);
}

void CrexInput::skip_spaces()
{
    while (cur < data + data_len && isspace(*cur))
        ++cur;
}

void CrexInput::skip_data_and_spaces(unsigned datalen)
{
    cur += datalen;
    skip_spaces();
}

void CrexInput::mark_section_start(unsigned num)
{
    check_eof("start of section 1");
    if (cur >= data + data_len)
        parse_error("end of CREX message at start of section %u", num);
    sec[num] = cur - data;
}

void CrexInput::read_word(char* buf, size_t len)
{
    size_t i;
    for (i = 0; i < len - 1 && !eof() && !isspace(*cur); ++cur, ++i)
        buf[i] = *cur;
    buf[i] = 0;

    skip_spaces();
}

void CrexInput::parse_value(int len, int is_signed, const char** d_start, const char** d_end)
{
    //TRACE("crex_decoder_parse_value(%d, %s): ", len, is_signed ? "signed" : "unsigned");

    /* Check for 2 more because we may have extra sign and check digit */
    check_available_data(len + 2, "end of data descriptor section");

    if (has_check_digit)
    {
        if ((*cur - '0') != expected_check_digit)
            parse_error("check digit mismatch: expected %d, found %d, rest of message: %.*s",
                    expected_check_digit,
                    (*cur - '0'),
                    (int)remaining(),
                    cur);

        expected_check_digit = (expected_check_digit + 1) % 10;
        ++cur;
    }

    /* Set the value to start after the check digit (if present) */
    *d_start = cur;

    /* Cope with one extra character in case the sign is present */
    if (is_signed && *cur == '-')
        ++len;

    /* Go to the end of the message */
    cur += len;

    /* Set the end value, removing trailing spaces */
    for (*d_end = cur; *d_end > *d_start && isspace(*(*d_end - 1)); (*d_end)--)
        ;

    /* Skip trailing spaces */
    skip_spaces();

    //TRACE("%.*s\n", *d_end - *d_start, *d_start);
}

void CrexInput::debug_dump_next(const char* desc) const
{
    fputs(desc, stderr);
    fputs(": ", stderr);
    for (size_t i = 0; i < 30 && cur + i < data + data_len; ++i)
    {
        switch (*(cur + i))
        {
            case '\r':
                fputs("\\r", stderr);
            case '\n':
                fputs("\\n", stderr);
                break;
            default:
                putc(*(cur + i), stderr);
                break;
        }
    }
    if (cur + 30 < data + data_len)
        fputs("…", stderr);
    putc('\n', stderr);
}


CrexOutput::CrexOutput(std::string& buf) : buf(buf), has_check_digit(0), expected_check_digit(0)
{
}

void CrexOutput::raw_append(const char* str, int len)
{
    buf.append(str, len);
}

void CrexOutput::raw_appendf(const char* fmt, ...)
{
    char sbuf[256];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(sbuf, 255, fmt, ap);
    va_end(ap);

    buf.append(sbuf, len);
}

void CrexOutput::encode_check_digit()
{
    if (!has_check_digit) return;

    char c = '0' + expected_check_digit;
    raw_append(&c, 1);
    expected_check_digit = (expected_check_digit + 1) % 10;
}

void CrexOutput::append_missing(Varinfo info)
{
    // TRACE("encode_b missing len: %d\n", info->len);
    for (unsigned i = 0; i < info->len; i++)
        raw_append("/", 1);
}

void CrexOutput::append_var(Varinfo info, const Var& var)
{
    if (var.value() == NULL)
        return append_missing(info);

    int len = info->len;
    raw_append(" ", 1);
    encode_check_digit();

    if (info->is_string()) {
        raw_appendf("%-*.*s", len, len, var.value());
        // TRACE("encode_b string len: %d val %-*.*s\n", len, len, len, var.value());
    } else {
        int val = var.enqi();

        /* FIXME: here goes handling of active C table modifiers */

        if (val < 0) ++len;

        raw_appendf("%0*d", len, val);
        // TRACE("encode_b num len: %d val %0*d\n", len, len, val);
    }
}

}
}
