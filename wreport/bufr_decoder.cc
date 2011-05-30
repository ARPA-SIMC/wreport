/*
 * wreport/bulletin - BUFR decoder
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

#include "config.h"

#include "opcode.h"
#include "bulletin.h"
#include "conv.h"

#include <stdio.h>
#include <netinet/in.h>

#include <stdlib.h>	/* malloc */
#include <ctype.h>	/* isspace */
#include <string.h>	/* memcpy */
#include <stdarg.h>	/* va_start, va_end */
#include <math.h>	/* NAN */
#include <time.h>
#include <errno.h>

#include <assert.h>

// #define TRACE_DECODER

#ifdef TRACE_DECODER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {
namespace {

// Unmarshal a big endian integer value n bytes long
static inline int readNumber(const unsigned char* buf, int bytes)
{
	int res = 0;
	int i;
	for (i = 0; i < bytes; ++i)
	{
		res <<= 8;
		res |= buf[i];
	}
	return res;
}

// Return a value with bitlen bits set to 1
static inline uint32_t all_ones(int bitlen)
{
	return ((1 << (bitlen - 1))-1) | (1 << (bitlen - 1));
}

/*
// Section names to used to format nice error messages
static const char* sec_names[] = {
    "indicator section", // Section 0
    "identification section", // Section 1
    "optional section", // Section 2
    "data descriptor section", // Section 3
    "data section", // Section 4
    "end section", // Section 5
};
*/

struct Input
{
    /* Input message data */
    const std::string& in;
    /* File name to use for error messages */
    const char* fname;
    /* File offset to use for error messages */
    size_t offset;
    /* Offsets of the start of BUFR sections */
    const unsigned char* sec[6];

    Input(const std::string& in, const char* fname, size_t offset)
        : in(in), fname(fname), offset(offset)
    {
        sec[0] = (const unsigned char*)in.data();
        for (int i = 1; i < 6; ++i)
            sec[i] = 0;
    }
    void parse_error(const unsigned char* pos, const char* fmt, ...) WREPORT_THROWF_ATTRS(3, 4)
    {
        char* context;
        char* message;

        va_list ap;
        va_start(ap, fmt);
        vasprintf(&message, fmt, ap);
        va_end(ap);

        asprintf(&context, "%s:%zd+%d: %s", fname, offset, (int)(pos - sec[0]), message);
        free(message);

        string msg(context);
        free(context);
        throw error_parse(msg);
    }

    void check_available_data(unsigned const char* start, size_t datalen, const char* next)
    {
        if (start + datalen > sec[0] + in.size())
            parse_error(start, "end of BUFR message while looking for %s", next);
        TRACE("check:%s starts at %d and contains at least %zd bytes\n", next, (int)(start - sec[0]), datalen);
    }

    void read_section_size(int num)
    {
        if (sec[num] + 3 > sec[0] + in.size())
            parse_error(sec[num], "end of BUFR message while looking for section %d (%s)", num, sec[num]);
        sec[num+1] = sec[num] + readNumber(sec[num], 3);
        if (sec[num+1] > sec[0] + in.size())
            parse_error(sec[num], "section %d (%s) claims to end past the end of the BUFR message",
                    num, sec[num]);
    }
};

struct Decoder
{
    /// Input data
    Input input;
    /* Output decoded variables */
    BufrBulletin& out;
    /// Number of expected subsets (read in decode_header, used in decode_data)
    size_t expected_subsets;

    Decoder(const std::string& in, const char* fname, size_t offset, BufrBulletin& out)
        : input(in, fname, offset), out(out)
    {
    }

    void decode_sec1ed3()
    {
        const unsigned char* sec1 = input.sec[1];
		// TODO: misses master table number in sec1[3]
		// Set length to 1 for now, will set the proper length later when we
		// parse the section itself
		out.optional_section_length = (sec1[7] & 0x80) ? 1 : 0;
		// subcentre in sec1[4]
		out.subcentre = (int)sec1[4];
		// centre in sec1[5]
		out.centre = (int)sec1[5];
		// Update sequence number sec1[6]
		out.update_sequence_number = sec1[6];
		out.master_table = sec1[10];
		out.local_table = sec1[11];
		out.type = (int)sec1[8];
		out.subtype = 255;
		out.localsubtype = (int)sec1[9];

		out.rep_year = (int)sec1[12];
		// Fix the century with a bit of euristics
		if (out.rep_year > 50)
			out.rep_year += 1900;
		else
			out.rep_year += 2000;
		out.rep_month = (int)sec1[13];
		out.rep_day = (int)sec1[14];
		out.rep_hour = (int)sec1[15];
		out.rep_minute = (int)sec1[16];
		if ((int)sec1[17] != 0)
			out.rep_year = (int)sec1[17] * 100 + (out.rep_year % 100);
	}

    void decode_sec1ed4()
    {
        const unsigned char* sec1 = input.sec[1];
		// TODO: misses master table number in sec1[3]
		// centre in sec1[4-5]
		out.centre = readNumber(sec1+4, 2);
		// subcentre in sec1[6-7]
		out.subcentre = readNumber(sec1+6, 2);
        // update sequence number sec1[8]
        out.update_sequence_number = sec1[8];
		// has_optional in sec1[9]
		// Set length to 1 for now, will set the proper length later when we
		// parse the section itself
		out.optional_section_length = (sec1[9] & 0x80) ? 1 : 0;
		// category in sec1[10]
		out.type = (int)sec1[10];
		// international data sub-category in sec1[11]
		out.subtype = (int)sec1[11];
		// local data sub-category in sec1[12]
		out.localsubtype = (int)sec1[12];
		// version number of master table in sec1[13]
		out.master_table = sec1[13];
		// version number of local table in sec1[14]
		out.local_table = sec1[14];
		// year in sec1[15-16]
		out.rep_year = readNumber(sec1 + 15, 2);
		// month in sec1[17]
		out.rep_month = (int)sec1[17];
		// day in sec1[18]
		out.rep_day = (int)sec1[18];
		// hour in sec1[19]
		out.rep_hour = (int)sec1[19];
		// minute in sec1[20]
		out.rep_minute = (int)sec1[20];
		// sec in sec1[21]
		out.rep_second = (int)sec1[21];
	}

