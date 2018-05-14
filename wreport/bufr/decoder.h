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

    /**
     * Return the reference to a subset that is receiving the data currently
     * decoded.
     *
     * For uncompressed decoders, this is the current subset. For compressed
     * decoders, it is the first subset.
     */
    virtual const Subset& reference_subset() const = 0;

    /**
     * Return information about a value previously stored at the given position
     */
    virtual Varinfo lookup_info(unsigned pos) const = 0;

    /**
     * Decode a value that must always be the same across all datasets.
     *
     * Do not add it to the output.
     *
     * @returns the decoded value
     */
    virtual Var decode_uniform_b_value(Varinfo info) = 0;

    /**
     * Decode and add the same value to all datasets, return a reference to one
     * of the variables added
     */
    virtual const Var& decode_and_add_to_all(Varinfo info) = 0;

    virtual const Var& decode_and_add_bitmap(const Tables& tables, Varcode code, unsigned bitmap_size) = 0;

    /**
     * Decode an attribute with the given description, and add it to data at
     * position pos
     */
    virtual void decode_and_set_attribute(Varinfo info, unsigned pos) = 0;

    /**
     * Decode a B-table value and add its value(s) to the target subset(s)
     */
    virtual void decode_and_add_b_value(Varinfo info) = 0;

    /**
     * Decode a B-table value with associated field, and add its value(s) to
     * the target subset(s)
     */
    virtual void decode_and_add_b_value_with_associated_field(Varinfo info, const bulletin::AssociatedField& field) = 0;

    /**
     * Decode raw character data described by \a code and add it to the target
     * subset(s)
     */
    virtual void decode_and_add_raw_character_data(Varinfo info) = 0;
};

struct UncompressedDecoderTarget : public DecoderTarget
{
    /// Subset where decoded variables go
    Subset& out;

    UncompressedDecoderTarget(Input& in, Subset& out);

    const Subset& reference_subset() const override;
    Varinfo lookup_info(unsigned pos) const override;
    Var decode_uniform_b_value(Varinfo info) override;
    const Var& decode_and_add_to_all(Varinfo info) override;
    const Var& decode_and_add_bitmap(const Tables& tables, Varcode code, unsigned bitmap_size) override;
    void decode_and_set_attribute(Varinfo info, unsigned pos) override;
    void decode_and_add_b_value(Varinfo info) override;
    void decode_and_add_b_value_with_associated_field(Varinfo info, const bulletin::AssociatedField& field) override;
    void decode_and_add_raw_character_data(Varinfo info) override;
};

struct CompressedDecoderTarget : public DecoderTarget
{
    /// Output bulletin
    Bulletin& out;

    /// Number of subsets in data section
    unsigned subset_count;

    CompressedDecoderTarget(Input& in, Bulletin& out);

    const Subset& reference_subset() const override;
    Varinfo lookup_info(unsigned pos) const override;
    Var decode_uniform_b_value(Varinfo info) override;
    const Var& decode_and_add_to_all(Varinfo info) override;
    const Var& decode_and_add_bitmap(const Tables& tables, Varcode code, unsigned bitmap_size) override;
    void decode_and_set_attribute(Varinfo info, unsigned pos) override;
    void decode_and_add_b_value(Varinfo info) override;
    void decode_and_add_b_value_with_associated_field(Varinfo info, const bulletin::AssociatedField& field) override;
    void decode_and_add_raw_character_data(Varinfo info) override;

protected:
    void decode_b_value(Varinfo info, std::function<void(unsigned, Var&&)> dest);
};

struct DataSectionDecoder : public bulletin::Interpreter
{
    DecoderTarget& target;

    DataSectionDecoder(Bulletin& bulletin, DecoderTarget& target);

    unsigned define_delayed_replication_factor(Varinfo info) override;
    unsigned define_associated_field_significance(Varinfo info) override;
    unsigned define_bitmap_delayed_replication_factor(Varinfo info) override;
    void define_bitmap(unsigned bitmap_size) override;
    void define_attribute(Varinfo info, unsigned pos) override;
    void define_substituted_value(unsigned pos) override;
    void define_variable(Varinfo info) override;
    void define_variable_with_associated_field(Varinfo info) override;
    void define_raw_character_data(Varcode code);
};

}
}
#endif
