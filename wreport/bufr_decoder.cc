#include "bulletin.h"
#include "bulletin/internals.h"
#include "buffers/bufr.h"
#include <cstring>
#include "config.h"

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
    buffers::BufrInput in;
    /* Output decoded variables */
    BufrBulletin& out;
    /// Number of expected subsets (read in decode_header, used in decode_data)
    size_t expected_subsets;
    /// True if undefined attributes are added to the output, else false
    bool conf_add_undef_attrs;
    /// Optional section length decoded from the message
    unsigned optional_section_length = 0;

    Decoder(const std::string& buf, const char* fname, size_t offset, BufrBulletin& out)
        : in(buf), out(out), conf_add_undef_attrs(false)
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
        optional_section_length = in.sec[3] - in.sec[2];
        if (optional_section_length)
            optional_section_length -= 4;
        // subcentre in sec1[4]
        out.originating_subcentre = in.read_byte(1, 4);
        // centre in sec1[5]
        out.originating_centre = in.read_byte(1, 5);
        // Update sequence number sec1[6]
        out.update_sequence_number = in.read_byte(1, 6);
        out.master_table_version_number = in.read_byte(1, 10);
        out.master_table_version_number_local = in.read_byte(1, 11);
        out.data_category = in.read_byte(1, 8);
        out.data_subcategory = 0xff;
        out.data_subcategory_local = in.read_byte(1, 9);

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
        out.originating_centre = in.read_number(1, 4, 2);
        // subcentre in sec1[6-7]
        out.originating_subcentre = in.read_number(1, 6, 2);
        // update sequence number sec1[8]
        out.update_sequence_number = in.read_byte(1, 8);
        // has_optional in sec1[9]
        // Once we know if the optional section is available, we can scan
        // section lengths for the rest of the message
        in.scan_other_sections(in.read_byte(1, 9) & 0x80);
        optional_section_length = in.sec[3] - in.sec[2];
        if (optional_section_length)
            optional_section_length -= 4;
        // category in sec1[10]
        out.data_category = in.read_byte(1, 10);
        // international data sub-category in sec1[11]
        out.data_subcategory = in.read_byte(1, 11);
        // local data sub-category in sec1[12]
        out.data_subcategory_local = in.read_byte(1, 12);
        // version number of master table in sec1[13]
        out.master_table_version_number = in.read_byte(1, 13);
        // version number of local table in sec1[14]
        out.master_table_version_number_local = in.read_byte(1, 14);
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
        out.edition_number = in.read_byte(0, 7);
        if (out.edition_number != 2 && out.edition_number != 3 && out.edition_number != 4)
            in.parse_error(0, 7, "Only BUFR edition 2, 3, and 4 are supported (this message is edition %d)", out.edition_number);

        // Looks like a BUFR, scan section starts
        in.scan_lead_sections();

        // Read bufr section 1 (Identification section)
        in.check_available_data(1, 0, out.edition_number == 4 ? 22 : 18, "section 1 of BUFR message (identification section)");

        switch (out.edition_number)
        {
            case 2: decode_sec1ed3(); break;
            case 3: decode_sec1ed3(); break;
            case 4: decode_sec1ed4(); break;
            default:
                error_consistency::throwf("BUFR edition is %d, but I can only decode 2, 3 and 4", out.edition_number);
        }

        TRACE("BUFR:edition %d, optional section %ub, update sequence number %d\n",
                out.edition, optional_section_length, out.update_sequence_number);
        TRACE("     origin %d.%d tables %d.%d type %d.%d %04d-%02d-%02d %02d:%02d\n",
                out.centre, out.subcentre,
                out.master_table, out.local_table,
                out.type, out.subtype,
                out.rep_year, out.rep_month, out.rep_day, out.rep_hour, out.rep_minute);

        // Read BUFR section 2 (Optional section)
        if (optional_section_length)
        {
            out.optional_section = string(
                    (const char*)in.data + in.sec[2] + 4,
                    in.read_number(2, 0, 3) - 4);
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

/// Decoder for uncompressed data
struct UncompressedBufrDecoder : public bulletin::UncompressedDecoder
{
    /// Input buffer
    buffers::BufrInput& in;

    /// If set, it is the associated field for the next variable to be decoded
    Var* cur_associated_field = nullptr;

    UncompressedBufrDecoder(Bulletin& bulletin, unsigned subset_no, buffers::BufrInput& in)
        : bulletin::UncompressedDecoder(bulletin, subset_no), in(in)
    {
    }

    ~UncompressedBufrDecoder()
    {
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

    void define_substituted_value(unsigned pos) override
    {
        // Use the details of the corrisponding variable for decoding
        Varinfo info = output_subset[pos].info();
        Var var = decode_b_value(info);
        TRACE(" define_substituted_value adding var %01d%02d%03d %s as attribute to %01d%02d%03d\n",
                WR_VAR_FXY(var.code()), var.value(), WR_VAR_FXY(output_subset[var_pos].code()));
        output_subset[pos].seta(var);
    }

    void define_attribute(Varinfo info, unsigned pos) override
    {
        Var var = decode_b_value(info);
        TRACE(" define_attribute adding var %01d%02d%03d %s as attribute to %01d%02d%03d\n",
                WR_VAR_FXY(var.code()), var.value(), WR_VAR_FXY(output_subset[var_pos].code()));
        output_subset[pos].seta(var);
    }

    /**
     * Request processing, according to \a info, of a data variable.
     */
    void define_variable(Varinfo info) override
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

        output_subset.store_variable(decode_b_value(info));
        IFTRACE {
            TRACE(" define_variable decoded: ");
            output_subset.back().print(stderr);
        }
        if (cur_associated_field)
        {
            IFTRACE {
                TRACE(" define_variable with associated field: ");
                cur_associated_field->print(stderr);
            }
            unique_ptr<Var> af(cur_associated_field);
            cur_associated_field = 0;
            output_subset.back().seta(move(af));
        }
    }

    /**
     * Request processing of C05yyy character data
     */
    void define_raw_character_data(Varcode code)
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
        output_subset.store_variable(cdata);

        TRACE("decode_c_data:decoded string %s\n", buf.c_str());
    }

    unsigned define_delayed_replication_factor(Varinfo info) override
    {
        output_subset.store_variable(decode_b_value(info));
        return output_subset.back().enqi();
    }

    unsigned define_associated_field_significance(Varinfo info) override
    {
        output_subset.store_variable(decode_b_value(info));
        return output_subset.back().enq(63);
    }

    unsigned define_bitmap_delayed_replication_factor(Varinfo info) override
    {
        Var rep_count = decode_b_value(info);
        return rep_count.enqi();
    }

    void define_bitmap(unsigned bitmap_size) override
    {
        Varcode code = bitmaps.pending_definitions;

        TRACE("define_bitmap %d\n", bitmap_size);

        // Bitmap size is now in count

        // Read the bitmap
        string buf = in.decode_uncompressed_bitmap(bitmap_size);

        // Create a single use varinfo to store the bitmap
        Varinfo info = tables.get_bitmap(code, buf);

        // Store the bitmap
        Var bmp(info, buf);

        // Add var to subset(s)
        output_subset.store_variable(bmp);
        const Var& res = output_subset.back();

        // Bitmap will stay set as a reference to the variable to use as the
        // current bitmap. The subset(s) are taking care of memory managing it.

        IFTRACE {
            TRACE("Decoded bitmap count %u: ", bitmap_size);
            res.print(stderr);
            TRACE("\n");
        }

        bitmaps.define(res, output_subset);
    }
};