    /* Decode the message header only */
    void decode_header()
    {
        int i;

        /* Read BUFR section 0 (Indicator section) */
        input.check_available_data(input.sec[0], 8, "section 0 of BUFR message (indicator section)");
        input.sec[1] = input.sec[0] + 8;
        if (memcmp(input.sec[0], "BUFR", 4) != 0)
            input.parse_error(input.sec[0], "data does not start with BUFR header (\"%.4s\" was read instead)", input.sec[0]);

        /* Check the BUFR edition number */
        out.edition = input.sec[0][7];
        if (out.edition != 2 && out.edition != 3 && out.edition != 4)
            input.parse_error(input.sec[0] + 7, "Only BUFR edition 3 and 4 are supported (this message is edition %d)", out.edition);

        /* Read bufr section 1 (Identification section) */
        input.check_available_data(input.sec[1], out.edition == 4 ? 22 : 18, "section 1 of BUFR message (identification section)");
        input.read_section_size(1);

		switch (out.edition)
		{
			case 2: decode_sec1ed3(); break;
			case 3: decode_sec1ed3(); break;
			case 4: decode_sec1ed4(); break;
			default:
				error_consistency::throwf("BUFR edition is %d, but I can only decode 2, 3 and 4", out.edition);
		}

		TRACE("info:ed %d opt %d upd %d origin %d.%d tables %d.%d type %d.%d %04d-%02d-%02d %02d:%02d\n", 
				out.edition, out.optional_section_length, out.update_sequence_number,
				out.centre, out.subcentre,
				out.master_table, out.local_table,
				out.type, out.subtype,
				out.rep_year, out.rep_month, out.rep_day, out.rep_hour, out.rep_minute);

        /* Read BUFR section 2 (Optional section) */
        if (out.optional_section_length)
        {
            input.read_section_size(2);
            out.optional_section_length = readNumber(input.sec[2], 3) - 4;
            out.optional_section = new char[out.optional_section_length];
            if (out.optional_section == NULL)
                throw error_alloc("allocating space for the optional section");
            memcpy(out.optional_section, input.sec[2] + 4, out.optional_section_length);
        } else
            input.sec[3] = input.sec[2];

        /* Read BUFR section 3 (Data description section) */
        input.check_available_data(input.sec[3], 8, "section 3 of BUFR message (data description section)");
        input.read_section_size(3);
        expected_subsets = readNumber(input.sec[3] + 4, 2);
        out.compression = (input.sec[3][6] & 0x40) ? 1 : 0;
        for (i = 0; i < (input.sec[4] - input.sec[3] - 7)/2; i++)
            out.datadesc.push_back((Varcode)readNumber(input.sec[3] + 7 + i * 2, 2));
        TRACE("info:s3length %d subsets %zd observed %d compression %d byte7 %x\n",
                (int)(input.sec[4] - input.sec[3]), expected_subsets, (input.sec[3][6] & 0x80) ? 1 : 0, out.compression, (unsigned int)input.sec[3][6]);
        /*
       IFTRACE{
       TRACE(" -> data descriptor section: ");
       bufrex_opcode_print(msg->datadesc, stderr);
       TRACE("\n");
       }
       */
    }

    /* Decode message data section after the header has been decoded */
    void decode_data();
};

struct DataSection;

struct Bitmap
{
    const BufrBulletin& out;
    /* Data present bitmap */
    char* bitmap;
    /* Length of data present bitmap */
    size_t len;
    /* Number of elements set to true in the bitmap */
    int count;
    /* Next bitmap element for which we decode values */
    int use_index;
    /* Next subset element for which we decode attributes */
    int subset_index;

    Bitmap(const BufrBulletin& out)
        : out(out), bitmap(0), count(0)
    {
    }
    ~Bitmap()
    {
        if (bitmap) delete[] bitmap;
    }

    void next(DataSection& ds);
};

/**
 * Variable consumer
 *
 * This provides a generic interface to which variables are sent after
 * decoding.
 */
struct VarAdder
{
    virtual ~VarAdder() {}

    /**
     * Produce a variable for the given subset.
     *
     * A subset number of -1 means 'the current subset' and is used when
     * decoding uncompressed BUFR messages
     */
    virtual void add_var(const Var&, int subset=-1) = 0;
};

struct VarIgnorer : public VarAdder
{
    virtual void add_var(const Var&, int subset=-1) {}
};

template<typename CLS>
struct VarAdderProxy : public VarAdder
{
    typedef void (CLS::*adder_meth)(const Var&, int);
    CLS& obj;
    adder_meth adder;

    VarAdderProxy(CLS& obj, adder_meth adder) : obj(obj), adder(adder) {}

    virtual void add_var(const Var& var, int subset=-1)
    {
        (obj.*adder)(var, subset);
    }
};

struct DataSection
{
    Input& input;

    /* Bit decoding data */
    size_t cursor;
    unsigned char pbyte;
    int pbyte_len;

