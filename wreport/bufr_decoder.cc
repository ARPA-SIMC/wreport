/*
 * wreport/bulletin - BUFR decoder
 *
 * Copyright (C) 2005--2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#include "bulletin/internals.h"
#include "bulletin/buffers.h"
#include "conv.h"
#include "notes.h"

#include <stdio.h>
#include <netinet/in.h>

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cstdarg>
#include <cmath>
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

// Return a value with bitlen bits set to 1
static inline uint32_t all_ones(int bitlen)
{
    return ((1 << (bitlen - 1))-1) | (1 << (bitlen - 1));
}

struct Decoder
{
    /// Input data
    bulletin::BufrInput& in;
    /* Output decoded variables */
    BufrBulletin& out;
    /// Number of expected subsets (read in decode_header, used in decode_data)
    size_t expected_subsets;
    /// True if undefined attributes are added to the output, else false
    bool conf_add_undef_attrs;

    Decoder(const std::string& buf, const char* fname, size_t offset, BufrBulletin& out)
        : in(out.reset_raw_details(buf)), out(out),
          conf_add_undef_attrs(false)
    {
        if (out.codec_options)
        {
            conf_add_undef_attrs = out.codec_options->decode_adds_undef_attrs;
        }
        in.fname = fname;
        in.start_offset = offset;
    }

    void decode_sec1ed3()
    {
        // master table number in sec1[3]
        out.master_table_number = in.read_byte(1, 3);
        // has_optional in sec1[7]
        // Once we know if the optional section is available, we can scan
        // section lengths for the rest of the message
        in.scan_other_sections(in.read_byte(1, 7) & 0x80);
        out.optional_section_length = in.sec[3] - in.sec[2];
        if (out.optional_section_length)
            out.optional_section_length -= 4;
        // subcentre in sec1[4]
        out.subcentre = in.read_byte(1, 4);
        // centre in sec1[5]
        out.centre = in.read_byte(1, 5);
        // Update sequence number sec1[6]
        out.update_sequence_number = in.read_byte(1, 6);
        out.master_table = in.read_byte(1, 10);
        out.local_table = in.read_byte(1, 11);
        out.type = in.read_byte(1, 8);
        out.subtype = 255;
        out.localsubtype = in.read_byte(1, 9);

        out.rep_year = in.read_byte(1, 12);
        // Fix the century with a bit of euristics
        if (out.rep_year > 50)
            out.rep_year += 1900;
        else
            out.rep_year += 2000;
        out.rep_month = in.read_byte(1, 13);
        out.rep_day = in.read_byte(1, 14);
        out.rep_hour = in.read_byte(1, 15);
        out.rep_minute = in.read_byte(1, 16);
        if (in.read_byte(1, 17) != 0)
            out.rep_year = in.read_byte(1, 17) * 100 + (out.rep_year % 100);
    }

    void decode_sec1ed4()
    {
        // master table number in sec1[3]
        out.master_table_number = in.read_byte(1, 3);
        // centre in sec1[4-5]
        out.centre = in.read_number(1, 4, 2);
        // subcentre in sec1[6-7]
        out.subcentre = in.read_number(1, 6, 2);
        // update sequence number sec1[8]
        out.update_sequence_number = in.read_byte(1, 8);
        // has_optional in sec1[9]
        // Once we know if the optional section is available, we can scan
        // section lengths for the rest of the message
        in.scan_other_sections(in.read_byte(1, 9) & 0x80);
        out.optional_section_length = in.sec[3] - in.sec[2];
        if (out.optional_section_length)
            out.optional_section_length -= 4;
        // category in sec1[10]
        out.type = in.read_byte(1, 10);
        // international data sub-category in sec1[11]
        out.subtype = in.read_byte(1, 11);
        // local data sub-category in sec1[12]
        out.localsubtype = in.read_byte(1, 12);
        // version number of master table in sec1[13]
        out.master_table = in.read_byte(1, 13);
        // version number of local table in sec1[14]
        out.local_table = in.read_byte(1, 14);
        // year in sec1[15-16]
        out.rep_year = in.read_number(1, 15, 2);
        // month in sec1[17]
        out.rep_month = in.read_byte(1, 17);
        // day in sec1[18]
        out.rep_day = in.read_byte(1, 18);
        // hour in sec1[19]
        out.rep_hour = in.read_byte(1, 19);
        // minute in sec1[20]
        out.rep_minute = in.read_byte(1, 20);
        // sec in sec1[21]
        out.rep_second = in.read_byte(1, 21);
    }

    /* Decode the message header only */
    void decode_header()
    {
        // Read BUFR section 0 (Indicator section)
        if (memcmp(in.data + in.sec[0], "BUFR", 4) != 0)
            in.parse_error(0, 0, "data does not start with BUFR header (\"%.4s\" was read instead)", in.data + in.sec[0]);

        // Check the BUFR edition number
        out.edition = in.read_byte(0, 7);
        if (out.edition != 2 && out.edition != 3 && out.edition != 4)
            in.parse_error(0, 7, "Only BUFR edition 3 and 4 are supported (this message is edition %d)", out.edition);

        // Looks like a BUFR, scan section starts
        in.scan_lead_sections();

        // Read bufr section 1 (Identification section)
        in.check_available_data(1, 0, out.edition == 4 ? 22 : 18, "section 1 of BUFR message (identification section)");

        switch (out.edition)
        {
            case 2: decode_sec1ed3(); break;
            case 3: decode_sec1ed3(); break;
            case 4: decode_sec1ed4(); break;
            default:
                error_consistency::throwf("BUFR edition is %d, but I can only decode 2, 3 and 4", out.edition);
        }

        TRACE("BUFR:edition %d, optional section %db, update sequence number %d\n",
                out.edition, out.optional_section_length, out.update_sequence_number);
        TRACE("     origin %d.%d tables %d.%d type %d.%d %04d-%02d-%02d %02d:%02d\n",
                out.centre, out.subcentre,
                out.master_table, out.local_table,
                out.type, out.subtype,
                out.rep_year, out.rep_month, out.rep_day, out.rep_hour, out.rep_minute);

        /* Read BUFR section 2 (Optional section) */
        if (out.optional_section_length > 0)
        {
            out.optional_section_length = in.read_number(2, 0, 3) - 4;
            out.optional_section = new char[out.optional_section_length];
            if (out.optional_section == NULL)
                throw error_alloc("allocating space for the optional section");
            memcpy(out.optional_section, in.data + in.sec[2] + 4, out.optional_section_length);
        }

        /* Read BUFR section 3 (Data description section) */
        in.check_available_data(3, 0, 8, "section 3 of BUFR message (data description section)");
        expected_subsets = in.read_number(3, 4, 2);
        out.compression = (in.read_byte(3, 6) & 0x40) ? 1 : 0;
        for (unsigned i = 0; i < (in.sec[4] - in.sec[3] - 7)/2; i++)
            out.datadesc.push_back((Varcode)in.read_number(3, 7 + i * 2, 2));
        TRACE("     s3length %d subsets %zd observed %d compression %d byte7 %x\n",
                in.sec[4] - in.sec[3], expected_subsets, (in.read_byte(3, 6) & 0x80) ? 1 : 0,
                out.compression, in.read_byte(3, 6));
        /*
       IFTRACE{
       TRACE(" -> data descriptor section: ");
       bufrex_opcode_print(msg->datadesc, stderr);
       TRACE("\n");
       }
       */
        // Once we filled the Bulletin header info, load decoding tables and allocate subsets
        out.load_tables();
    }

    /* Decode message data section after the header has been decoded */
    void decode_data();
};

