/*
 * wreport/bulletin/internals - Bulletin implementation helpers
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

#include "internals.h"
#include "var.h"
#include "subset.h"
#include "bulletin.h"
#include "notes.h"
#include <cmath>

// #define TRACE_INTERPRETER

#ifdef TRACE_INTERPRETER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {
namespace bulletin {

Bitmap::Bitmap() : bitmap(0) {}
Bitmap::~Bitmap() {}

void Bitmap::reset()
{
    bitmap = 0;
    old_anchor = 0;
    refs.clear();
    iter = refs.rend();
}

void Bitmap::init(const Var& bitmap, const Subset& subset, unsigned anchor)
{
    this->bitmap = &bitmap;
    refs.clear();

    // From the specs it looks like bitmaps refer to all data that precedes
    // the C operator that defines or uses the bitmap, but from the data
    // samples that we have it look like when multiple bitmaps are present,
    // they always refer to the same set of variables. For this reason we
    // remember the first anchor point that we see and always refer the
    // other bitmaps that we see to it.
    if (old_anchor)
        anchor = old_anchor;
    else
        old_anchor = anchor;

    unsigned b_cur = bitmap.info()->len;
    unsigned s_cur = anchor;
    if (b_cur == 0) throw error_consistency("data present bitmap has length 0");
    if (s_cur == 0) throw error_consistency("data present bitmap is anchored at start of subset");

    while (true)
    {
        --b_cur;
        --s_cur;
        while (WR_VAR_F(subset[s_cur].code()) != 0)
        {
            if (s_cur == 0) throw error_consistency("bitmap refers to variables before the start of the subset");
            --s_cur;
        }

        if (bitmap.enqc()[b_cur] == '+')
            refs.push_back(s_cur);

        if (b_cur == 0)
            break;
        if (s_cur == 0)
            throw error_consistency("bitmap refers to variables before the start of the subset");
    }

    iter = refs.rbegin();
}

bool Bitmap::eob() const { return iter == refs.rend(); }
unsigned Bitmap::next() { unsigned res = *iter; ++iter; return res; }


AssociatedField::AssociatedField() : btable(0), skip_missing(true), bit_count(0) {}
AssociatedField::~AssociatedField() {}

void AssociatedField::reset(const Vartable& btable)
{
    this->btable = &btable;
    bit_count = 0;
    significance = 63;
}

std::unique_ptr<Var> AssociatedField::make_attribute(unsigned value) const
{
    switch (significance)
    {
        case 1:
            // Add attribute B33002=value
            return unique_ptr<Var>(new Var(btable->query(WR_VAR(0, 33, 2)), (int)value));
        case 2:
            // Add attribute B33003=value
            return unique_ptr<Var>(new Var(btable->query(WR_VAR(0, 33, 3)), (int)value));
        case 3:
        case 4:
        case 5:
            // Reserved: ignored
            notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            return unique_ptr<Var>();
        case 6:
            // Add attribute B33050=value
            if (!skip_missing || value != 15)
            {
                unique_ptr<Var> res(new Var(btable->query(WR_VAR(0, 33, 50))));
                if (value != 15)
                   res->seti(value);
                return res;
            } else
                return unique_ptr<Var>();
        case 7:
            // Add attribute B33040=value
            return unique_ptr<Var>(new Var(btable->query(WR_VAR(0, 33, 40)), (int)value));
        case 8:
            // Add attribute B33002=value
            if (!skip_missing || value != 3)
            {
                unique_ptr<Var> res(new Var(btable->query(WR_VAR(0, 33, 2))));
                if (value != 3)
                   res->seti(value);
                return res;
            } else
                return unique_ptr<Var>();
        case 21:
            // Add attribute B33041=value
            if (!skip_missing || value != 1)
                return unique_ptr<Var>(new Var(btable->query(WR_VAR(0, 33, 41)), 0));
            else
                return unique_ptr<Var>();
        case 63:
            /*
             * Ignore quality information if B31021 is missing.
             * The Guide to FM94-BUFR says:
             *   If the quality information has no meaning for some
             *   of those following elements, but the field is
             *   still there, there is at present no explicit way
             *   to indicate "no meaning" within the currently
             *   defined meanings. One must either redefine the
             *   meaning of the associated field in its entirety
             *   (by including 0 31 021 in the message with a data
             *   value of 63 - "missing value") or remove the
             *   associated field bits by the "cancel" operator: 2
             *   04 000.
             */
            return unique_ptr<Var>();
        default:
            if (significance >= 9 and significance <= 20)
                // Reserved: ignored
                notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            else if (significance >= 22 and significance <= 62)
                notes::logf("Ignoring B31021=%d, which is documented as 'reserved for local use'\n",
                        significance);
            else
                error_unimplemented::throwf("C04 modifiers with B31021=%d are not supported", significance);
            return unique_ptr<Var>();
    }
}

