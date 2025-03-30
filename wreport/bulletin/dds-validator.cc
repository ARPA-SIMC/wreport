#include "wreport/bulletin/dds-validator.h"
#include "wreport/vartable.h"

using namespace std;

namespace wreport {
namespace bulletin {

DDSValidator::DDSValidator(const Bulletin& b, unsigned subset_idx)
    : UncompressedEncoder(b, subset_idx)
{
    is_crex = dynamic_cast<const CrexBulletin*>(&b) != NULL;
}

void DDSValidator::check_fits(Varinfo info, const Var& var)
{
    if (var.code() != info->code)
        error_consistency::throwf("input variable %d%02d%03d differs from "
                                  "expected variable %d%02d%03d",
                                  WR_VAR_F(var.code()), WR_VAR_X(var.code()),
                                  WR_VAR_Y(var.code()), WR_VAR_F(info->code),
                                  WR_VAR_X(info->code), WR_VAR_Y(info->code));

    if (!var.isset())
        ;
    else
        switch (info->type)
        {
            case Vartype::String:  break;
            case Vartype::Binary:  break;
            case Vartype::Integer:
            case Vartype::Decimal: {
                unsigned encoded;
                if (is_crex)
                    encoded = info->encode_decimal(var.enqd());
                else
                    encoded = info->encode_binary(var.enqd());
                if (!is_crex)
                {
                    if (encoded >= (1u << info->bit_len))
                        error_consistency::throwf(
                            "value %f (%u) does not fit in %u bits", var.enqd(),
                            encoded, info->bit_len);
                }
            }
            break;
        }
}

void DDSValidator::check_attr(Varinfo info, unsigned var_pos)
{
    const Var& var = get_var(var_pos);
    if (const Var* a = var.enqa(info->code))
        check_fits(info, *a);
}

void DDSValidator::encode_var(Varinfo info, const Var& var)
{
    check_fits(info, var);
}

void DDSValidator::define_substituted_value(unsigned pos)
{
    // Use the details of the corrisponding variable for decoding
    Varinfo info = current_subset[pos].info();
    check_attr(info, pos);
}

void DDSValidator::define_attribute(Varinfo info, unsigned pos)
{
    check_attr(info, pos);
}

void DDSValidator::define_raw_character_data(Varcode code)
{
    const Var& var = get_var();
    if (var.code() != code)
        error_consistency::throwf("input variable %d%02d%03d differs from "
                                  "expected variable %d%02d%03d",
                                  WR_VAR_F(var.code()), WR_VAR_X(var.code()),
                                  WR_VAR_Y(var.code()), WR_VAR_F(code),
                                  WR_VAR_X(code), WR_VAR_Y(code));
}

void DDSValidator::define_c03_refval_override(Varcode code)
{
    // Scan the subset looking for a variables with the given code, to see what
    // is its bit_ref
    bool found  = false;
    int bit_ref = 0;
    for (const auto& var : current_subset)
    {
        Varinfo info = var.info();
        if (info->code == code)
        {
            bit_ref = info->bit_ref;
            found   = true;
            break;
        }
    }

    if (!found)
    {
        // If not found, take the default
        Varinfo info = current_subset.tables->btable->query(code);
        bit_ref      = info->bit_ref;
    }

    // Encode
    uint32_t encoded;
    unsigned nbits;
    if (bit_ref < 0)
    {
        encoded = -bit_ref;
        nbits   = 32 - __builtin_clz(encoded);
        encoded |= 1 << (c03_refval_override_bits - 1);
    }
    else
    {
        encoded = bit_ref;
        nbits   = 32 - __builtin_clz(encoded);
    }

    // Check if it fits (encoded bits plus 1 for the sign)
    if (nbits + 1 > c03_refval_override_bits)
        error_consistency::throwf(
            "C03 reference value override requested for value %d, encoded as "
            "%u, which does not fit in %u bits (requires %u bits)",
            bit_ref, encoded, c03_refval_override_bits, nbits + 1);

    c03_refval_overrides[code] = bit_ref;
}

} // namespace bulletin
} // namespace wreport

/* vim:set ts=4 sw=4: */