/// Common functions to both decoders
struct BaseBufrDecoder : public bulletin::Parser
{
    /// Decoder object with configuration information
    Decoder& d;

    /// Input buffer
    bulletin::BufrInput& in;

    BaseBufrDecoder(Decoder& d, unsigned subset_idx, const Subset& current_subset)
        : bulletin::Parser(d.out.tables, d.out.datadesc, subset_idx, current_subset), d(d), in(d.in)
    {
        associated_field.skip_missing = !d.conf_add_undef_attrs;
    }

    /**
     * Decode a value that must always be the same acrosso all datasets.
     *
     * @returns the decoded value
     */
    virtual Var decode_semantic_b_value(Varinfo info) = 0;

    /**
     * Add \a var to all datasets, returning a pointer to one version of \a var
     * that is memory managed by one of the datasets.
     */
    virtual const Var& add_to_all(const Var& var) = 0;

    const Var& define_semantic_var(Varinfo info) override
    {
        return add_to_all(decode_semantic_b_value(info));
    }
    void define_bitmap(Varcode rep_code, Varcode delayed_code, const Opcodes& ops) override;
};

/// Decoder for uncompressed data
struct UncompressedBufrDecoder : public BaseBufrDecoder
{
    /// Subset we send variables to
    Subset* target = nullptr;