const Var* AssociatedField::get_attribute(const Var& var) const
{
    /*
     * Query variable attribute according to significance given in CODE TABLE
     * 031021
     */
    switch (significance)
    {
        case 1:
        case 8:
            return var.enqa(WR_VAR(0, 33, 2));
            break;
        case 2:
            return var.enqa(WR_VAR(0, 33, 3));
            break;
        case 3:
        case 4:
        case 5:
            // Reserved: ignored
            notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            break;
        case 6: return var.enqa(WR_VAR(0, 33, 50)); break;
        case 7: return var.enqa(WR_VAR(0, 33, 40)); break;
        case 21: return var.enqa(WR_VAR(0, 33, 41)); break;
        case 63:
            /*
             * Ignore quality information if B31021 is missing.
             * The Guide to FM94-BUFR says:
             *   If the quality information has no meaning for some
             *   of those following elements, but the field is
             *   still there, there is at present no explicit way
             *   to indicate "no meaning" within the currently
             *   defined meanings. One must either redefine the
             *   meaning of the associated field in its entirety
             *   (by including 0 31 021 in the message with a data
             *   value of 63 - "missing value") or remove the
             *   associated field bits by the "cancel" operator: 2
             *   04 000.
             */
            break;
        default:
            if (significance >= 9 and significance <= 20)
                // Reserved: ignored
                notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            else if (significance >= 22 and significance <= 62)
                notes::logf("Ignoring B31021=%d, which is documented as 'reserved for local use'\n",
                        significance);
            else
                error_unimplemented::throwf("C04 modifiers with B31021=%d are not supported", significance);
            break;
    }
    return 0;
}


Parser::Parser(const Tables& tables, const Opcodes& opcodes) : DDSInterpreter(tables, opcodes), current_subset(0) {}
Parser::~Parser() {}

Varinfo Parser::get_varinfo(Varcode code)
{
    Varinfo peek = tables.btable->query(code);

    if (!c_scale_change && !c_width_change && !c_string_len_override && !c_scale_ref_width_increase)
        return peek;

    int scale = peek->scale;
    if (c_scale_change)
    {
        TRACE("get_varinfo:applying %d scale change\n", c_scale_change);
        scale += c_scale_change;
    }

    int bit_len = peek->bit_len;
    if (peek->type == Vartype::String && c_string_len_override)
    {
        TRACE("get_varinfo:overriding string to %d bytes\n", c_string_len_override);
        bit_len = c_string_len_override * 8;
    }
    else if (c_width_change)
    {
        TRACE("get_varinfo:applying %d width change\n", c_width_change);
        bit_len += c_width_change;
    }

    if (c_scale_ref_width_increase)
    {
        TRACE("get_varinfo:applying %d increase of scale, ref, width\n", c_scale_ref_width_increase);
        // TODO: misses reference value adjustment
        scale += c_scale_ref_width_increase;
        bit_len += (10 * c_scale_ref_width_increase + 2) / 3;
        // c_ref *= 10**code
    }

    TRACE("get_info:requesting alteration scale:%d, bit_len:%d\n", scale, bit_len);
    return tables.btable->query_altered(code, scale, bit_len);
}

