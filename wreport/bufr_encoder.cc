#include "bulletin.h"
#include "bulletin/buffers.h"
#include "bulletin/internals.h"
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
    bulletin::BufrOutput& ob;

    DDSEncoder(Bulletin& b, unsigned subset_idx, bulletin::BufrOutput& ob)
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
    void define_variable(Varinfo info) override
    {
        const Var& var = get_var();

        // Deal with an associated field
        if (associated_field.bit_count)
        {
            const Var* att = associated_field.get_attribute(var);
            if (att && att->isset())
                ob.add_bits(att->enqi(), associated_field.bit_count);
            else
                ob.append_missing(associated_field.bit_count);
        }

        ob.append_var(info, var);
    }
    const Var& define_semantic_variable(Varinfo info) override
    {
        const Var& var = get_var();
        ob.append_var(info, var);
        return var;
    }
    void define_bitmap(Varcode rep_code, Varcode delayed_code, const Opcodes& ops) override
    {
        const Var& var = get_var();
        if (WR_VAR_F(var.code()) != 2)
            error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                    current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
        IFTRACE{
            TRACE("Encoding data present bitmap:");
            var.print(stderr);
        }

        //int group = WR_VAR_X(rep_code);
        int count = WR_VAR_Y(rep_code);

        if (count == 0)
        {
            Varinfo info = tables.btable->query(delayed_code);
            count = var.info()->len;
            ob.add_bits(count, info->bit_len);
        }
        TRACE("encode_r_data bitmap %d items %d times%s\n", group, count, delayed_code ? " (delayed)" : "");

        // Encode the bitmap here directly
        if (ops[0] != WR_VAR(0, 31, 31))
            error_consistency::throwf("bitmap data descriptor is %d%02d%03d instead of B31031",
                    WR_VAR_F(ops[0]), WR_VAR_X(ops[0]), WR_VAR_Y(ops[0]));
        if (ops.size() != 1)
            error_consistency::throwf("repeated sequence for bitmap encoding contains more than just B31031");

        for (unsigned i = 0; i < var.info()->len; ++i)
            ob.add_bits(var.enqc()[i] == '+' ? 0 : 1, 1);

        bitmaps.define(var, current_subset);
    }
    void define_raw_character_data(Varcode code) override
    {
        const Var& var = get_var();
        const char* val = var.enq("");
        ob.append_string(val, WR_VAR_Y(code) * 8);
    }
};


struct Encoder
{
    /* Input message data */
    BufrBulletin& in;
    /// Output buffer
    bulletin::BufrOutput& out;

    /*
     * Offset of the start of BUFR sections
     *
     * We have to memorise offsets rather than pointers, because e->out->buf
     * can get reallocated during the encoding
     */
    unsigned sec[6] = { 0, 0, 0, 0, 0, 0 };

    Encoder(BufrBulletin& in, bulletin::BufrOutput& out)
        : in(in), out(out)
    {
    }

    void encode_sec0()
    {
        // Encode bufr section 0 (Indicator section)
        out.raw_append("BUFR\0\0\0", 7);
        out.append_byte(in.edition);

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
    out.append_byte(in.subcentre);
    // Originating/generating centre (Common Code tableC-1)
    out.append_byte(in.centre);
    // Update sequence number (zero for original BUFR messages; incremented for updates)
    out.append_byte(in.update_sequence_number);
    // Bit 1: 0 No optional section, 1 Optional section included
    // Bits 2 to 8 set to zero (reserved)
    out.append_byte(in.optional_section_length ? 0x80 : 0);

    // Data category (BUFR Table A)
    out.append_byte(in.type);
    // Data sub-category (defined by local ADP centres)
    out.append_byte(in.localsubtype);
    // Version number of master tables used (currently 9 for WMO FM 94 BUFR tables)
    out.append_byte(in.master_table);
    // Version number of local tables used to augment the master table in use
    out.append_byte(in.local_table);

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
    out.append_short(in.centre);
    // Originating/generating sub-centre (defined by Originating/generating centre)
    out.append_short(in.subcentre);
    // Update sequence number (zero for original BUFR messages; incremented for updates)
    out.append_byte(in.update_sequence_number);
    // Bit 1: 0 No optional section, 1 Optional section included
    // Bits 2 to 8 set to zero (reserved)
    out.append_byte(in.optional_section_length ? 0x80 : 0);

    // Data category (BUFR Table A)
    out.append_byte(in.type);
    // International data sub-category
    out.append_byte(in.subtype);
    // Local subcategory (defined by local ADP centres)
    out.append_byte(in.localsubtype);
    // Version number of master tables used (currently 9 for WMO FM 94 BUFR tables)
    out.append_byte(in.master_table);
    // Version number of local tables used to augment the master table in use
    out.append_byte(in.local_table);

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

    if (in.optional_section_length)
    {
        bool pad = in.optional_section_length % 2 == 1;

        // Length of section
        if (pad)
            out.add_bits(4 + in.optional_section_length + 1, 24);
        else
            out.add_bits(4 + in.optional_section_length, 24);

        // Set to 0 (reserved)
        out.append_byte(0);

        // Append the raw optional section data
        out.raw_append(in.optional_section, in.optional_section_length);

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

void BufrBulletin::encode(std::string& buf)
{
    bulletin::BufrOutput out(buf);

    Encoder e(*this, out);

    e.encode_sec0();

    switch (edition)
    {
        case 2:
        case 3: e.encode_sec1ed3(); break;
        case 4: e.encode_sec1ed4(); break;
        default:
            error_unimplemented::throwf("Encoding BUFR edition %d is not implemented", edition);
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
}

}