    DataSection(Input& input)
        : input(input), cursor(input.sec[4] + 4 - input.sec[0]), pbyte(0), pbyte_len(0)
    {
    }
    virtual ~DataSection() {}

    /* Return the current decoding byte offset */
    int offset() const { return cursor; }

    /* Return the number of bits left in the message to be decoded */
    int bits_left() const { return (input.in.size() - cursor) * 8 + pbyte_len; }

    /**
     * Get the integer value of the next 'n' bits from the decode input
     * n must be <= 32.
     */
    uint32_t get_bits(int n)
    {
        uint32_t result = 0;

        if (cursor == input.in.size())
            parse_error("end of buffer while looking for %d bits of bit-packed data", n);

        for (int i = 0; i < n; i++) 
        {
            if (pbyte_len == 0) 
            {
                pbyte_len = 8;
                pbyte = input.sec[0][cursor++];
            }
            result <<= 1;
            if (pbyte & 0x80)
                result |= 1;
            pbyte <<= 1;
            pbyte_len--;
        }

        return result;
    }

    /* Dump 'count' bits of 'buf', starting at the 'ofs-th' bit */
    void dump_next_bits(int count, FILE* out)
    {
        size_t cursor = this->cursor;
        int pbyte = this->pbyte;
        int pbyte_len = this->pbyte_len;
        int i;

        for (i = 0; i < count; ++i) 
        {
            if (cursor == input.in.size())
                break;
            if (pbyte_len == 0) 
            {
                pbyte_len = 8;
                pbyte = input.sec[0][cursor++];
                putc(' ', out);
            }
            putc((pbyte & 0x80) ? '1' : '0', out);
            pbyte <<= 1;
            --pbyte_len;
        }
    }

    void parse_error(const char* fmt, ...) WREPORT_THROWF_ATTRS(2, 3)
    {
        char* context;
        char* message;

        va_list ap;
        va_start(ap, fmt);
        vasprintf(&message, fmt, ap);
        va_end(ap);

        asprintf(&context, "%s:%zd+%zd: %s", input.fname, input.offset, cursor, message);
        free(message);

        string msg(context);
        free(context);

        throw error_parse(msg);
    }

    void decode_b_value(Varinfo info, VarAdder& adder)
    {
        if (info->is_string())
            decode_b_string(info, adder);
        else
            decode_b_num(info, adder);
    }

    virtual void decode_b_num(Varinfo info, VarAdder& out)
    {
        /* Read a value */
        Var var(info);

        uint32_t val = get_bits(info->bit_len);

        TRACE("datasec:decode_b_num:reading %s (%s), size %d, scale %d, starting point %d\n", info->desc, info->bufr_unit, info->bit_len, info->scale, val);

        /* Check if there are bits which are not 1 (that is, if the value is present) */
        bool missing = (val == all_ones(info->bit_len));

        /*bufr_decoder_debug(decoder, "  %s: %d%s\n", info.desc, val, info.type);*/
        TRACE("datasec:decode_b_num:len %d val %d info-len %d info-desc %s\n", info->bit_len, val, info->bit_len, info->desc);

        /* Store the variable that we found */
        if (missing)
        {
            /* Create the new Var */
            TRACE("datasec:decode_b_num:decoded as missing\n");
        } else {
            double dval = info->bufr_decode_int(val);
            TRACE("datasec:decode_b_num:decoded as %f %s\n", dval, info->bufr_unit);
            /* Convert to target unit */
            dval = convert_units(info->bufr_unit, info->unit, dval);
            TRACE("datasec:decode_b_num:converted to %f %s\n", dval, info->unit);
            /* Create the new Var */
            var.setd(dval);
        }
        out.add_var(var);
    }

    /**
     * Read a string from the data section
     *
     * @param info
     *   Description of how the string is encoded
     * @param str
     *   Buffer where the string is written. Must be big enough to contain the
     *   longest string described by info, plus 2 bytes
     * @return
     *   true if we decoded a real string, false if we decoded a missing string
     *   value
     */
    bool read_base_string(Varinfo info, char* str, size_t& len)
    {
        int toread = info->bit_len;
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
            for (--len; len > 0 && isspace(str[len]);
                    len--)
                str[len] = 0;
        }

        TRACE("datasec:read_base_string:bufr_message_decode_b_data len %zd val %s missing %d info-len %d info-desc %s\n", len, str, (int)missing, info->bit_len, info->desc);

        return !missing;
    }

    virtual void decode_b_string(Varinfo info, VarAdder& adder)
    {
        /* Read a string */
        Var var(info);

        char str[info->bit_len / 8 + 2];
        size_t len;
        bool missing = !read_base_string(info, str, len);

        /* Store the variable that we found */
        // Set the variable value
        if (!missing)
            var.setc(str);
        adder.add_var(var);
    }

    /**
     * Decode a delayed replication factor, send the resulting variables to
     * \a adder and return the replication count
     */
    virtual int decode_delayed_replication_factor(Varinfo info, VarAdder& adder)
    {
        // Fetch the repetition count
        uint32_t count = get_bits(info->bit_len);
        //if (count == all_ones(info->bit_len))
        //{
        //    throw error_parse("Found MISSING in delayed replication factor");
        //    //TRACE("datasec:decode_delayed_replication_factor: found MISSING delayed replication factor (%d)\n", (int)count);
        //    //count = 0;
        //}

        /* Insert the repetition count among the parsed variables */
        adder.add_var(Var(info, (int)count));

        return count;
    }
};

struct CompressedDataSection : public DataSection
{
    unsigned subset_count;

    CompressedDataSection(Input& input, unsigned subset_count)
        : DataSection(input), subset_count(subset_count) {}