void Parser::b_variable(Varcode code)
{
    Varinfo info = get_varinfo(code);
    // Choose which value we should encode
    if (WR_VAR_F(code) == 0 && WR_VAR_X(code) == 33 && !bitmap.eob())
    {
        // Attribute of the variable pointed by the bitmap
        unsigned target = bitmap.next();
        TRACE("b_variable attribute %01d%02d%03d subset pos %u\n",
                WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code), target);
        do_attr(info, target, code);
    } else {
        // Proper variable
        TRACE("b_variable variable %01d%02d%03d\n",
                WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var));
        do_var(info);
        ++data_pos;
    }
}


void Parser::c_modifier(Varcode code)
{
    TRACE("C DATA %01d%02d%03d\n", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));
}

void Parser::c_change_data_width(Varcode code, int change)
{
    TRACE("Set width change from %d to %d\n", c_width_change, change);
    c_width_change = change;
}

void Parser::c_change_data_scale(Varcode code, int change)
{
    TRACE("Set scale change from %d to %d\n", c_scale_change, change);
    c_scale_change = change;
}

void Parser::c_increase_scale_ref_width(Varcode code, int change)
{
    TRACE("Increase scale, reference value and data width by %d\n", change);
    c_scale_ref_width_increase = change;
}

void Parser::c_associated_field(Varcode code, Varcode sig_code, unsigned nbits)
{
    // Add associated field
    TRACE("Set C04 bits to %d\n", WR_VAR_Y(code));
    // FIXME: nested C04 modifiers are not currently implemented
    if (WR_VAR_Y(code) && associated_field.bit_count)
        throw error_unimplemented("nested C04 modifiers are not yet implemented");
    if (WR_VAR_Y(code) > 32)
        error_unimplemented::throwf("C04 modifier wants %d bits but only at most 32 are supported", WR_VAR_Y(code));
    if (WR_VAR_Y(code))
    {
        // Get encoding informations for this associated_field_significance
        Varinfo info = tables.btable->query(WR_VAR(0, 31, 21));

        // Encode B31021
        const Var& var = do_semantic_var(info);
        associated_field.significance = var.enq(63);
        ++data_pos;
    }
    associated_field.bit_count = WR_VAR_Y(code);
}

void Parser::c_char_data(Varcode code)
{
    do_char_data(code);
}

void Parser::c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits)
{
    // Length of next local descriptor
    if (WR_VAR_Y(code))
    {
        bool skip = true;
        if (tables.btable->contains(desc_code))
        {
            Varinfo info = get_varinfo(desc_code);
            if (info->bit_len == WR_VAR_Y(code))
            {
                // If we can resolve the descriptor and the size is the
                // same, attempt decoding
                do_var(info);
                skip = false;
            }
        }
        if (skip)
        {
            Varinfo info = tables.get_unknown(desc_code, WR_VAR_Y(code));
            do_var(info);
        }
        ++data_pos;
    }
}

void Parser::c_char_data_override(Varcode code, unsigned new_length)
{
    IFTRACE {
        if (new_length)
            TRACE("decode_c_data:character size overridden to %d chars for all fields\n", new_length);
        else
            TRACE("decode_c_data:character size overridde end\n");
    }
    c_string_len_override = new_length;
}

void Parser::c_quality_information_bitmap(Varcode code)
{
    // Quality information
    if (WR_VAR_Y(code) != 0)
        error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                    WR_VAR_F(code),
                    WR_VAR_X(code),
                    WR_VAR_Y(code));
    want_bitmap = code;
}

void Parser::c_substituted_value_bitmap(Varcode code)
{
    want_bitmap = code;
}

