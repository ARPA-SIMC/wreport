#include "input.h"
#include "trace.h"
#include "utils/sys.h"
#include "wreport/bulletin/associated_fields.h"
#include <algorithm>
#include <cstdarg>
#include <regex.h>

namespace {

const char* bufr_sec_names[] = {"Indicator section", "Identification section",
                                "Optional section",  "Data desription section",
                                "Data section",      "End section"};

// Return a value with bitlen bits set to 1
static inline uint32_t all_ones(int bitlen)
{
    return ((1 << (bitlen - 1)) - 1) | (1 << (bitlen - 1));
}

} // namespace

namespace wreport {
namespace bufr {

Input::Input(const std::string& in) : sec()
{
    data     = (const unsigned char*)in.data();
    data_len = in.size();
    for (auto& s : sec)
        s = 0;
}

void Input::scan_section_length(unsigned sec_no)
{
    if (sec[sec_no] + 3 > data_len)
        parse_error(
            sec[sec_no],
            "section %u (%s) is too short to hold the section size indicator",
            sec_no, bufr_sec_names[sec_no]);

    sec[sec_no + 1] = sec[sec_no] + read_number(sec_no, 0, 3);

    if (sec[sec_no + 1] > data_len)
        parse_error(
            sec[sec_no],
            "section %u (%s) claims to end past the end of the BUFR message",
            sec_no, bufr_sec_names[sec_no]);
}

void Input::scan_lead_sections()
{
    sec[1] = sec[0] + 8;
    scan_section_length(1);
}

void Input::scan_other_sections(bool has_optional)
{
    if (has_optional)
    {
        scan_section_length(2);
    }
    else
        sec[3] = sec[2];

    for (unsigned i = 3; i < 5; ++i)
        scan_section_length(i);

    s4_cursor = sec[4] + 4;
}

void Input::debug_dump_next_bits(const char* desc, unsigned count,
                                 const std::vector<unsigned>& groups) const
{
    static const char* cols[] = {"\033[33m", "\033[37m"};
    unsigned cols_pos         = 0;
    unsigned cols_size        = 2;
    fputs(desc, stderr);
    size_t cursor                 = s4_cursor;
    int pbyte                     = this->pbyte;
    int pbyte_len                 = this->pbyte_len;
    std::vector<unsigned> markers = groups;
    for (unsigned i = 1; i < markers.size(); ++i)
        markers[i] += markers[i - 1];

    if (!markers.empty())
    {
        fputs(cols[cols_pos], stderr);
        cols_pos = (cols_pos + 1) % cols_size;
    }

    for (unsigned i = 0; i < count; ++i)
    {
        if (cursor == data_len)
            break;
        if (!markers.empty() && markers[0] == i)
        {
            while (!markers.empty() && markers[0] == i)
                markers.erase(markers.begin());
            if (markers.empty())
                fputs("\033[39m\033[49m", stderr);
            else
            {
                fputs(cols[cols_pos], stderr);
                cols_pos = (cols_pos + 1) % cols_size;
            }
        }
        if (pbyte_len == 0)
        {
            pbyte_len = 8;
            pbyte     = data[cursor++];
            // putc(' ', stderr);
        }
        putc((pbyte & 0x80) ? '1' : '0', stderr);
        pbyte <<= 1;
        --pbyte_len;
    }
    putc('\n', stderr);
}

void Input::debug_find_sequence(const char* pattern) const
{
    // Build a regexp with the pattern
    regex_t compiled;
    if (int code = regcomp(&compiled, pattern, REG_EXTENDED))
    {
        size_t size = regerror(code, &compiled, nullptr, 0);
        char* buf   = new char[size];
        regerror(code, &compiled, buf, size);
        std::string msg(buf);
        delete[] buf;
        throw std::runtime_error(msg);
    }

    // Build a string with '0' and '1' for all remaining bits in the input
    std::string haystack;
    size_t cursor = s4_cursor;
    int pbyte     = this->pbyte;
    int pbyte_len = this->pbyte_len;
    while (true)
    {
        if (cursor == data_len)
            break;
        if (pbyte_len == 0)
        {
            pbyte_len = 8;
            pbyte     = data[cursor++];
        }
        haystack += (pbyte & 0x80) ? '1' : '0';
        pbyte <<= 1;
        --pbyte_len;
    }

    // Match the pattern
    regmatch_t matches[2];
    if (regexec(&compiled, haystack.c_str(), 2, matches, 0) != REG_NOMATCH)
    {
        fprintf(stderr, "'%s' found at position +%u\n", pattern,
                (unsigned)matches[0].rm_so);
    }
    else
    {
        fprintf(stderr, "'%s' not found\n", pattern);
    }
}

void Input::parse_error(const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    if (vasprintf(&message, fmt, ap) == -1)
        message = nullptr;
    va_end(ap);

    if (asprintf(&context, "%s:%zu+%u: %s", fname, start_offset, s4_cursor,
                 message ? message : fmt) == -1)
        context = nullptr;
    free(message);

    std::string msg(context ? context : fmt);
    free(context);

    throw error_parse(msg);
}

void Input::parse_error(unsigned pos, const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    if (vasprintf(&message, fmt, ap) == -1)
        message = nullptr;
    va_end(ap);

    if (asprintf(&context, "%s:%zu+%u: %s", fname, start_offset, pos,
                 message ? message : fmt) == -1)
        context = nullptr;
    free(message);

    std::string msg(context ? context : fmt);
    free(context);
    throw error_parse(msg);
}

void Input::parse_error(unsigned section, unsigned pos, const char* fmt,
                        ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    if (vasprintf(&message, fmt, ap) == -1)
        message = nullptr;
    va_end(ap);

    if (asprintf(&context, "%s:%zu+%u: %s (%ub inside section %s)", fname,
                 start_offset, sec[section] + pos, message ? message : fmt, pos,
                 bufr_sec_names[section]) == -1)
        context = nullptr;
    free(message);

    std::string msg(context ? context : fmt);
    free(context);
    throw error_parse(msg);
}

void Input::check_available_data(unsigned pos, size_t datalen,
                                 const char* expected)
{
    if (pos + datalen > data_len)
        parse_error(pos, "end of BUFR message while looking for %s", expected);
}

void Input::check_available_message_data(unsigned section, unsigned pos,
                                         size_t datalen, const char* expected)
{
    if (sec[section] + pos + datalen > data_len)
        parse_error(section, pos, "end of BUFR message while looking for %s",
                    expected);
}

void Input::check_available_section_data(unsigned section, unsigned pos,
                                         size_t datalen, const char* expected)
{
    if (section < 5)
    {
        if (sec[section] + pos + datalen > sec[section + 1])
            parse_error(section, pos,
                        "end of BUFR section while looking for %s", expected);
    }
    else
    {
        if (sec[section] + pos + datalen > data_len)
            parse_error(section, pos,
                        "end of BUFR message while looking for %s", expected);
    }
}

bool Input::decode_string(unsigned bit_len, char* str, size_t& len)
{
    int toread   = bit_len;
    bool missing = true;
    len          = 0;

    while (toread > 0)
    {
        int count       = toread > 8 ? 8 : toread;
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
        for (; len > 1 && (str[len - 1] == 0 || isspace(str[len - 1])); --len)
            str[len - 1] = 0;
    }

    return !missing;
}

void Input::decode_string(Var& dest)
{
    Varinfo info = dest.info();

    sys::TempBuffer str(info->bit_len / 8 + 2);
    size_t len;
    bool missing = !decode_string(info->bit_len, str, len);

    /* Store the variable that we found */
    // Set the variable value
    if (!missing)
        dest.setc(str);
}

void Input::decode_binary(Var& dest)
{
    Varinfo info = dest.info();

    sys::TempBuffer<unsigned char> buf(info->bit_len / 8 + 1);
    size_t len      = 0;
    unsigned toread = info->bit_len;
    bool missing    = true;

    while (toread > 0)
    {
        unsigned count  = toread > 8 ? 8 : toread;
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
        dest.setc((char*)(buf.data()));
}

void Input::decode_number(Var& dest)
{
    Varinfo info = dest.info();

    uint32_t val = get_bits(info->bit_len);

    // TRACE("datasec:decode_b_num:reading %s (%s), size %d, scale %d, starting
    // point %d\n", info->desc, info->bufr_unit, info->bit_len, info->scale,
    // val);

    // Check if there are bits which are not 1 (that is, if the value is
    // present) In case of delayed replications, there is no missing value
    bool missing = false;
    if (WR_VAR_X(info->code) == 31 && WR_VAR_F(info->code) == 0)
        switch (WR_VAR_Y(info->code))
        {
            case 0:
            case 1:
            case 2:
            case 11:
            case 12: break;
            default: missing = (val == all_ones(info->bit_len));
        }
    else
        missing = (val == all_ones(info->bit_len));

    // TRACE("datasec:decode_b_num:len %d val %d info-len %d info-desc %s\n",
    // info->bit_len, val, info->bit_len, info->desc);

    /* Store the variable that we found */
    if (missing)
    {
        /* Create the new Var */
        // TRACE("datasec:decode_b_num:decoded as missing\n");
        dest.unset();
    }
    else
    {
        double dval = info->decode_binary(val);
        // TRACE("datasec:decode_b_num:decoded as %f %s\n", dval,
        // info->bufr_unit);
        /* Create the new Var */
        dest.setd(dval);
    }
}

bool Input::decode_compressed_base(Varinfo info, uint32_t& base,
                                   uint32_t& diffbits)
{
    // Data field base value
    base = get_bits(info->bit_len);

    // Check if there are bits which are not 1 (that is, if the value is
    // present)
    bool missing = (base == all_ones(info->bit_len));

    // We just decoded the base value. Now we need to decode all the offsets

    // Decode the number of bits (encoded in 6 bits) that these difference
    // values occupy
    diffbits = get_bits(6);
    if (missing && diffbits != 0)
        error_consistency::throwf(
            "When decoding compressed BUFR data, the difference bit length "
            "must be 0 (and not %u like in this case) when the base value is "
            "missing",
            diffbits);

    TRACE("Input:decode_compressed_base:decoded base %u diffbits %u "
          "(%smissing) for %01d%02d%03d %s\n",
          (unsigned)base, (unsigned)diffbits, missing ? "" : "not ",
          WR_VAR_FXY(info->code), info->unit);

    return missing;
}

void Input::decode_compressed_number(Var& dest, uint32_t base,
                                     unsigned diffbits)
{
    Varinfo info = dest.info();

    // Decode the difference value
    uint32_t diff = get_bits(diffbits);

    // Check if it's all 1s: in that case it's a missing value
    if (base == all_ones(info->bit_len) || diff == all_ones(diffbits))
    {
        /* Missing value */
        // TRACE("datasec:decode_b_num:decoded[%d] as missing\n", i);
        dest.unset();
    }
    else
    {
        // Compute the value for this subset
        uint32_t newval = base + diff;
        if (newval == all_ones(info->bit_len))
            dest.unset();
        else
        {
            double dval = info->decode_binary(newval);
            TRACE("Input:decode_compressed_number:decoded diffbits %u "
                  "%u+%u=%u->%f %01d%02d%03d %s\n",
                  diffbits, base, diff, newval, dval, WR_VAR_FXY(info->code),
                  info->unit);

            /* Create the new Var */
            dest.setd(dval);
        }
    }
}

void Input::decode_compressed_number_af(
    Varinfo info, const bulletin::AssociatedField& associated_field,
    unsigned subsets, std::function<void(unsigned, Var&&)> dest)
{
    // debug_dump_next_bits("Input:decode_compressed_base:", 500,
    // {associated_field.bit_count, 6, info->bit_len, 6});

    /* I could not find any specification describing the behaviour of associated
     * fields in compressed BUFRs.
     *
     * By empirical observation, I assume this behaviour:
     *
     *  - $ASSOCIATED_FIELD_BITS bits of base value for the associated field
     *  - 6 bits specifying the number of bits used to encode the associated
     *    field difference values
     *  - base value for the actual variable (number of bits defined in table B)
     *  - 6 difference bits for the actual variable
     *
     * Then, for each subset:
     *  - the difference bits for the associated field
     *  - the difference bits for the actual variable
     */

    /// Associated field base value
    uint32_t af_base     = get_bits(associated_field.bit_count);
    /// Number of bits used to encode the associated field differences
    uint32_t af_diffbits = get_bits(6);

    if (af_base == all_ones(associated_field.bit_count))
    {
        // Associated field is missing
        decode_compressed_number(info, subsets, dest);
    }
    else
    {
        Var var(info);

        std::vector<std::unique_ptr<Var>> associated_fields(subsets);
        for (unsigned i = 0; i < subsets; ++i)
        {
            uint32_t diff  = get_bits(af_diffbits);
            uint32_t value = af_base + diff;
            TRACE("Input:decode_compressed_base:decoded af %ubits %u+%u=%u\n",
                  af_diffbits, af_base, diff, value);
            associated_fields[i] = associated_field.make_attribute(value);
        }

        decode_compressed_number(
            info, subsets, [&](unsigned subset, Var&& var) {
                if (associated_fields[subset].get())
                    var.seta(std::move(associated_fields[subset]));
                dest(subset, std::move(var));
            });
    }
}

void Input::decode_compressed_semantic_number(Var& dest, unsigned)
{
    Varinfo info = dest.info();

    uint32_t base = get_bits(info->bit_len);

    // TRACE("datasec:decode_b_num:reading %s (%s), size %d, scale %d, starting
    // point %d\n", info->desc, info->bufr_unit, info->bit_len, info->scale,
    // base);

    // Check if there are bits which are not 1 (that is, if the value is
    // present)
    bool missing = (base == all_ones(info->bit_len));

    // TRACE("datasec:decode_b_num:len %d base %d info-len %d info-desc %s\n",
    // info->bit_len, base, info->bit_len, info->desc);

    // Store the variable that we found

    /* If compression is in use, then we just decoded the base value. Now we
     * need to decode all the offsets. However, since this value cannot change
     * across subsets without breaking the alignment of the variables in the
     * various subsets, we only need to check that the 6-bits difference value
     * size is 0, and use the base value we just decoded as the final value */
    uint32_t diffbits = get_bits(6);
    if (diffbits)
        error_consistency::throwf(
            "cannot handle a semantic variable (like a repetition count) that "
            "differs across subsets");

    // TRACE("Compressed number, base value %d diff bits %d\n", base, diffbits);

    // Decode the destination variable
    if (missing)
    {
        // TRACE("datasec:decode_b_num:decoded[%d] as missing\n", i);
        dest.unset();
    }
    else
    {
        double dval = info->decode_binary(base);
        dest.setd(dval);
    }
}

void Input::decode_string(Varinfo info, unsigned subsets,
                          std::function<void(unsigned, Var&&)> dest)
{
    sys::TempBuffer str(info->bit_len / 8 + 2);
    size_t len;
    bool missing = !decode_string(info->bit_len, str, len);

    /* Store the variable that we found */

    /* If compression is in use, then we just decoded the base value.  Now
     * we need to decode all the offsets */

    /* Decode the number of bits (encoded in 6 bits) that these difference
     * values occupy */
    uint32_t diffbits = get_bits(6);

    // TRACE("datadesc:decode_b_string:compressed string, base:%.*s, diff bits
    // %d\n", (int)len, str, diffbits);

    if (diffbits != 0)
    {
        Var var(info);

        /* Let's also check that the number of
         * difference characters is the same length as
         * the reference string */
        if (diffbits * 8 > info->bit_len)
            error_unimplemented::throwf(
                "compressed strings with %u bits have %u bit deltas (deltas "
                "should not be longer than field)",
                info->bit_len, diffbits);

        for (unsigned i = 0; i < subsets; ++i)
        {
            // Set the variable value
            if (decode_string(diffbits * 8, str, len))
            {
                /* Compute the value for this subset */
                // TRACE("datadesc:decode_b_string:decoded[%d] as \"%s\"\n", i,
                // str);
                var.setc(str);
            }
            else
            {
                /* Missing value */
                // TRACE("datadesc:decode_b_string:decoded[%d] as missing\n",
                // i);
                var.unset();
            }

            // Add it to this subset
            dest(i, std::move(var));
        }
    }
    else
    {
        Var var(info);
        if (!missing)
            var.setc(str);

        // Add the string to all the subsets
        for (unsigned i = 0; i < subsets; ++i)
            dest(i, Var(var));
    }
}

void Input::decode_string(Var& dest, unsigned subsets)
{
    Varinfo info = dest.info();

    sys::TempBuffer str(info->bit_len / 8 + 2);
    size_t len;
    bool missing = !decode_string(info->bit_len, str, len);

    /* Store the variable that we found */

    /* If compression is in use, then we just decoded the base value.  Now
     * we need to decode all the offsets */

    /* Decode the number of bits (encoded in 6 bits) that these difference
     * values occupy */
    uint32_t diffbits = get_bits(6);

    // TRACE("datadesc:decode_b_string:compressed string, base:%.*s, diff bits
    // %d\n", (int)len, str, diffbits);

    if (diffbits == 0)
    {
        if (!missing)
            dest.setc(str);
    }
    else
    {
        /* Let's also check that the number of
         * difference characters is the same length as
         * the reference string */
        if (diffbits * 8 > info->bit_len)
            error_unimplemented::throwf(
                "compressed strings with %u bits have %u bit deltas (deltas "
                "should not be longer than field)",
                info->bit_len, diffbits);

        // Set the variable value
        if (decode_string(diffbits * 8, str, len))
        {
            /* Compute the value for this subset */
            // TRACE("datadesc:decode_b_string:decoded[%d] as \"%s\"\n", i,
            // str);
            dest.setc(str);
        }
        else
        {
            /* Missing value */
            // TRACE("datadesc:decode_b_string:decoded[%d] as missing\n", i);
            dest.unset();
        }

        Var copy(dest.info());
        for (unsigned i = 1; i < subsets; ++i)
        {
            // TODO: only compare the diffbits without needing to reconstruct
            // the var

            // Set the variable value
            if (decode_string(diffbits * 8, str, len))
            {
                /* Compute the value for this subset */
                // TRACE("datadesc:decode_b_string:decoded[%d] as \"%s\"\n", i,
                // str);
                copy.setc(str);
            }
            else
            {
                /* Missing value */
                // TRACE("datadesc:decode_b_string:decoded[%d] as missing\n",
                // i);
                copy.unset();
            }

            if (dest != copy)
            {
                std::string val1 = dest.format();
                std::string val2 = copy.format();
                error_consistency::throwf(
                    "When decoding %d%02d%03d from compressed BUFR data, "
                    "decoded values differ (%s != %s) but should all be the "
                    "same",
                    WR_VAR_F(dest.code()), WR_VAR_X(dest.code()),
                    WR_VAR_Y(dest.code()), val2.c_str(), val1.c_str());
            }
        }
    }
}

void Input::decode_compressed_number(Varinfo info, unsigned subsets,
                                     std::function<void(unsigned, Var&&)> dest)
{
    // Data field base value
    uint32_t base;

    // Number of bits used for each difference value
    uint32_t diffbits;

    bool missing = decode_compressed_base(info, base, diffbits);
    if (missing)
    {
        for (unsigned i = 0; i < subsets; ++i)
            dest(i, Var(info));
    }
    else if (!diffbits)
    {
        Var var(info, info->decode_binary(base));
        for (unsigned i = 0; i < subsets; ++i)
            dest(i, Var(var));
    }
    else
    {
        Var var(info);
        for (unsigned i = 0; i < subsets; ++i)
        {
            decode_compressed_number(var, base, diffbits);
            dest(i, std::move(var));
        }
    }
}

void Input::decode_string(Varinfo info, unsigned subsets,
                          DispatchToSubsets& dest)
{
    // Decode the base value
    sys::TempBuffer str(info->bit_len / 8 + 2);
    size_t len;
    bool missing = !decode_string(info->bit_len, str, len);

    // Decode the number of bits (encoded in 6 bits) for each difference
    // value
    uint32_t diffbits = get_bits(6);

    if (missing && diffbits == 0)
        dest.add_missing(info);
    else if (diffbits == 0)
    {
        // Add the same string to all the subsets
        dest.add_same(Var(info, str));
    }
    else
    {
        /* Let's also check that the number of
         * difference characters is the same length as
         * the reference string */
        if (diffbits * 8 > info->bit_len)
            error_unimplemented::throwf(
                "compressed strings with %u bits have %u bit deltas (deltas "
                "should not be longer than field)",
                info->bit_len, diffbits * 8);

        for (unsigned i = 0; i < subsets; ++i)
        {
            // Set the variable value
            if (decode_string(diffbits * 8, str, len))
            {
                // Compute the value for this subset
                dest.add_var(i, Var(info, str));
            }
            else
            {
                // Missing value
                dest.add_var(i, Var(info));
            }
        }
    }
}

void Input::decode_compressed_number(Varinfo info, unsigned subsets,
                                     DispatchToSubsets& dest)
{
    // Data field base value
    uint32_t base;

    // Number of bits used for each difference value
    uint32_t diffbits;

    bool missing = decode_compressed_base(info, base, diffbits);
    if (missing)
        dest.add_missing(info);
    else if (!diffbits)
        dest.add_same(Var(info, info->decode_binary(base)));
    else
    {
        Var var(info);
        for (unsigned i = 0; i < subsets; ++i)
        {
            decode_compressed_number(var, base, diffbits);
            dest.add_var(i, std::move(var));
        }
    }
}

std::string Input::decode_uncompressed_bitmap(unsigned size)
{
    std::string buf;
    buf.resize(size);
    for (unsigned i = 0; i < size; ++i)
    {
        uint32_t val = get_bits(1);
        buf[i]       = (val == 0) ? '+' : '-';
    }
    return buf;
}

std::string Input::decode_compressed_bitmap(unsigned size)
{
    std::string buf;
    buf.resize(size);
    for (unsigned i = 0; i < size; ++i)
    {
        uint32_t val = get_bits(1);
        buf[i]       = (val == 0) ? '+' : '-';
        // Decode the number of bits (encoded in 6 bits) of difference
        // values. It's odd to repeat this for each bit in the bitmap, but
        // that's how things are transmitted and it's somewhat consistent
        // with how data compression is specified
        val          = get_bits(6);
        // If compressed, ensure that the difference bits are 0 and they are
        // not trying to transmit odd things like delta bitmaps
        if (val != 0)
            parse_error("bitmap entry %u declares %u difference bits, but we "
                        "only support 0",
                        i, val);
    }
    return buf;
}

} // namespace bufr
} // namespace wreport
