#include "decoder.h"
#include "trace.h"
#include "wreport/vartable.h"
#include <cstring>

namespace wreport {
namespace bufr {

// Return a value with bitlen bits set to 1
static inline uint32_t all_ones(int bitlen)
{
    return ((1 << (bitlen - 1)) - 1) | (1 << (bitlen - 1));
}

Decoder::Decoder(const std::string& buf, const char* fname, size_t offset,
                 BufrBulletin& out)
    : in(buf), out(out)
{
    in.fname        = fname;
    in.start_offset = offset;
}

void Decoder::read_options(const BufrCodecOptions& opts)
{
    conf_add_undef_attrs = opts.decode_adds_undef_attrs;
}

void Decoder::decode_sec1ed3()
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
    out.originating_subcentre             = in.read_byte(1, 4);
    // centre in sec1[5]
    out.originating_centre                = in.read_byte(1, 5);
    // Update sequence number sec1[6]
    out.update_sequence_number            = in.read_byte(1, 6);
    out.master_table_version_number       = in.read_byte(1, 10);
    out.master_table_version_number_local = in.read_byte(1, 11);
    out.data_category                     = in.read_byte(1, 8);
    out.data_subcategory                  = 0xff;
    out.data_subcategory_local            = in.read_byte(1, 9);