/// Decoder for compressed data
struct CompressedBufrDecoder : public bulletin::CompressedDecoder
{
    /// Input buffer
    buffers::BufrInput& in;

    /// Number of subsets in data section
    unsigned subset_count;

    CompressedBufrDecoder(BufrBulletin& bulletin, buffers::BufrInput& in)
        : bulletin::CompressedDecoder(bulletin), in(in), subset_count(bulletin.subsets.size())
    {
    }

    void decode_b_value(Varinfo info, std::function<void(unsigned, Var&&)> dest)
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
                if (associated_field.bit_count)
                {
                    in.decode_compressed_number(info, associated_field.bit_count, subset_count, [&](unsigned subset_no, Var&& var, uint32_t associated_field_val) {
                        unique_ptr<Var> af(associated_field.make_attribute(associated_field_val));
                        if (af.get()) var.seta(move(af));
                        dest(subset_no, move(var));
                    });
                }
                else
                    in.decode_compressed_number(info, subset_count, dest);
                break;
        }
    }

    /**
     * Decode a value that must always be the same acrosso all datasets.
     *
     * @returns the decoded value
     */
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
                in.decode_compressed_semantic_number(var, subset_count);
                break;
        }
        return var;
    }

    /**
     * Add \a var to all datasets, returning a pointer to one version of \a var
     * that is memory managed by one of the datasets.
     */
    void add_to_all(const Var& var)
    {
        for (unsigned i = 0; i < subset_count; ++i)
            output_bulletin.subsets[i].store_variable(var);
    }

    void define_variable(Varinfo info) override
    {
        decode_b_value(info, [&](unsigned idx, Var&& var){
            output_bulletin.subsets[idx].store_variable(var);
        });
    }

    void define_substituted_value(unsigned pos) override
    {
        // Use the details of the corrisponding variable for decoding
        Varinfo info = output_bulletin.subset(0)[pos].info();
        decode_b_value(info, [&](unsigned idx, Var&& var) {
            output_bulletin.subsets[idx][pos].seta(var);
        });
    }

    void define_attribute(Varinfo info, unsigned pos) override
    {
        decode_b_value(info, [&](unsigned idx, Var&& var) {
            output_bulletin.subsets[idx][pos].seta(var);
        });
    }

    void define_raw_character_data(Varcode code) override
    {
        // TODO: if compressed, extract the data from each subset? Store it in each dataset?
        error_unimplemented::throwf("C05%03d character data found in compressed message and it is not clear how it should be handled", WR_VAR_Y(code));
    }

    unsigned define_delayed_replication_factor(Varinfo info) override
    {
        Var res(decode_semantic_b_value(info));
        add_to_all(res);
        return res.enqi();
    }

    unsigned define_associated_field_significance(Varinfo info) override
    {
        Var res(decode_semantic_b_value(info));
        add_to_all(res);
        return res.enq(63);
    }

    unsigned define_bitmap_delayed_replication_factor(Varinfo info) override
    {
        Var rep_count = decode_semantic_b_value(info);
        return rep_count.enqi();
    }
    void define_bitmap(unsigned bitmap_size) override
    {
        Varcode code = bitmaps.pending_definitions;

        // Read the bitmap
        string buf = in.decode_compressed_bitmap(bitmap_size);

        // Create a single use varinfo to store the bitmap
        Varinfo info = tables.get_bitmap(code, buf);

        // Create the bitmap variable
        Var bmp(info, buf);

        // Add var to subset(s)
        add_to_all(bmp);

        // Bitmap will stay set as a reference to the variable to use as the
        // current bitmap. The subset(s) are taking care of memory managing it.

        IFTRACE {
            TRACE("Decoded bitmap count %u: ", bitmap_size);
            bmp.print(stderr);
            TRACE("\n");
        }

        bitmaps.define(move(bmp), output_bulletin.subset(0));
    }
};

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
        CompressedBufrDecoder dec(out, in);
        dec.associated_field.skip_missing = !conf_add_undef_attrs;
        dec.run();
    } else {
        // Run once per subset
        for (unsigned i = 0; i < out.subsets.size(); ++i)
        {
            UncompressedBufrDecoder dec(out, i, in);
            dec.associated_field.skip_missing = !conf_add_undef_attrs;
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


std::unique_ptr<BufrBulletin> BufrBulletin::decode_header(const std::string& buf, const char* fname, size_t offset)
{
    auto res = BufrBulletin::create();
    res->fname = fname;
    res->offset = offset;
    Decoder d(buf, fname, offset, *res);
    d.decode_header();
    return res;
}

std::unique_ptr<BufrBulletin> BufrBulletin::decode(const std::string& buf, const char* fname, size_t offset)
{
    auto res = BufrBulletin::create();
    res->fname = fname;
    res->offset = offset;
    Decoder d(buf, fname, offset, *res);
    d.decode_header();
    d.decode_data();
    return res;
}

}
