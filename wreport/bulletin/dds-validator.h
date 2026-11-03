/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

// Copyright (C) 2011--2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef WREPORT_BULLETIN_DDS_VALIDATOR_H
#define WREPORT_BULLETIN_DDS_VALIDATOR_H

#include <vector>
#include <wreport/bulletin.h>
#include <wreport/bulletin/internals.h>

namespace wreport {
namespace bulletin {

/**
 * Interpreter that checks if the data that has been added to the bulletin
 * subsets can actualy be encoded matching the bulletin Data Descriptor
 * Section.
 */
class DDSValidator : public UncompressedEncoder
{
    bool is_crex;
    void check_fits(Varinfo info, const Var& var);
    void check_attr(Varinfo info, unsigned var_pos);

public:
    /**
     * Create a new DDS validator
     *
     * @param b
     *   Reference to the bulletin being visited
     */
    DDSValidator(const Bulletin& b, unsigned subset_idx);

    // void define_bitmap(unsigned bitmap_size) override;
    void define_substituted_value(unsigned pos) override;
    void define_attribute(Varinfo info, unsigned pos) override;
    void define_raw_character_data(Varcode code) override;
    void define_c03_refval_override(Varcode code) override;
    void encode_var(Varinfo info, const Var& var) override;
};

} // namespace bulletin
} // namespace wreport

#endif