    // Some sources use the 18th byte (in.read_byte(1, 17)) for the century,
    // but they do so inconsistently, sometimes using 20 for 20xx, sometimes
    // using 21 for "21st century". Some other sources store other values in
    // that byte.
    // We revert to ignoring it and perform an educated guess, hopefully by
    // 2050 nobody will write BUFR ed3 anymore, and we'll only have to deal
    // with historical data written before that time.
    out.rep_year = in.read_byte(1, 12);
    // Fix the century with a bit of euristics
    if (out.rep_year > 50)
        out.rep_year += 1900;
    else
        out.rep_year += 2000;
    out.rep_month  = in.read_byte(1, 13);
    out.rep_day    = in.read_byte(1, 14);
    out.rep_hour   = in.read_byte(1, 15);
    out.rep_minute = in.read_byte(1, 16);
}

void Decoder::decode_sec1ed4()
{
    // master table number in sec1[3]
    out.master_table_number    = in.read_byte(1, 3);
    // centre in sec1[4-5]
    out.originating_centre     = in.read_number(1, 4, 2);
    // subcentre in sec1[6-7]
    out.originating_subcentre  = in.read_number(1, 6, 2);
    // update sequence number sec1[8]
    out.update_sequence_number = in.read_byte(1, 8);
    // has_optional in sec1[9]
    // Once we know if the optional section is available, we can scan
    // section lengths for the rest of the message
    in.scan_other_sections(in.read_byte(1, 9) & 0x80);
    optional_section_length = in.sec[3] - in.sec[2];
    if (optional_section_length)
    {
        if (optional_section_length >= 4)
            optional_section_length -= 4;
        else
            error_consistency::throwf(
                "Section 2 length is %u but, if present, it must be at least 4",
                optional_section_length);
    }
    // category in sec1[10]
    out.data_category                     = in.read_byte(1, 10);
    // international data sub-category in sec1[11]
    out.data_subcategory                  = in.read_byte(1, 11);
    // local data sub-category in sec1[12]
    out.data_subcategory_local            = in.read_byte(1, 12);
    // version number of master table in sec1[13]
    out.master_table_version_number       = in.read_byte(1, 13);
    // version number of local table in sec1[14]
    out.master_table_version_number_local = in.read_byte(1, 14);
    // year in sec1[15-16]
    out.rep_year                          = in.read_number(1, 15, 2);
    // month in sec1[17]
    out.rep_month                         = in.read_byte(1, 17);
    // day in sec1[18]
    out.rep_day                           = in.read_byte(1, 18);
    // hour in sec1[19]
    out.rep_hour                          = in.read_byte(1, 19);
    // minute in sec1[20]
    out.rep_minute                        = in.read_byte(1, 20);
    // sec in sec1[21]
    out.rep_second                        = in.read_byte(1, 21);
}

/* Decode the message header only */
void Decoder::decode_header()
{
    in.check_available_data(0, 8,
                            "section 0 of BUFR message (indicator section)");

    // Read BUFR section 0 (Indicator section)
    if (memcmp(in.data + in.sec[0], "BUFR", 4) != 0)
        in.parse_error(
            0, 0,
            "data does not start with BUFR header (\"%.4s\" was read instead)",
            in.data + in.sec[0]);

    // Check the BUFR edition number
    out.edition_number = in.read_byte(0, 7);
    if (out.edition_number != 2 && out.edition_number != 3 &&
        out.edition_number != 4)
        in.parse_error(0, 7,
                       "Only BUFR edition 2, 3, and 4 are supported (this "
                       "message is edition %d)",
                       out.edition_number);

    // Looks like a BUFR, scan section starts
    in.scan_lead_sections();

    // Read bufr section 1 (Identification section)
    in.check_available_message_data(
        1, 0, out.edition_number == 4 ? 22 : 18,
        "section 1 of BUFR message (identification section)");

    switch (out.edition_number)
    {
        case 2: decode_sec1ed3(); break;
        case 3: decode_sec1ed3(); break;
        case 4: decode_sec1ed4(); break;
        default:
            error_consistency::throwf(
                "BUFR edition is %d, but I can only decode 2, 3 and 4",
                out.edition_number);
    }

    TRACE("BUFR:edition %d, optional section %ub, update sequence number %d\n",
          out.edition_number, optional_section_length,
          out.update_sequence_number);
    TRACE(
        "     origin %d.%d tables %d.%d type %d.%d %04d-%02d-%02d %02d:%02d\n",
        out.originating_centre, out.originating_subcentre,
        out.master_table_number, out.master_table_version_number_local,
        out.data_category, out.data_subcategory, out.rep_year, out.rep_month,
        out.rep_day, out.rep_hour, out.rep_minute);

    // Read BUFR section 2 (Optional section)
    if (optional_section_length)
    {
        in.check_available_section_data(
            2, 0, 3, "section 2 of BUFR message (optional section length)");
        unsigned s2_length = in.read_number(2, 0, 3);
        in.check_available_section_data(
            2, 0, s2_length, "section 2 of BUFR message (optional section)");
        if (s2_length < 4)
            error_consistency::throwf(
                "Optional section length is %u but it must be at least 4",
                s2_length);
        out.optional_section =
            std::string((const char*)in.data + in.sec[2] + 4, s2_length - 4);
    }

    /* Read BUFR section 3 (Data description section) */
    if (in.sec[4] - in.sec[3] < 7)
        error_consistency::throwf(
            "Data descriptor section length is %u but it must be at least 7",
            in.sec[4] - in.sec[3]);
    in.check_available_section_data(
        3, 0, 8, "section 3 of BUFR message (data description section)");
    expected_subsets          = in.read_number(3, 4, 2);
    out.compression           = (in.read_byte(3, 6) & 0x40) ? 1 : 0;
    unsigned descriptor_count = (in.sec[4] - in.sec[3] - 7) / 2;
    in.check_available_section_data(3, 7, descriptor_count * 2,
                                    "data descriptor list");
    for (unsigned i = 0; i < descriptor_count; i++)
        out.datadesc.push_back((Varcode)in.read_number(3, 7 + i * 2, 2));
    TRACE("     s3length %d subsets %zu observed %d compression %d byte7 %x\n",
          in.sec[4] - in.sec[3], expected_subsets,
          (in.read_byte(3, 6) & 0x80) ? 1 : 0, out.compression,
          in.read_byte(3, 6));
    /*
   IFTRACE{
   TRACE(" -> data descriptor section: ");
   bufrex_opcode_print(msg->datadesc, stderr);
   TRACE("\n");
   }
   */
    // Once we filled the Bulletin header info, load decoding tables and
    // allocate subsets
    out.load_tables();
}

void Decoder::decode_data()
{
    out.obtain_subset(expected_subsets - 1);

    /* Read BUFR section 4 (Data section) */
    TRACE("  decode_data:section 4 is %d bytes long (%02x %02x %02x %02x)\n",
          in.read_number(4, 0, 3), in.read_byte(4, 0), in.read_byte(4, 1),
          in.read_byte(4, 2), in.read_byte(4, 3));

    if (out.compression)
    {
        // Run only once
        CompressedDecoderTarget target(in, out);
        std::unique_ptr<DataSectionDecoder> dec;
        if (verbose_output)
            dec.reset(
                new VerboseDataSectionDecoder(out, target, verbose_output));
        else
            dec.reset(new DataSectionDecoder(out, target));
        dec->associated_field.skip_missing = !conf_add_undef_attrs;
        dec->run();
    }
    else
    {
        // Run once per subset
        for (unsigned i = 0; i < out.subsets.size(); ++i)
        {
            UncompressedDecoderTarget target(in, out.obtain_subset(i));
            std::unique_ptr<DataSectionDecoder> dec;
            if (verbose_output)
                dec.reset(
                    new VerboseDataSectionDecoder(out, target, verbose_output));
            else
                dec.reset(new DataSectionDecoder(out, target));
            dec->associated_field.skip_missing = !conf_add_undef_attrs;
            dec->run();
        }
    }

    IFTRACE
    {
        if (in.bits_left() > 32)
        {
            fprintf(
                stderr,
                "The data section of %s:%zu still contains %u unparsed bits\n",
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
    in.check_available_section_data(5, 0, 4,
                                    "section 5 of BUFR message (end section)");

    if (memcmp(in.data + in.sec[5], "7777", 4) != 0)
        in.parse_error(5, 0, "section 5 does not contain '7777'");

    for (unsigned i = 0; i < 5; ++i)
        out.section_end[i] = in.sec[i + 1];
    out.section_end[5] = out.section_end[4] + 4;

    // if (subsets_no != out.subsets.size())
    //     parse_error(sec5, "header advertised %u subsets but only %zu found",
    //     subsets_no, out.subsets.size());
}

/*
 * UncompressedDecoderTarget
 */

UncompressedDecoderTarget::UncompressedDecoderTarget(Input& in, Subset& out)
    : DecoderTarget(in), out(out)
{
}

const Subset& UncompressedDecoderTarget::reference_subset() const
{
    return out;
}

Varinfo UncompressedDecoderTarget::lookup_info(unsigned pos) const
{
    return out[pos].info();
}

Var UncompressedDecoderTarget::decode_uniform_b_value(Varinfo info)
{
    Var var(info);
    switch (info->type)
    {
        case Vartype::String:  in.decode_string(var); break;
        case Vartype::Binary:  in.decode_binary(var); break;
        case Vartype::Integer:
        case Vartype::Decimal: in.decode_number(var); break;
    }
    return var;
}

const Var& UncompressedDecoderTarget::decode_and_add_to_all(Varinfo info)
{
    out.store_variable(decode_uniform_b_value(info));
    return out.back();
}

const Var& UncompressedDecoderTarget::decode_and_add_bitmap(
    const Tables& tables, Varcode code, unsigned bitmap_size)
{
    // Read the bitmap
    std::string buf = in.decode_uncompressed_bitmap(bitmap_size);

    // Create a single use varinfo to store the bitmap
    Varinfo info = tables.get_bitmap(code, buf);

    // Store the bitmap
    out.store_variable(Var(info, buf));

    return out.back();
}

void UncompressedDecoderTarget::decode_and_set_attribute(Varinfo info,
                                                         unsigned pos)
{
    Var var = decode_uniform_b_value(info);
    TRACE(" define_attribute adding var %01d%02d%03d %s as attribute to "
          "%01d%02d%03d\n",
          WR_VAR_FXY(var.code()), var.enqc(), WR_VAR_FXY(out[pos].code()));
    if (!var.isset())
        return;
    out[pos].seta(std::move(var));
}

void UncompressedDecoderTarget::decode_and_add_b_value(Varinfo info)
{
    Var var = decode_uniform_b_value(info);
    out.store_variable(var);
    IFTRACE
    {
        TRACE(" define_variable decoded: ");
        out.back().print(stderr);
    }
}

void UncompressedDecoderTarget::decode_and_add_b_value_with_associated_field(
    Varinfo info, const bulletin::AssociatedField& field)
{
    /// If set, it is the associated field for the next variable to be decoded
    TRACE("decode_b_data:reading %d bits of C04 information\n",
          field.bit_count);
    uint32_t val = in.get_bits(field.bit_count);
    TRACE("decode_b_data:read C04 information %x\n", val);
    auto cur_associated_field = field.make_attribute(val);

    Var var = decode_uniform_b_value(info);
    out.store_variable(var);
    IFTRACE
    {
        TRACE(" define_variable decoded: ");
        out.back().print(stderr);
    }
    if (cur_associated_field.get())
    {
        IFTRACE
        {
            TRACE(" define_variable with associated field: ");
            cur_associated_field->print(stderr);
        }
        out.back().seta(std::move(cur_associated_field));
    }
}

void UncompressedDecoderTarget::decode_and_add_raw_character_data(Varinfo info)
{
    std::string buf;
    buf.resize(info->len);
    TRACE("decode_c_data:character data %d long\n", info->len);
    for (unsigned i = 0; i < info->len; ++i)
    {
        uint32_t bitval = in.get_bits(8);
        TRACE("decode_c_data:decoded character %d %c\n", (int)bitval,
              (char)bitval);
        buf[i] = bitval;
    }

    // Add as C variable to the subset

    // Store the character data
    Var cdata(info, buf);
    out.store_variable(cdata);

    TRACE("decode_c_data:decoded string %s\n", buf.c_str());
}

int UncompressedDecoderTarget::decode_c03_refval_override(unsigned bits)
{
    uint32_t res      = in.get_bits(bits);
    uint32_t sign_bit = 1 << (bits - 1);

    if (res & (1 << (bits - 1)))
        return -(res & ~sign_bit);
    else
        return res;
}

void UncompressedDecoderTarget::print_last_variable_added(FILE* out)
{
    this->out.back().format(out, "-");
}

void UncompressedDecoderTarget::print_last_attribute_added(FILE* out,
                                                           Varcode code,
                                                           unsigned pos)
{
    auto attr = this->out[pos].enqa(code);
    if (attr)
        attr->format(out, "-");
    else
        putc('-', out);
}

/*
 * CompressedDecoderTarget
 */

CompressedDecoderTarget::CompressedDecoderTarget(Input& in, Bulletin& out)
    : DecoderTarget(in), out(out), subset_count(out.subsets.size())
{
}

const Subset& CompressedDecoderTarget::reference_subset() const
{
    return out.subsets[0];
}

Varinfo CompressedDecoderTarget::lookup_info(unsigned pos) const
{
    return out.subset(0)[pos].info();
}

Var CompressedDecoderTarget::decode_uniform_b_value(Varinfo info)
{
    Var var(info);
    switch (info->type)
    {
        case Vartype::String: in.decode_string(var, subset_count); break;
        case Vartype::Binary: throw error_unimplemented("decode_b_binary TODO");
        case Vartype::Integer:
        case Vartype::Decimal:
            in.decode_compressed_semantic_number(var, subset_count);
            break;
    }
    return var;
}

void CompressedDecoderTarget::decode_b_value(
    Varinfo info, std::function<void(unsigned, Var&&)> dest)
{
    switch (info->type)
    {
        case Vartype::String: in.decode_string(info, subset_count, dest); break;
        case Vartype::Binary: throw error_unimplemented("decode_b_binary TODO");
        case Vartype::Integer:
        case Vartype::Decimal:
            in.decode_compressed_number(info, subset_count, dest);
            break;
    }
}

const Var& CompressedDecoderTarget::decode_and_add_to_all(Varinfo info)
{
    Var res(decode_uniform_b_value(info));
    for (unsigned i = 0; i < subset_count; ++i)
        out.subsets[i].store_variable(res);
    return out.subsets[0].back();
}

const Var& CompressedDecoderTarget::decode_and_add_bitmap(const Tables& tables,
                                                          Varcode code,
                                                          unsigned bitmap_size)
{
    // Read the bitmap
    std::string buf = in.decode_compressed_bitmap(bitmap_size);

    // Create a single use varinfo to store the bitmap
    Varinfo info = tables.get_bitmap(code, buf);

    // Store the bitmap
    Var bmp(info, buf);
    for (unsigned i = 0; i < subset_count; ++i)
        out.subsets[i].store_variable(bmp);

    return out.subsets[0].back();
}

void CompressedDecoderTarget::decode_and_set_attribute(Varinfo info,
                                                       unsigned pos)
{
    decode_b_value(info, [&](unsigned idx, Var&& var) {
        TRACE("define_attribute:seta subset[%u][%u] (%01d%02d%03d) "
              "%01d%02d%03d %s\n",
              idx, pos, WR_VAR_FXY(out.subsets[idx][pos].code()),
              WR_VAR_FXY(var.code()), var.enq("-"));
        if (!var.isset())
            return;
        out.subsets[idx][pos].seta(std::move(var));
    });
}

void CompressedDecoderTarget::decode_and_add_b_value(Varinfo info)
{
    DispatchToSubsets dest(out, subset_count);
    switch (info->type)
    {
        case Vartype::String: in.decode_string(info, subset_count, dest); break;
        case Vartype::Binary: throw error_unimplemented("decode_b_binary TODO");
        case Vartype::Integer:
        case Vartype::Decimal:
            in.decode_compressed_number(info, subset_count, dest);
            break;
    }
}

void CompressedDecoderTarget::decode_and_add_b_value_with_associated_field(
    Varinfo info, const bulletin::AssociatedField& field)
{
    DispatchToSubsets dest(out, subset_count);
    switch (info->type)
    {
        case Vartype::String: in.decode_string(info, subset_count, dest); break;
        case Vartype::Binary: throw error_unimplemented("decode_b_binary TODO");
        case Vartype::Integer:
        case Vartype::Decimal:
            in.decode_compressed_number_af(
                info, field, subset_count, [&](unsigned subset_no, Var&& var) {
                    dest.add_var(subset_no, std::move(var));
                });
            break;
    }
}

void CompressedDecoderTarget::decode_and_add_raw_character_data(Varinfo info)
{
    // TODO: if compressed, extract the data from each subset? Store it in each
    // dataset?
    error_unimplemented::throwf(
        "C05%03d character data found in compressed message and it is not "
        "clear how it should be handled",
        WR_VAR_Y(info->code));
}

int CompressedDecoderTarget::decode_c03_refval_override(unsigned bits)
{
    // TODO: if compressed, take the overrides only once? We have a compressed
    // integer value that could give a different override for each subset?
    error_unimplemented::throwf(
        "C03%03u reference value override found in compressed message and it "
        "is not clear how it should be handled",
        bits);
}

void CompressedDecoderTarget::print_last_variable_added(FILE* out)
{
    for (const auto& s : this->out.subsets)
    {
        s.back().format(out, "-");
        putc(' ', out);
    }
}

void CompressedDecoderTarget::print_last_attribute_added(FILE* out,
                                                         Varcode code,
                                                         unsigned pos)
{
    for (const auto& s : this->out.subsets)
    {
        auto attr = s[pos].enqa(code);
        if (attr)
            attr->format(out, "-");
        else
            putc('-', out);
        putc(' ', out);
    }
}

/*
 * DataSectionDecoder
 */

DataSectionDecoder::DataSectionDecoder(Bulletin& bulletin,
                                       DecoderTarget& target)
    : Interpreter(bulletin.tables, bulletin.datadesc), target(target)
{
}

unsigned
DataSectionDecoder::define_bitmap_delayed_replication_factor(Varinfo info)
{
    Var rep_count = target.decode_uniform_b_value(info);
    return rep_count.enqi();
}

unsigned DataSectionDecoder::define_delayed_replication_factor(Varinfo info)
{
    return target.decode_and_add_to_all(info).enqi();
}

unsigned DataSectionDecoder::define_associated_field_significance(Varinfo info)
{
    return target.decode_and_add_to_all(info).enq(63);
}

void DataSectionDecoder::define_c03_refval_override(Varcode code)
{
    int refval = target.decode_c03_refval_override(c03_refval_override_bits);
    c03_refval_overrides[code] = refval;
}

void DataSectionDecoder::define_bitmap(unsigned bitmap_size)
{
    TRACE("define_bitmap %d\n", bitmap_size);

    const Var& bmp = target.decode_and_add_bitmap(
        tables, bitmaps.pending_definitions, bitmap_size);

    IFTRACE
    {
        TRACE("Decoded bitmap count %u: ", bitmap_size);
        bmp.print(stderr);
        TRACE("\n");
    }

    bitmaps.define(bmp, target.reference_subset());
}

void DataSectionDecoder::define_attribute(Varinfo info, unsigned pos)
{
    target.decode_and_set_attribute(info, pos);
}

void DataSectionDecoder::define_substituted_value(unsigned pos)
{
    Varinfo info = target.lookup_info(pos);
    target.decode_and_set_attribute(info, pos);
}

void DataSectionDecoder::define_variable(Varinfo info)
{
    target.decode_and_add_b_value(info);
}

void DataSectionDecoder::define_variable_with_associated_field(Varinfo info)
{
    target.decode_and_add_b_value_with_associated_field(info, associated_field);
}

void DataSectionDecoder::define_raw_character_data(Varcode code)
{
    // Create a single use varinfo to store the bitmap
    Varinfo info = tables.get_chardata(code, WR_VAR_Y(code));
    target.decode_and_add_raw_character_data(info);
}

/*
 * VerboseDataSectionDecoder
 */

VerboseDataSectionDecoder::VerboseDataSectionDecoder(Bulletin& bulletin,
                                                     DecoderTarget& target,
                                                     FILE* out)
    : DataSectionDecoder(bulletin, target), out(out)
{
}

void VerboseDataSectionDecoder::print_lead(Varcode code)
{
    fprintf(out, "%*s%d%02d%03d ", static_cast<int>(indent), "", WR_VAR_F(code),
            WR_VAR_X(code), WR_VAR_Y(code));
}

void VerboseDataSectionDecoder::print_lead_continued()
{
    fprintf(out, "%*s       ", static_cast<int>(indent), "");
}

void VerboseDataSectionDecoder::b_variable(Varcode code)
{
    print_lead(code);
    if (tables.btable)
    {
        if (tables.btable->contains(code))
        {
            Varinfo info = tables.btable->query(code);
            fprintf(out, "%s[%s]", info->desc, info->unit);
        }
        else
            fprintf(out, "(missing in B table %s)",
                    tables.btable->path().c_str());
    }
    putc('\n', out);
    DataSectionDecoder::b_variable(code);
}

void VerboseDataSectionDecoder::c_modifier(Varcode code, Opcodes& next)
{
    print_lead(code);
    Interpreter::print_c_modifier(out, code, next);
    DataSectionDecoder::c_modifier(code, next);
}

void VerboseDataSectionDecoder::r_replication(Varcode code,
                                              Varcode delayed_code,
                                              const Opcodes& ops)
{
    print_lead(code);
    unsigned group = WR_VAR_X(code);
    unsigned count = WR_VAR_Y(code);
    fprintf(out, "replicate %u descriptors", group);
    if (count)
        fprintf(out, " %u times\n", count);
    else
        fprintf(out, " (delayed %d%02d%03d) times\n", WR_VAR_F(delayed_code),
                WR_VAR_X(delayed_code), WR_VAR_Y(delayed_code));
    indent += indent_step;
    DataSectionDecoder::r_replication(code, delayed_code, ops);
    indent -= indent_step;
}

void VerboseDataSectionDecoder::run_d_expansion(Varcode code)
{
    print_lead(code);
    fputs(" (group)\n", out);
    indent += indent_step;
    Interpreter::run_d_expansion(code);
    indent -= indent_step;
}

unsigned
VerboseDataSectionDecoder::define_delayed_replication_factor(Varinfo info)
{
    unsigned res = DataSectionDecoder::define_delayed_replication_factor(info);
    print_lead_continued();
    fprintf(out, "delayed replication factor: %u\n", res);
    return res;
}
unsigned
VerboseDataSectionDecoder::define_associated_field_significance(Varinfo info)
{
    return DataSectionDecoder::define_associated_field_significance(info);
    // TODO: print results
}
unsigned VerboseDataSectionDecoder::define_bitmap_delayed_replication_factor(
    Varinfo info)
{
    return DataSectionDecoder::define_bitmap_delayed_replication_factor(info);
    // TODO: print results
}

void VerboseDataSectionDecoder::define_c03_refval_override(Varcode code)
{
    DataSectionDecoder::define_c03_refval_override(code);
    fprintf(out, "C03 reference value override for %d%02d%03d: %u\n",
            WR_VAR_FXY(code), c03_refval_overrides[code]);
}

void VerboseDataSectionDecoder::define_bitmap(unsigned bitmap_size)
{
    DataSectionDecoder::define_bitmap(bitmap_size);
    // TODO: print results
}
void VerboseDataSectionDecoder::define_attribute(Varinfo info, unsigned pos)
{
    DataSectionDecoder::define_attribute(info, pos);
    print_lead_continued();
    fprintf(out, "attribute %d%02d%03d %s\n", WR_VAR_FXY(info->code),
            info->desc);
    Varinfo pos_info = target.lookup_info(pos);
    print_lead_continued();
    fprintf(out, "at position %u: %d%02d%03d %s\n", pos,
            WR_VAR_FXY(pos_info->code), pos_info->desc);
    print_lead_continued();
    target.print_last_attribute_added(out, info->code, pos);
    putc('\n', out);
}
void VerboseDataSectionDecoder::define_substituted_value(unsigned pos)
{
    DataSectionDecoder::define_substituted_value(pos);
    // TODO: print results
}
void VerboseDataSectionDecoder::define_variable(Varinfo info)
{
    DataSectionDecoder::define_variable(info);
    print_lead_continued();
    target.print_last_variable_added(out);
    putc('\n', out);
}
void VerboseDataSectionDecoder::define_variable_with_associated_field(
    Varinfo info)
{
    DataSectionDecoder::define_variable_with_associated_field(info);
    // TODO: print results
}
void VerboseDataSectionDecoder::define_raw_character_data(Varcode code)
{
    DataSectionDecoder::define_raw_character_data(code);
    // TODO: print results
}

} // namespace bufr
} // namespace wreport