    virtual void decode_b_num(Varinfo info, VarAdder& out)
    {
        /* Read a value */
        Var var(info);

        uint32_t val = get_bits(info->bit_len);

        TRACE("datasec:decode_b_num:reading %s (%s), size %d, scale %d, starting point %d\n", info->desc, info->bufr_unit, info->bit_len, info->scale, val);

        /* Check if there are bits which are not 1 (that is, if the value is present) */
        bool missing = (val == all_ones(info->bit_len));

        /*bufr_decoder_debug(decoder, "  %s: %d%s\n", info.desc, val, info.type);*/
        TRACE("datasec:decode_b_num:len %d val %d info-len %d info-desc %s\n", info->bit_len, val, info->bit_len, info->desc);

        /* Store the variable that we found */

        /* If compression is in use, then we just decoded the base value.  Now
         * we need to decode all the offsets */

        /* Decode the number of bits (encoded in 6 bits) that these difference
         * values occupy */
        uint32_t diffbits = get_bits(6);
        if (missing && diffbits != 0)
            error_consistency::throwf("When decoding compressed BUFR data, the difference bit length must be 0 (and not %d like in this case) when the base value is missing", diffbits);

        TRACE("Compressed number, base value %d diff bits %d\n", val, diffbits);

        for (unsigned i = 0; i < subset_count; ++i)
        {
            /* Decode the difference value */
            uint32_t diff = get_bits(diffbits);

            /* Check if it's all 1: in that case it's a missing value */
            if (missing || diff == all_ones(diffbits))
            {
                /* Missing value */
                TRACE("datasec:decode_b_num:decoded[%d] as missing\n", i);
            } else {
                /* Compute the value for this subset */
                uint32_t newval = val + diff;
                double dval = info->bufr_decode_int(newval);
                TRACE("datasec:decode_b_num:decoded[%d] as %d+%d=%d->%f %s\n", i, val, diff, newval, dval, info->bufr_unit);

                /* Convert to target unit */
                dval = convert_units(info->bufr_unit, info->unit, dval);
                TRACE("datasec:decode_b_num:converted to %f %s\n", dval, info->unit);

                /* Create the new Var */
                var.setd(dval);
            }

            /* Add it to this subset */
            out.add_var(var, i);
        }
    }

    void decode_b_string(Varinfo info, VarAdder& adder)
    {
        /* Read a string */
        Var var(info);
        //size_t len = 0;

        char str[info->bit_len / 8 + 2];
        size_t len;
        bool missing = !read_base_string(info, str, len);

        /* Store the variable that we found */

        /* If compression is in use, then we just decoded the base value.  Now
         * we need to decode all the offsets */

        /* Decode the number of bits (encoded in 6 bits) that these difference
         * values occupy */
        uint32_t diffbits = get_bits(6);

        TRACE("datadesc:decode_b_string:compressed string, diff bits %d\n", diffbits);

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

            for (unsigned i = 0; i < subset_count; ++i)
            {
                unsigned j, missing = 1;

                /* Decode the difference value, reusing the str buffer */
                for (j = 0; j < diffbits; ++j)
                {
                    uint32_t bitval = get_bits(8);
                    /* Check that the string is not all 0xff, meaning missing value */
                    if (bitval != 0xff && bitval != 0)
                        missing = 0;
                    str[j] = bitval;
                }
                str[j] = 0;

                // Set the variable value
                if (missing)
                {
                    /* Missing value */
                    TRACE("datadesc:decode_b_string:decoded[%d] as missing\n", i);
                } else {
                    /* Convert space-padding into zero-padding */
                    for (--j; j > 0 && isspace(str[j]);
                            j--)
                        str[j] = 0;

                    /* Compute the value for this subset */
                    TRACE("datadesc:decode_b_string:decoded[%d] as \"%s\"\n", i, str);

                    var.setc(str);
                }

                /* Add it to this subset */
                adder.add_var(var, i);
            }
        } else {
            /* Add the string to all the subsets */
            for (unsigned i = 0; i < subset_count; ++i)
            {
                // Set the variable value
                if (!missing) var.setc(str);

                adder.add_var(var, i);
            }
        }
    }

    /**
     * Decode a delayed replication factor, send the resulting variables to
     * \a adder and return the replication count
     */
    int decode_delayed_replication_factor(Varinfo info, VarAdder& adder)
    {
        // Fetch the repetition count
        int count = get_bits(info->bit_len);

        /* Insert the repetition count among the parsed variables */

        /* If compression is in use, then we just decoded the base value.  Now
         * we need to decode all the repetition factors and see
         * that they are the same */

        /* Decode the number of bits (encoded in 6 bits) that these difference
         * values occupy */
        uint32_t diffbits = get_bits(6);

        TRACE("datadesc:decode_delayed_replication_factor:compressed delayed repetition, base value %d diff bits %d\n", count, diffbits);

        uint32_t repval = 0;
        for (unsigned i = 0; i < subset_count; ++i)
        {
            /* Decode the difference value */
            uint32_t diff = get_bits(diffbits);

            /* Compute the value for this subset */
            uint32_t newval = count + diff;
            TRACE("datadesc:decode_delayed_replication_factor:decoded[%d] as %d+%d=%d\n", i, count, diff, newval);

            if (i == 0)
                repval = newval;
            else if (repval != newval)
                parse_error("compressed delayed replication factor has different values for subsets (%d and %d)", repval, newval);

            adder.add_var(Var(info, (int)newval), i);
        }

        return count;
    }
};

