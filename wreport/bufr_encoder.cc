#include "bulletin.h"
#include "bulletin/internals.h"
#include "buffers/bufr.h"
#include "vartable.h"
#include <netinet/in.h>
#include <cstring>
#include "config.h"

// #define TRACE_ENCODER

#ifdef TRACE_ENCODER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {

namespace {

struct DDSEncoder : public bulletin::UncompressedEncoder
{
    buffers::BufrOutput& ob;

    DDSEncoder(const Bulletin& b, unsigned subset_idx, buffers::BufrOutput& ob)
        : UncompressedEncoder(b, subset_idx), ob(ob)
    {
    }
    virtual ~DDSEncoder() {}

    void define_substituted_value(unsigned pos) override
    {
        // Use the details of the corrisponding variable for decoding
        Varinfo info = current_subset[pos].info();
        encode_attr(info, pos, info->code);
    }

    void define_attribute(Varinfo info, unsigned pos) override
    {
        encode_attr(info, pos, info->code);
    }

    void encode_attr(Varinfo info, unsigned var_pos, Varcode attr_code)
    {
        const Var& var = get_var(var_pos);
        if (const Var* a = var.enqa(attr_code))
            ob.append_var(info, *a);
        else
            ob.append_missing(info);
    }

    void encode_associated_field(const Var& var) override
    {
        const Var* att = associated_field.get_attribute(var);
        if (att && att->isset())
            ob.add_bits(att->enqi(), associated_field.bit_count);
        else
            ob.append_missing(associated_field.bit_count);
    }

    void encode_var(Varinfo info, const Var& var) override
    {
        ob.append_var(info, var);
    }

    void define_bitmap(unsigned bitmap_size) override
    {
        const Var& var = get_var();
        if (WR_VAR_F(var.code()) != 2)
            error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                    current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
        IFTRACE{
            TRACE("Encoding data present bitmap:");
            var.print(stderr);
        }

        //TRACE("encode_r_data bitmap %d items %d times%s\n", group, count, delayed_code ? " (delayed)" : "");

        if (var.info()->len != bitmap_size)
            error_consistency::throwf("bitmap given is %u bits long, but we need to encode %u bits",
                    var.info()->len, bitmap_size);

        // Encode the bitmap here directly
        for (unsigned i = 0; i < bitmap_size; ++i)
            ob.add_bits(var.enqc()[i] == '+' ? 0 : 1, 1);

        bitmaps.define(var, current_subset, current_var);
    }
    void define_raw_character_data(Varcode code) override
    {
        const Var& var = get_var();
        const char* val = var.enq("");
        ob.append_string(val, WR_VAR_Y(code) * 8);
    }

    void define_c03_refval_override(Varcode code) override
    {
        // Scan the subset looking for a variables with the given code, to see what
        // is its bit_ref
        bool found = false;
        int bit_ref = 0;
        for (const auto& var: current_subset)
        {
            Varinfo info = var.info();
            if (info->code == code)
            {
                bit_ref = info->bit_ref;
                found = true;
                break;
            }
        }

        if (!found)
        {
            // If not found, take the default
            Varinfo info = current_subset.tables->btable->query(code);
            bit_ref = info->bit_ref;
        }


        // Encode
        uint32_t encoded;
        unsigned nbits;
        if (bit_ref < 0)
        {
            encoded = -bit_ref;
            nbits = 32 - __builtin_clz(encoded);
            encoded |= 1 << (c03_refval_override_bits - 1);
        } else {
            encoded = bit_ref;
            nbits = 32 - __builtin_clz(encoded);
        }

        // Check if it fits (encoded bits plus 1 for the sign)
        if (nbits + 1 > c03_refval_override_bits)
            error_consistency::throwf("C03 reference value override requested for value %d, encoded as %u, which does not fit in %u bits (requires %u bits)", bit_ref, encoded, c03_refval_override_bits, nbits + 1);

        ob.add_bits(encoded, c03_refval_override_bits);
        c03_refval_overrides[code] = bit_ref;
    }
};


struct Encoder
{
    /* Input message data */
    const BufrBulletin& in;
    /// Output buffer
    buffers::BufrOutput& out;