    /// If set, it is the associated field for the next variable to be decoded
    Var* cur_associated_field = nullptr;

    UncompressedBufrDecoder(Decoder& d, unsigned subset_no, const Subset& current_subset)
        : BaseBufrDecoder(d, subset_no, current_subset), target(&d.out.obtain_subset(subset_no))
    {
    }

    ~UncompressedBufrDecoder()
    {
        if (cur_associated_field)
            delete cur_associated_field;
    }

    Var decode_b_value(Varinfo info)
    {
        Var var(info);
        switch (info->type)
        {
            case Vartype::String:
                in.decode_string(var);
                break;
            case Vartype::Binary:
                in.decode_binary(var);
                break;
            case Vartype::Integer:
            case Vartype::Decimal:
                in.decode_number(var);
                break;
        }
        return var;
    }

    Var decode_semantic_b_value(Varinfo info)
    {
        return decode_b_value(info);
    }

    /**
     * Request processing, according to \a info, of the attribute \a attr_code
     * of the variable in position \a var_pos in the current subset.
     */
    void do_attr(Varinfo info, unsigned var_pos, Varcode attr_code) override
    {
        Var var = decode_b_value(info);
        TRACE(" do_attr adding var %01d%02d%03d %s as attribute to %01d%02d%03d\n",
                WR_VAR_F(var.code()),
                WR_VAR_X(var.code()),
                WR_VAR_Y(var.code()),
                var.value(),
                WR_VAR_F((*target)[var_pos].code()),
                WR_VAR_X((*target)[var_pos].code()),
                WR_VAR_Y((*target)[var_pos].code()));
        (*target)[var_pos].seta(var);
    }

    /**
     * Request processing, according to \a info, of a data variable.
     */
    void do_var(Varinfo info) override
    {
        if (associated_field.bit_count)
        {
            if (cur_associated_field)
            {
                delete cur_associated_field;
                cur_associated_field = 0;
            }
            TRACE("decode_b_data:reading %d bits of C04 information\n", associated_field.bit_count);
            uint32_t val = in.get_bits(associated_field.bit_count);
            TRACE("decode_b_data:read C04 information %x\n", val);
            cur_associated_field = associated_field.make_attribute(val).release();
        }

        target->store_variable(decode_b_value(info));
        IFTRACE {
            TRACE(" do_var decoded: ");
            target->back().print(stderr);
        }
        if (cur_associated_field)
        {
            IFTRACE {
                TRACE(" do_var with associated field: ");
                cur_associated_field->print(stderr);
            }
            unique_ptr<Var> af(cur_associated_field);
            cur_associated_field = 0;
            target->back().seta(move(af));
        }
    }

    const Var& add_to_all(const Var& var) override
    {
        target->store_variable(var);
        return current_subset.back();
    }