void Bitmap::next(DataSection& ds)
{
    if (bitmap == 0)
        ds.parse_error("applying a data present bitmap with no current bitmap");
    TRACE("bitmap:next:pre %d %d %zd\n", use_index, subset_index, len);
    if (out.subsets.size() == 0)
        ds.parse_error("no subsets created yet, but already applying a data present bitmap");
    ++use_index;
    ++subset_index;
    while (use_index < 0 || (
                (unsigned)use_index < len &&
                bitmap[use_index] == '-'))
    {
        TRACE("bitmap:next:INCR\n");
        ++use_index;
        ++subset_index;
        while ((unsigned)subset_index < out.subsets[0].size() &&
                WR_VAR_F(out.subsets[0][subset_index].code()) != 0)
            ++subset_index;
    }
    if ((unsigned)use_index > len)
        ds.parse_error("moved past end of data present bitmap");
    if ((unsigned)subset_index == out.subsets[0].size())
        ds.parse_error("end of data reached when applying attributes");
    TRACE("bitmap:next:post %d %d\n", use_index, subset_index);
}

struct opcode_interpreter
{
    struct AttrMode
    {
        opcode_interpreter& i;
        AttrMode(opcode_interpreter& i) : i(i)
        {
            i.set_attr_mode();
        }
        ~AttrMode()
        {
            i.set_normal_mode();
        }
    };
    struct SubstMode
    {
        opcode_interpreter& i;
        SubstMode(opcode_interpreter& i) : i(i)
        {
            i.set_subst_mode();
        }
        ~SubstMode()
        {
            i.set_normal_mode();
        }
    };

    Decoder& d;
    DataSection& ds;
    Bitmap bitmap;
    /// Currently active variable destination
    VarAdder* current_adder;
    //VarAdder* attr_adder;

    /* Current value of scale change from C modifier */
    int c_scale_change;
    /* Current value of width change from C modifier */
    int c_width_change;
    /** Current value of string length override from C08 modifiers (0 for no
     * override)
     */
    int c_string_len_override;
    /**
     * Number of extra bits inserted by the current C04yyy modifier (0 for no
     * C04yyy operator in use)
     */
    int c04_bits;

    opcode_interpreter(Decoder& d, DataSection& ds)
        : d(d), ds(ds), bitmap(d.out), current_adder(0),
        c_scale_change(0), c_width_change(0), c_string_len_override(0), c04_bits(0)
    {
    }

    ~opcode_interpreter()
    {
    }

    virtual void set_normal_mode() = 0;
    virtual void set_attr_mode() = 0;
    virtual void set_subst_mode() = 0;

    /**
     * Get the Varinfo needed to decode \a code, applying C operator changes if
     * any are active.
     */
    Varinfo get_info(Varcode code)
    {
        Varinfo peek = d.out.btable->query(code);

        if (!c_scale_change && !c_width_change && !c_string_len_override)
            return peek;

        int scale = peek->scale;
        if (c_scale_change)
        {
            TRACE("get_info:applying %d scale change\n", c_scale_change);
            scale += c_scale_change;
        }
        int bit_len = peek->bit_len;

        if (peek->is_string() && c_string_len_override)
        {
            TRACE("get_info:overriding string to %d bytes\n", c_string_len_override);
            bit_len = c_string_len_override * 8;
        }
        else if (c_width_change)
        {
            TRACE("get_info:applying %d width change\n", c_width_change);
            bit_len += c_width_change;
        }

        TRACE("get_info:requesting alteration scale:%d, bit_len:%d\n", scale, bit_len);
        return d.out.btable->query_altered(code, scale, bit_len);
    }

    unsigned decode_b_data(const Opcodes& ops)
    {
        IFTRACE {
            TRACE("decode_b_data:items: ");
            ops.print(stderr);
            TRACE("\n");
        }

        Varinfo info = get_info(ops.head());

        IFTRACE {
            TRACE("decode_b_data:parsing @%zd+%d [bl %d sc %d ref %d]: %d%02d%03d %s[%s]\n",
                    ds.cursor, 8-ds.pbyte_len,
                    info->bit_len, info->scale, info->bit_ref,
                    WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var),
                    info->desc, info->unit);
            ds.dump_next_bits(64, stderr);
            TRACE("\n");
        }

        if (WR_VAR_X(info->var) == 33 && bitmap.bitmap)
        {
            if (c04_bits)
                throw error_unimplemented("C04 modifiers in B33yyy attributes are not supported");
            bitmap.next(ds);

            /* Get the real datum */
            AttrMode am(*this);
            ds.decode_b_value(info, *current_adder);
        } else {
            /* Get the real datum */
            if (c04_bits)
            {
                TRACE("decode_b_data:reading %d bits of C04 information\n", c04_bits);
                uint32_t val = ds.get_bits(c04_bits);
                TRACE("decode_b_data:read C04 information %x\n", val);
                // TODO: use the result
                val = val;
            }
            ds.decode_b_value(info, *current_adder);
        }