    /*
     * Offset of the start of BUFR sections
     *
     * We have to memorise offsets rather than pointers, because e->out->buf
     * can get reallocated during the encoding
     */
    unsigned sec[6] = { 0, 0, 0, 0, 0, 0 };

    Encoder(const BufrBulletin& in, buffers::BufrOutput& out)
        : in(in), out(out)
    {
    }

    void encode_sec0()
    {
        // Encode bufr section 0 (Indicator section)
        out.raw_append("BUFR\0\0\0", 7);
        out.append_byte(in.edition_number);

        TRACE("sec0 ends at %zd\n", out.out.size());
    }
    void encode_sec1ed3();
    void encode_sec1ed4();
    void encode_sec2();
    void encode_sec3();
    void encode_sec4();
    void encode_sec5()
    {
        sec[5] = out.out.size();

        // Encode section 5 (End section)
        out.raw_append("7777", 4);

        TRACE("sec5 ends at %zd\n", out.out.size());
    }
};

void Encoder::encode_sec1ed3()
{
    // Encode bufr section 1 (Identification section)
    sec[1] = out.out.size();

    // Length of section
    out.add_bits(18, 24);
    // Master table number
    out.append_byte(in.master_table_number);
    // Originating/generating sub-centre (defined by Originating/generating centre)
    out.append_byte(in.originating_subcentre);
    // Originating/generating centre (Common Code tableC-1)
    out.append_byte(in.originating_centre);
    // Update sequence number (zero for original BUFR messages; incremented for updates)
    out.append_byte(in.update_sequence_number);
    // Bit 1: 0 No optional section, 1 Optional section included
    // Bits 2 to 8 set to zero (reserved)
    out.append_byte(in.optional_section.empty() ? 0 : 0x80);

    // Data category (BUFR Table A)
    out.append_byte(in.data_category);
    // Data sub-category (defined by local ADP centres)
    out.append_byte(in.data_subcategory_local);
    // Version number of master tables used (currently 9 for WMO FM 94 BUFR tables)
    out.append_byte(in.master_table_version_number);
    // Version number of local tables used to augment the master table in use
    out.append_byte(in.master_table_version_number_local);

    // Year of century
    out.append_byte(in.rep_year == 2000 ? 100 : (in.rep_year % 100));
    // Month
    out.append_byte(in.rep_month);
    // Day
    out.append_byte(in.rep_day);
    // Hour
    out.append_byte(in.rep_hour);
    // Minute
    out.append_byte(in.rep_minute);
    // Century
    out.append_byte(in.rep_year / 100);

    TRACE("sec1 ends at %zd\n", out.out.size());
}

void Encoder::encode_sec1ed4()
{
    // Encode bufr section 1 (Identification section)
    sec[1] = out.out.size();

    // Length of section
    out.add_bits(22, 24);
    // Master table number
    out.append_byte(0);
    // Originating/generating centre (Common Code tableC-1)
    out.append_short(in.originating_centre);
    // Originating/generating sub-centre (defined by Originating/generating centre)
    out.append_short(in.originating_subcentre);
    // Update sequence number (zero for original BUFR messages; incremented for updates)
    out.append_byte(in.update_sequence_number);
    // Bit 1: 0 No optional section, 1 Optional section included
    // Bits 2 to 8 set to zero (reserved)
    out.append_byte(in.optional_section.empty() ? 0 : 0x80);

    // Data category (BUFR Table A)
    out.append_byte(in.data_category);
    // International data sub-category
    out.append_byte(in.data_subcategory);
    // Local subcategory (defined by local ADP centres)
    out.append_byte(in.data_subcategory_local);
    // Version number of master tables used (currently 9 for WMO FM 94 BUFR tables)
    out.append_byte(in.master_table_version_number);
    // Version number of local tables used to augment the master table in use
    out.append_byte(in.master_table_version_number_local);

    // Year of century
    out.append_short(in.rep_year);
    // Month
    out.append_byte(in.rep_month);
    // Day
    out.append_byte(in.rep_day);
    // Hour
    out.append_byte(in.rep_hour);
    // Minute
    out.append_byte(in.rep_minute);
    // Second
    out.append_byte(in.rep_second);

    TRACE("sec1 ends at %zd\n", out.out.size());
}

void Encoder::encode_sec2()
{
    // Encode BUFR section 2 (Optional section)
    sec[2] = out.out.size();

    if (!in.optional_section.empty())
    {
        bool pad = in.optional_section.size() % 2 == 1;

        // Length of section
        if (pad)
            out.add_bits(4 + in.optional_section.size() + 1, 24);
        else
            out.add_bits(4 + in.optional_section.size(), 24);

        // Set to 0 (reserved)
        out.append_byte(0);

        // Append the raw optional section data
        out.raw_append(in.optional_section.data(), in.optional_section.size());

        // Pad to even number of bytes
        if (pad) out.append_byte(0);
    }

    TRACE("sec2 ends at %zd\n", out.out.size());
}

void Encoder::encode_sec3()
{
    // Encode BUFR section 3 (Data description section)
    sec[3] = out.out.size();

	if (in.subsets.empty())
		throw error_consistency("message to encode has no data subsets");

	if (in.datadesc.empty())
		throw error_consistency("message to encode has no data descriptors");

    // Length of section
    out.add_bits(8 + 2*in.datadesc.size(), 24);
    // Set to 0 (reserved)
    out.append_byte(0);
    // Number of data subsets
    out.append_short(in.subsets.size());
    // Bit 0 = observed data; bit 1 = use compression
    out.append_byte(128);

    // Data descriptors
    for (unsigned i = 0; i < in.datadesc.size(); ++i)
        out.append_short(in.datadesc[i]);

    // One padding byte to make the section even
    out.append_byte(0);

    TRACE("sec3 ends at %zd\n", out.out.size());
}

void Encoder::encode_sec4()
{
    // Encode BUFR section 4 (Data section)
    sec[4] = out.out.size();

    // Length of section (currently set to 0, will be filled in later)
    out.add_bits(0, 24);
    out.append_byte(0);

    // Encode all the subsets
    for (unsigned i = 0; i < in.subsets.size(); ++i)
    {
        // Encode the data of this subset
        DDSEncoder e(in, i, out);
        e.run();
    }

    // Write all the bits and pad the data section to reach an even length
    out.flush();
    if ((out.out.size() % 2) == 1)
        out.append_byte(0);
    out.flush();

    // Write the length of the section in its header
    {
        uint32_t val = htonl(out.out.size() - sec[4]);
        memcpy((char*)out.out.data() + sec[4], ((char*)&val) + 1, 3);

        TRACE("sec4 size %zd\n", out.out.size() - sec[4]);
    }

    TRACE("sec4 ends at %zd\n", out.out.size());
}

}

string BufrBulletin::encode() const
{
    std::string buf;
    buf.reserve(1024);
    buffers::BufrOutput out(buf);

    Encoder e(*this, out);

    e.encode_sec0();

    switch (edition_number)
    {
        case 2:
        case 3: e.encode_sec1ed3(); break;
        case 4: e.encode_sec1ed4(); break;
        default:
            error_unimplemented::throwf("Encoding BUFR edition %d is not implemented", edition_number);
    }

    e.encode_sec2();
    e.encode_sec3();
    e.encode_sec4();
    e.encode_sec5();

    // Write the length of the BUFR message in its header
    {
        uint32_t val = htonl(out.out.size());
        memcpy((char*)out.out.data() + 4, ((char*)&val) + 1, 3);
        TRACE("msg size %zd\n", out.out.size());
    }

#if 0
    // Not doing it because we are const

    // Store the section offsets in the BUFR bulletin
    for (unsigned i = 0; i < 5; ++i)
        section_end[i] = e.sec[i + 1];
    section_end[5] = section_end[4] + 4;
#endif

    return buf;
}

}