    /**
     * Request processing of C05yyy character data
     */
    void do_char_data(Varcode code)
    {
        unsigned cdatalen = WR_VAR_Y(code);
        string buf;
        buf.resize(cdatalen);
        TRACE("decode_c_data:character data %d long\n", cdatalen);
        for (unsigned i = 0; i < cdatalen; ++i)
        {
            uint32_t bitval = in.get_bits(8);
            TRACE("decode_c_data:decoded character %d %c\n", (int)bitval, (char)bitval);
            buf[i] = bitval;
        }

        // Add as C variable to the subset

        // Create a single use varinfo to store the bitmap
        Varinfo info = tables.get_chardata(code, buf);

        // Store the character data
        Var cdata(info, buf);
        add_to_all(cdata);

        TRACE("decode_c_data:decoded string %s\n", buf.c_str());
    }
};

struct DataSink : public bulletin::CompressedVarSink
{
    Bulletin& out;
    DataSink(Bulletin& out) : out(out) {}
    virtual void operator()(const Var& var, unsigned idx)
    {
        out.subsets[idx].store_variable(var);
    }
};

struct AttrSink : public bulletin::CompressedVarSink
{
    Bulletin& out;
    unsigned var_pos;
    AttrSink(Bulletin& out, unsigned var_pos) : out(out), var_pos(var_pos) {}
    virtual void operator()(const Var& var, unsigned idx)
    {
        out.subsets[idx][var_pos].seta(var);
    }
};

/// Decoder for compressed data
struct CompressedBufrDecoder : public BaseBufrDecoder
{
    /// Number of subsets in data section
    unsigned subset_count;

    CompressedBufrDecoder(Decoder& d, unsigned subset_no, const Subset& current_subset)
        : BaseBufrDecoder(d, subset_no, current_subset), subset_count(d.out.subsets.size())
    {
    }

    void decode_b_value(Varinfo info, bulletin::CompressedVarSink& dest)
    {
        switch (info->type)
        {
            case Vartype::String:
                in.decode_string(info, subset_count, dest);
                break;
            case Vartype::Binary:
                throw error_unimplemented("decode_b_binary TODO");
            case Vartype::Integer:
            case Vartype::Decimal:
                in.decode_number(info, subset_count, associated_field, dest);
                break;
        }
    }

    Var decode_semantic_b_value(Varinfo info)
    {
        Var var(info);
        switch (info->type)
        {
            case Vartype::String:
                in.decode_string(var, subset_count);
                break;
            case Vartype::Binary:
                throw error_unimplemented("decode_b_binary TODO");
            case Vartype::Integer:
            case Vartype::Decimal:
                in.decode_number(var, subset_count);
                break;
        }
        return var;
    }

    const Var& add_to_all(const Var& var) override
    {
        const Var* res = 0;
        for (unsigned i = 0; i < subset_count; ++i)
        {
            d.out.subsets[i].store_variable(var);
            if (!res) res = &d.out.subsets[i].back();
        }
        return *res;
    }

    void do_var(Varinfo info) override
    {
        DataSink target(d.out);
        decode_b_value(info, target);
    }

    void do_attr(Varinfo info, unsigned var_pos, Varcode attr_code) override
    {
        AttrSink target(d.out, var_pos);
        decode_b_value(info, target);
    }

    void do_char_data(Varcode code) override
    {
        // TODO: if compressed, extract the data from each subset? Store it in each dataset?
        error_unimplemented::throwf("C05%03d character data found in compressed message and it is not clear how it should be handled", WR_VAR_Y(code));
    }
};