        return 1;
    }

    unsigned decode_bitmap(const Opcodes& ops, Varcode code, VarAdder& adder);

    /**
     * Decode instant or delayed replication information.
     *
     * In case of delayed replication, store a variable in the subset(s)
     * with the repetition count.
     */
    unsigned decode_replication_info(const Opcodes& ops, int& group, int& count, VarAdder& adder)
    {
        unsigned used = 1;
        group = WR_VAR_X(ops.head());
        count = WR_VAR_Y(ops.head());

        if (count == 0)
        {
            // Delayed replication

            // We also use the delayed replication factor opcode
            ++used;

            Varcode rep_op = ops[1];

            // Fetch the repetition count
            Varinfo rep_info = d.out.btable->query(rep_op);
            count = ds.decode_delayed_replication_factor(rep_info, adder);

            TRACE("decode_replication_info:%d items %d times (delayed)\n", group, count);
        } else
            TRACE("decode_replication_info:%d items %d times\n", group, count);

        return used;
    }

    unsigned decode_r_data(const Opcodes& ops)
    {
        // Read replication information
        int group, count;
        unsigned first;
        first = decode_replication_info(ops, group, count, *current_adder);

        TRACE("decode_r_data:%01d%02d%03d %d %d\n", 
                WR_VAR_F(ops.head()), WR_VAR_X(ops.head()), WR_VAR_Y(ops.head()), group, count);

        // Extract the first `group' nodes, to handle here
        Opcodes group_ops = ops.sub(first, group);

        // decode_data_section on it `count' times
        for (int i = 0; i < count; ++i)
            decode_data_section(group_ops);

        // Number of items processed
        return first + group;
    }

	unsigned decode_c_data(const Opcodes& ops);

	/* Run the opcode interpreter to decode the data section */
	void decode_data_section(const Opcodes& ops)
	{
		/*
		fprintf(stderr, "read_data: ");
		bufrex_opcode_print(ops, stderr);
		fprintf(stderr, "\n");
		*/
		TRACE("decode_data_section:START\n");

		for (unsigned i = 0; i < ops.size(); )
		{
			IFTRACE{
				TRACE("decode_data_section:pos %zd/%zd TODO: ", ds.cursor, ds.input.in.size());
				ops.sub(i).print(stderr);
				TRACE("\n");
			}

			switch (WR_VAR_F(ops[i]))
			{
				case 0: i += decode_b_data(ops.sub(i)); break;
				case 1: i += decode_r_data(ops.sub(i)); break;
				case 2: i += decode_c_data(ops.sub(i)); break;
				case 3:
				{
					Opcodes exp = d.out.dtable->query(ops[i]);
					decode_data_section(exp);
					++i;
					break;
				}
				default:
					ds.parse_error("cannot handle field %01d%02d%03d",
								WR_VAR_F(ops[i]),
								WR_VAR_X(ops[i]),
								WR_VAR_Y(ops[i]));
			}
		}
	}

    virtual void run() = 0;
};

struct opcode_interpreter_plain : public opcode_interpreter
{
    Subset* current_subset;
    VarAdderProxy<opcode_interpreter_plain> adder;

    opcode_interpreter_plain(Decoder& d, DataSection& ds)
        : opcode_interpreter(d, ds),
          current_subset(0),
          adder(*this, &opcode_interpreter_plain::add_var)
    {
        current_adder = &adder;
    }

    virtual void set_normal_mode()
    {
        adder.adder = &opcode_interpreter_plain::add_var;
    }
    virtual void set_attr_mode()
    {
        adder.adder = &opcode_interpreter_plain::add_attr;
    }
    virtual void set_subst_mode()
    {
        adder.adder = &opcode_interpreter_plain::add_subst;
    }

    void add_var(const Var& var, int subset=-1)
    {
        TRACE("bulletin:adding var %01d%02d%03d %s to current subset\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value());
        current_subset->store_variable(var);
    }

    void add_attr(const Var& var, int subset=-1)
    {
        TRACE("bulletin:adding var %01d%02d%03d %s as attribute to %01d%02d%03d bsi %d/%zd\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F((*current_subset)[bitmap.subset_index].code()),
                WR_VAR_X((*current_subset)[bitmap.subset_index].code()),
                WR_VAR_Y((*current_subset)[bitmap.subset_index].code()),
                bitmap.subset_index, current_subset->size());
        (*current_subset)[bitmap.subset_index].seta(var);
    }

    void add_subst(const Var& var, int subset=-1)
    {
        TRACE("bulletin:adding substitute value %01d%02d%03d %s as attribute to %01d%02d%03d bsi %d/%zd\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F((*current_subset)[bitmap.subset_index].code()),
                WR_VAR_X((*current_subset)[bitmap.subset_index].code()),
                WR_VAR_Y((*current_subset)[bitmap.subset_index].code()),
                bitmap.subset_index, current_subset->size());
        (*current_subset)[bitmap.subset_index].seta(var);
    }

    virtual void run()
    {
        /* Iterate on the number of subgroups */
        for (size_t i = 0; i < d.out.subsets.size(); ++i)
        {
            current_subset = &d.out.obtain_subset(i);
            decode_data_section(Opcodes(d.out.datadesc));
        }

        IFTRACE {
            if (ds.bits_left() > 32)
            {
                fprintf(stderr, "The data section of %s:%zd still contains %d unparsed bits\n",
                        d.input.fname, d.input.offset, ds.bits_left() - 32);
                /*
                   err = dba_error_parse(msg->file->name, POS + vec->cursor,
                   "the data section still contains %d unparsed bits",
                   bitvec_bits_left(vec));
                   goto fail;
                   */
            }
        }
    }
};

struct opcode_interpreter_compressed : public opcode_interpreter
{
    VarAdderProxy<opcode_interpreter_compressed> adder;

    opcode_interpreter_compressed(Decoder& d, CompressedDataSection& ds)
        : opcode_interpreter(d, ds),
          adder(*this, &opcode_interpreter_compressed::add_var)
    {
        current_adder = &adder;
    }

    virtual void set_normal_mode()
    {
        adder.adder = &opcode_interpreter_compressed::add_var;
    }
    virtual void set_attr_mode()
    {
        adder.adder = &opcode_interpreter_compressed::add_attr;
    }
    virtual void set_subst_mode()
    {
        adder.adder = &opcode_interpreter_compressed::add_subst;
    }

