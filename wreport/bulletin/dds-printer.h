#ifndef WREPORT_BULLETIN_DDS_PRINTER_H
#define WREPORT_BULLETIN_DDS_PRINTER_H

#include <cstdio>
#include <vector>
#include <wreport/bulletin.h>
#include <wreport/bulletin/internals.h>

namespace wreport {
namespace bulletin {

/**
 * Interpreter that prints the bulletin contents and its structure
 */
class DDSPrinter : public UncompressedEncoder
{
    std::vector<Varcode> stack;
    FILE* out;
    unsigned subset_no;

    void print_context(Varinfo info, unsigned var_pos);
    void print_context(Varcode code, unsigned var_pos);
    void print_attr(Varinfo info, unsigned var_pos);

public:
    /**
     * Create a new DDS printer
     *
     * @param b
     *   Reference to the bulletin being visited
     * @param out
     *   FILE to print to
     */
    DDSPrinter(const Bulletin& b, FILE* out, unsigned subset_idx);
    virtual ~DDSPrinter();

    void define_bitmap(unsigned bitmap_size) override;
    void define_substituted_value(unsigned pos) override;
    void define_attribute(Varinfo info, unsigned pos) override;
    void define_raw_character_data(Varcode code) override;
    void encode_var(Varinfo info, const Var& var) override;
    void encode_associated_field(const Var& var) override;

    void r_replication(Varcode code, Varcode delayed_code,
                       const Opcodes& ops) override;
    void run_d_expansion(Varcode code) override;
};

} // namespace bulletin
} // namespace wreport
#endif
