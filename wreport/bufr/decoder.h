#ifndef WREPORT_BUFR_DECODER_H
#define WREPORT_BUFR_DECODER_H

#include <wreport/var.h>
#include <wreport/bulletin.h>
#include <wreport/bulletin/interpreter.h>
#include <wreport/bufr/input.h>

namespace wreport {
namespace bufr {
struct DispatchToSubsets;

struct Decoder
{
    /// Input data
    Input in;
    /* Output decoded variables */
    BufrBulletin& out;
    /// Number of expected subsets (read in decode_header, used in decode_data)
    size_t expected_subsets;
    /// True if undefined attributes are added to the output, else false
    bool conf_add_undef_attrs = false;
    /// Optional section length decoded from the message
    unsigned optional_section_length = 0;

    Decoder(const std::string& buf, const char* fname, size_t offset, BufrBulletin& out);

    void read_options(const BufrCodecOptions& opts);

    void decode_sec1ed3();
    void decode_sec1ed4();

    /* Decode the message header only */
    void decode_header();

    /* Decode message data section after the header has been decoded */
    void decode_data();
};

struct DecoderTarget
{
    /// Input buffer
    Input& in;

    DecoderTarget(Input& in) : in(in) {}
    virtual ~DecoderTarget() {}
};

struct UncompressedDecoderTarget : public DecoderTarget
{
    /// Subset where decoded variables go
    Subset& out;

    UncompressedDecoderTarget(Input& in, Subset& out);
};

struct CompressedDecoderTarget : public DecoderTarget
{
    /// Output bulletin
    Bulletin& out;

    /// Number of subsets in data section
    unsigned subset_count;

    CompressedDecoderTarget(Input& in, Bulletin& out);
};

struct DataSectionDecoder : public bulletin::Interpreter
{
    /// Input buffer
    Input& in;

    /// Output bulletin
    Bulletin& output_bulletin;

    DataSectionDecoder(Bulletin& bulletin, Input& in);

    virtual DecoderTarget& target() = 0;

    //virtual Var decode_semantic_b_value(Varinfo info);
};

/// Decoder for uncompressed data
struct UncompressedBufrDecoder : public DataSectionDecoder
{
    UncompressedDecoderTarget m_target;
    unsigned subset_no;

    /// If set, it is the associated field for the next variable to be decoded
    Var* cur_associated_field = nullptr;

    UncompressedBufrDecoder(Bulletin& bulletin, unsigned subset_no, Input& in);
    ~UncompressedBufrDecoder();

    DecoderTarget& target() override { return m_target; }

    Var decode_b_value(Varinfo info);
    void define_substituted_value(unsigned pos) override;
    void define_attribute(Varinfo info, unsigned pos) override;

    /**
     * Request processing, according to \a info, of a data variable.
     */
    void define_variable(Varinfo info) override;

    /**
     * Request processing of C05yyy character data
     */
    void define_raw_character_data(Varcode code);

    unsigned define_delayed_replication_factor(Varinfo info) override;

    unsigned define_associated_field_significance(Varinfo info) override;

    unsigned define_bitmap_delayed_replication_factor(Varinfo info) override;

    void define_bitmap(unsigned bitmap_size) override;
};

/// Decoder for compressed data
struct CompressedBufrDecoder : public DataSectionDecoder
{
    CompressedDecoderTarget m_target;
    /// Number of subsets in data section
    unsigned subset_count;

    CompressedBufrDecoder(BufrBulletin& bulletin, Input& in);

    DecoderTarget& target() override { return m_target; }

    void decode_b_value(Varinfo info, DispatchToSubsets& dest);

    void decode_b_value(Varinfo info, std::function<void(unsigned, Var&&)> dest);

    /**
     * Decode a value that must always be the same acrosso all datasets.
     *
     * @returns the decoded value
     */
    Var decode_semantic_b_value(Varinfo info);

    /// Add \a var to all datasets
    void add_to_all(const Var& var);

    void define_variable(Varinfo info) override;
    void define_substituted_value(unsigned pos) override;
    void define_attribute(Varinfo info, unsigned pos) override;
    void define_raw_character_data(Varcode code) override;
    unsigned define_delayed_replication_factor(Varinfo info) override;
    unsigned define_associated_field_significance(Varinfo info) override;
    unsigned define_bitmap_delayed_replication_factor(Varinfo info) override;
    void define_bitmap(unsigned bitmap_size) override;
};

}
}

#endif