    void add_var(const Var& var, int subset)
    {
        TRACE("bulletin:adding var %01d%02d%03d %s to subset %d\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(), subset);
        d.out.subsets[subset].store_variable(var);
    }

    void add_attr(const Var& var, int subset)
    {
        TRACE("bulletin:adding var %01d%02d%03d %s as attribute to %01d%02d%03d bsi %d/%zd\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F(d.out.subsets[subset][bitmap.subset_index].code()),
                WR_VAR_X(d.out.subsets[subset][bitmap.subset_index].code()),
                WR_VAR_Y(d.out.subsets[subset][bitmap.subset_index].code()),
                bitmap.subset_index, d.out.subsets[subset].size());
        d.out.subsets[subset][bitmap.subset_index].seta(var);
    }

    void add_subst(const Var& var, int subset=-1)
    {
        TRACE("bulletin:adding substitute var %01d%02d%03d %s as attribute to %01d%02d%03d bsi %d/%zd\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F(d.out.subsets[subset][bitmap.subset_index].code()),
                WR_VAR_X(d.out.subsets[subset][bitmap.subset_index].code()),
                WR_VAR_Y(d.out.subsets[subset][bitmap.subset_index].code()),
                bitmap.subset_index, d.out.subsets[subset].size());
        d.out.subsets[subset][bitmap.subset_index].seta(var);
    }

    virtual void run()
    {
        /* Only needs to parse once */
        decode_data_section(Opcodes(d.out.datadesc));

        IFTRACE {
            if (ds.bits_left() > 32)
            {
                fprintf(stderr, "The data section of %s:%zd still contains %d unparsed bits\n",
                        d.input.fname, d.input.offset, ds.bits_left() - 32);
                /*
                   err = dba_error_parse(msg->file->name, POS + vec->cursor,
                   "the data section still contains %d unparsed bits",
                   bitvec_bits_left(vec));
                   goto fail;
                   */
            }
        }
    }
};

void Decoder::decode_data()
{
    // Once we filled the Bulletin header info, load decoding tables and allocate subsets
    out.load_tables();
    out.obtain_subset(expected_subsets - 1);

    /* Read BUFR section 4 (Data section) */
    input.read_section_size(4);
    TRACE("decode_data:section 4 is %d bytes long (%02x %02x %02x %02x)\n", readNumber(input.sec[4], 3),
            (unsigned int)*(input.sec[4]),
            (unsigned int)*(input.sec[4]+1),
            (unsigned int)*(input.sec[4]+2),
            (unsigned int)*(input.sec[4]+3));

    if (out.compression)
    {
        CompressedDataSection ds(input, out.subsets.size());
        opcode_interpreter_compressed interpreter(*this, ds);
        interpreter.run();
    } else {
        DataSection ds(input);
        opcode_interpreter_plain interpreter(*this, ds);
        interpreter.run();
    }

	/* Read BUFR section 5 (Data section) */
	input.check_available_data(input.sec[5], 4, "section 5 of BUFR message (end section)");

	if (memcmp(input.sec[5], "7777", 4) != 0)
		input.parse_error(input.sec[5], "section 5 does not contain '7777'");

#if 0
	for (i = 0; i < out.subsets; ++i)
	{
		bufrex_subset subset;
		DBA_RUN_OR_RETURN(bufrex_msg_get_subset(out, i, &subset));
		/* Copy the decoded attributes into the decoded variables */
		DBA_RUN_OR_RETURN(bufrex_subset_apply_attributes(subset));
	}
#endif
    //if (subsets_no != out.subsets.size())
    //    parse_error(sec5, "header advertised %u subsets but only %zd found", subsets_no, out.subsets.size());
}

}


void BufrBulletin::decode_header(const std::string& buf, const char* fname, size_t offset)
{
	clear();
	Decoder d(buf, fname, offset, *this);
	d.decode_header();
}

void BufrBulletin::decode(const std::string& buf, const char* fname, size_t offset)
{
	clear();
	Decoder d(buf, fname, offset, *this);
	d.decode_header();
	d.decode_data();
}


unsigned opcode_interpreter::decode_bitmap(const Opcodes& ops, Varcode code, VarAdder& adder)
{
	if (bitmap.bitmap) delete[] bitmap.bitmap;
	bitmap.bitmap = 0;

    int group;
    int count;
    VarIgnorer ignorer;
    unsigned used = decode_replication_info(ops, group, count, ignorer);

	// Sanity checks

	if (group != 1)
		ds.parse_error("bitmap section replicates %d descriptors instead of one", group);

	if (used >= ops.size())
		ds.parse_error("there are no descriptor after bitmap replicator (expected B31031)");

	if (ops[used] != WR_VAR(0, 31, 31))
		ds.parse_error("bitmap element descriptor is %02d%02d%03d instead of B31031",
				WR_VAR_F(ops[used]), WR_VAR_X(ops[used]), WR_VAR_Y(ops[used]));

	// If compressed, ensure that the difference bits are 0 and they are
	// not trying to transmit odd things like delta bitmaps 
	if (d.out.compression)
	{
		/* Decode the number of bits (encoded in 6 bits) that these difference
		 * values occupy */
		uint32_t diffbits = ds.get_bits(6);
		if (diffbits != 0)
			ds.parse_error("bitmap declares %d difference bits per bitmap value, but we only support 0", diffbits);
	}

	// Consume the data present indicator from the opcodes to process
	++used;

	// Bitmap size is now in count

	// Read the bitmap
	bitmap.count = 0;
	bitmap.bitmap = new char[count + 1];
	for (int i = 0; i < count; ++i)
	{
		uint32_t val = ds.get_bits(1);
		bitmap.bitmap[i] = (val == 0) ? '+' : '-';
		if (val == 0) ++bitmap.count;
	}
	bitmap.bitmap[count] = 0;
	bitmap.len = count;

	// Create a single use varinfo to store the bitmap
	MutableVarinfo info(MutableVarinfo::create_singleuse());
	info->set_string(code, "DATA PRESENT BITMAP", count);

	// Store the bitmap
	Var bmp(info, bitmap.bitmap);

    // Add var to subset(s)
    if (d.out.compression)
    {
        for (unsigned i = 0; i < d.out.subsets.size(); ++i)
            adder.add_var(bmp, i);
    } else {
        adder.add_var(bmp);
    }

	// Bitmap will stay set as a reference to the variable to use as the
	// current bitmap. The subset(s) are taking care of memory managing it.

	IFTRACE {
		TRACE("Decoded bitmap count %d: ", bitmap.count);
		for (unsigned i = 0; i < bitmap.len; ++i)
			TRACE("%c", bitmap.bitmap[i]);
		TRACE("\n");
	}

    // Move to first bitmap use index
    bitmap.use_index = -1;
    bitmap.subset_index = -1;

    return used;
}

unsigned opcode_interpreter::decode_c_data(const Opcodes& ops)
{
	unsigned used = 1;
	
	Varcode code = ops.head();

	TRACE("decode_c_data:%01d%02d%03d\n", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));