void BaseBufrDecoder::define_bitmap(Varcode rep_code, Varcode delayed_code, const Opcodes& ops)
{
    Varcode code = bitmaps.pending_definitions;

    unsigned group = WR_VAR_X(rep_code);
    unsigned count = WR_VAR_Y(rep_code);

    TRACE("define_bitmap %d\n", count);

    if (count == 0)
    {
        // Fetch the repetition count
        Varinfo rep_info = tables.btable->query(delayed_code);
        Var rep_count = decode_semantic_b_value(rep_info);
        count = rep_count.enqi();
    }

    // Sanity checks
    if (group != 1)
        in.parse_error("bitmap section replicates %u descriptors instead of one", group);
    if (ops.size() != 1)
        in.parse_error("there are %u descriptors after bitmap replicator instead of just one", ops.size());
    if (ops[0] != WR_VAR(0, 31, 31))
        in.parse_error("bitmap element descriptor is %01d%02d%03d instead of B31031",
                WR_VAR_F(ops[0]), WR_VAR_X(ops[0]), WR_VAR_Y(ops[0]));

    // Bitmap size is now in count

    // Read the bitmap
    string buf;
    buf.resize(count);
    if (d.out.compression)
    {
        for (unsigned i = 0; i < count; ++i)
        {
            uint32_t val = in.get_bits(1);
            buf[i] = (val == 0) ? '+' : '-';
            // Decode the number of bits (encoded in 6 bits) of difference
            // values. It's odd to repeat this for each bit in the bitmap, but
            // that's how things are transmitted and it's somewhat consistent
            // with how data compression is specified
            val = in.get_bits(6);
            // If compressed, ensure that the difference bits are 0 and they are
            // not trying to transmit odd things like delta bitmaps 
            if (val != 0)
                in.parse_error("bitmap entry %u declares %u difference bits, but we only support 0", i, val);
        }
    } else {
        for (unsigned i = 0; i < count; ++i)
        {
            uint32_t val = in.get_bits(1);
            buf[i] = (val == 0) ? '+' : '-';
        }
    }

    // Create a single use varinfo to store the bitmap
    Varinfo info = tables.get_bitmap(code, buf);

    // Store the bitmap
    Var bmp(info, buf);

    // Add var to subset(s)
    const Var& res = add_to_all(bmp);

    // Bitmap will stay set as a reference to the variable to use as the
    // current bitmap. The subset(s) are taking care of memory managing it.

    IFTRACE {
        TRACE("Decoded bitmap count %u: ", count);
        res.print(stderr);
        TRACE("\n");
    }

    bitmaps.define(res, current_subset, data_pos);
}

void Decoder::decode_data()
{
    out.obtain_subset(expected_subsets - 1);

    /* Read BUFR section 4 (Data section) */
    TRACE("  decode_data:section 4 is %d bytes long (%02x %02x %02x %02x)\n",
            in.read_number(4, 0, 3),
            in.read_byte(4, 0),
            in.read_byte(4, 1),
            in.read_byte(4, 2),
            in.read_byte(4, 3));

    if (out.compression)
    {
        // Run only once
        CompressedBufrDecoder dec(*this, 0, out.subsets[0]);
        dec.run();
    } else {
        // Run once per subset
        for (unsigned i = 0; i < out.subsets.size(); ++i)
        {
            UncompressedBufrDecoder dec(*this, i, out.subsets[i]);
            dec.run();
        }
    }

    IFTRACE {
        if (in.bits_left() > 32)
        {
            fprintf(stderr, "The data section of %s:%zd still contains %d unparsed bits\n",
                    in.fname, in.start_offset, in.bits_left() - 32);
            /*
               err = dba_error_parse(msg->file->name, POS + vec->cursor,
               "the data section still contains %d unparsed bits",
               bitvec_bits_left(vec));
               goto fail;
               */
        }
    }

    /* Read BUFR section 5 (Data section) */
    in.check_available_data(5, 0, 4, "section 5 of BUFR message (end section)");

    if (memcmp(in.data + in.sec[5], "7777", 4) != 0)
        in.parse_error(5, 0, "section 5 does not contain '7777'");

    //if (subsets_no != out.subsets.size())
    //    parse_error(sec5, "header advertised %u subsets but only %zd found", subsets_no, out.subsets.size());
}

}


void BufrBulletin::decode_header(const std::string& buf, const char* fname, size_t offset)
{
    clear();
    this->fname = fname;
    this->offset = offset;
    Decoder d(buf, fname, offset, *this);
    d.decode_header();
}

void BufrBulletin::decode(const std::string& buf, const char* fname, size_t offset)
{
    clear();
    this->fname = fname;
    this->offset = offset;
    Decoder d(buf, fname, offset, *this);
    d.decode_header();
    d.decode_data();
}

}
/* vim:set ts=4 sw=4: */