void Parser::c_substituted_value(Varcode code)
{
    if (bitmap.bitmap == NULL)
        error_consistency::throwf("found C23255 with no active bitmap");
    if (bitmap.eob())
        error_consistency::throwf("found C23255 while at the end of active bitmap");
    unsigned target = bitmap.next();
    // Use the details of the corrisponding variable for decoding
    Varinfo info = (*current_subset)[target].info();
    // Encode the value
    do_attr(info, target, info->code);
}

/* If using delayed replication and count is not -1, use count for the delayed
 * replication factor; else, look for a delayed replication factor among the
 * input variables */
void Parser::r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
{
    // unsigned group = WR_VAR_X(code);
    unsigned count = WR_VAR_Y(code);

    IFTRACE{
        TRACE("visitor r_replication %01d%02d%03d, %u times, %u opcodes: ",
                WR_VAR_F(delayed_code), WR_VAR_X(delayed_code), WR_VAR_Y(delayed_code), count, WR_VAR_X(code));
        ops.print(stderr);
        TRACE("\n");
    }

    if (want_bitmap)
    {
        if (count == 0 && delayed_code == 0)
            delayed_code = WR_VAR(0, 31, 12);
        const Var& bitmap_var = do_bitmap(want_bitmap, code, delayed_code, ops);
        bitmap.init(bitmap_var, *current_subset, data_pos);
        if (delayed_code)
            ++data_pos;
        want_bitmap = 0;
    } else {
        if (count == 0)
        {
            Varinfo info = tables.btable->query(delayed_code ? delayed_code : WR_VAR(0, 31, 12));
            const Var& var = do_semantic_var(info);
            if (var.code() == WR_VAR(0, 31, 0))
            {
                count = var.isset() ? 0 : 1;
            } else {
                count = var.enqi();
            }
            ++data_pos;
        }
        IFTRACE {
            TRACE("visitor r_replication %d items %d times%s\n", WR_VAR_X(code), count, delayed_code ? " (delayed)" : "");
            TRACE("Repeat opcodes: ");
            ops.print(stderr);
            TRACE("\n");
        }

        // encode_data_section on it `count' times
        for (unsigned i = 0; i < count; ++i)
        {
            do_start_repetition(i);
            opcode_stack.push(ops);
            run();
            opcode_stack.pop();
        }
    }
}

void Parser::do_start_subset(unsigned subset_no, const Subset& current_subset)
{
    TRACE("visit: start encoding subset %u\n", subset_no);

    this->current_subset = &current_subset;

    c_scale_change = 0;
    c_width_change = 0;
    c_string_len_override = 0;
    c_scale_ref_width_increase = 0;
    bitmap.reset();
    associated_field.reset(*tables.btable);
    want_bitmap = 0;
    data_pos = 0;
}

void Parser::do_start_repetition(unsigned idx) {}



BaseParser::BaseParser(Bulletin& bulletin)
    : Parser(bulletin.tables, bulletin.datadesc), bulletin(bulletin), current_subset_no(0)
{
}

Var& BaseParser::get_var()
{
    Var& res = get_var(current_var);
    ++current_var;
    return res;
}

Var& BaseParser::get_var(unsigned var_pos) const
{
    unsigned max_var = current_subset->size();
    if (var_pos >= max_var)
        error_consistency::throwf("requested variable #%u out of a maximum of %u in subset %u",
                var_pos, max_var, current_subset_no);
    return bulletin.subsets[current_subset_no][var_pos];
}

void BaseParser::do_start_subset(unsigned subset_no, const Subset& current_subset)
{
    Parser::do_start_subset(subset_no, current_subset);
    if (subset_no >= bulletin.subsets.size())
        error_consistency::throwf("requested subset #%u out of a maximum of %zd", subset_no, bulletin.subsets.size());
    this->current_subset = &(bulletin.subsets[subset_no]);
    current_subset_no = subset_no;
    current_var = 0;
}

const Var& BaseParser::do_bitmap(Varcode code, Varcode rep_code, Varcode delayed_code, const Opcodes& ops)
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    return var;
}

}
}