	switch (WR_VAR_X(code))
	{
		case 1:
			c_width_change = WR_VAR_Y(code) == 0 ? 0 : WR_VAR_Y(code) - 128;
			break;
		case 2:
			c_scale_change = WR_VAR_Y(code) == 0 ? 0 : WR_VAR_Y(code) - 128;
			break;
        case 4:
            // FIXME: nested C04 modifiers are not currently implemented
            if (WR_VAR_Y(code) && c04_bits)
                throw error_unimplemented("nested C04 modifiers are not yet implemented");
            if (WR_VAR_Y(code) > 32)
                error_unimplemented::throwf("C04 modifier wants %d bits but only at most 32 are supported", WR_VAR_Y(code));
            if (WR_VAR_Y(code))
            {
                // TODO: Read B31021
                used += decode_b_data(ops.sub(1));
            }
            c04_bits = WR_VAR_Y(code);
            break;
		case 5: {
			int cdatalen = WR_VAR_Y(code);
			char buf[cdatalen + 1];
			TRACE("decode_c_data:character data %d long\n", cdatalen);
			int i;
			for (i = 0; i < cdatalen; ++i)
			{
				uint32_t bitval = ds.get_bits(8);
				TRACE("decode_c_data:decoded character %d %c\n", (int)bitval, (char)bitval);
				buf[i] = bitval;
			}
			buf[i] = 0;

            // Add as C variable to the subset

            // Create a single use varinfo to store the bitmap
            MutableVarinfo info(MutableVarinfo::create_singleuse());
            info->set_string(code, "CHARACTER DATA", cdatalen);

            // Store the bitmap
            Var cdata(info, buf);

            // TODO: if compressed, extract the data from each subset? Store it in each dataset?
            if (d.out.compression)
                error_unimplemented::throwf("C05%03d character data found in compressed message and it is not clear how it should be handled", cdatalen);
            else
                current_adder->add_var(cdata);

            TRACE("decode_c_data:decoded string %s\n", buf);
            break;
        }
        case 8: {
            int cdatalen = WR_VAR_Y(code);
            IFTRACE {
                if (cdatalen)
                    TRACE("decode_c_data:character size overridden to %d chars for all fields\n", cdatalen);
                else
                    TRACE("decode_c_data:character size overridde end\n");
            }
            c_string_len_override = cdatalen;
            break;
        }
		case 22:
			if (WR_VAR_Y(code) == 0)
			{
				used += decode_bitmap(ops.sub(1), code, *current_adder);
			} else
				ds.parse_error("decode_c_data:C modifier %d%02d%03d not yet supported",
							WR_VAR_F(code),
							WR_VAR_X(code),
							WR_VAR_Y(code));
			break;
        case 23:
            if (WR_VAR_Y(code) == 0)
            {
                used += decode_bitmap(ops.sub(1), code, *current_adder);
            } else if (WR_VAR_Y(code) == 255) {
                if (!bitmap.bitmap)
                    ds.parse_error("C23255 found but there is no active bitmap");
                bitmap.next(ds);
                // Read substituted value

                // Use the details of the corrisponding variable for decoding
                Varinfo info = d.out.subsets[0][bitmap.subset_index].info();
                // Switch to 'add substituted value as attribute' mode
                SubstMode sm(*this);
                // Decode the value
                ds.decode_b_value(info, *current_adder);
            } else
                ds.parse_error("decode_c_data:C modifier %d%02d%03d not yet supported",
                        WR_VAR_F(code),
                        WR_VAR_X(code),
                        WR_VAR_Y(code));
            break;
		case 24:
			if (WR_VAR_Y(code) == 0)
			{
				used += decode_r_data(ops.sub(1));
			} else
				ds.parse_error("decode_c_data:C modifier %d%02d%03d not yet supported",
							WR_VAR_F(code),
							WR_VAR_X(code),
							WR_VAR_Y(code));
			break;
		default:
			ds.parse_error("decode_c_data:C modifiers (%d%02d%03d in this case) are not yet supported",
						WR_VAR_F(code),
						WR_VAR_X(code),
						WR_VAR_Y(code));
	}

	return used;
}

}
/* vim:set ts=4 sw=4: */
