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


AssociatedField::AssociatedField(const Vartable& btable)
    : btable(btable), skip_missing(true), bit_count(0), significance(63)
{
}
AssociatedField::~AssociatedField() {}

std::unique_ptr<Var> AssociatedField::make_attribute(unsigned value) const
{
    switch (significance)
    {
        case 1:
            // Add attribute B33002=value
            return unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 2)), (int)value));
        case 2:
            // Add attribute B33003=value
            return unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 3)), (int)value));
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
                unique_ptr<Var> res(new Var(btable.query(WR_VAR(0, 33, 50))));
                if (value != 15)
                   res->seti(value);
                return res;
            } else
                return unique_ptr<Var>();
        case 7:
            // Add attribute B33040=value
            return unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 40)), (int)value));
        case 8:
            // Add attribute B33002=value
            if (!skip_missing || value != 3)
            {
                unique_ptr<Var> res(new Var(btable.query(WR_VAR(0, 33, 2))));
                if (value != 3)
                   res->seti(value);
                return res;
            } else
                return unique_ptr<Var>();
        case 21:
            // Add attribute B33041=value
            if (!skip_missing || value != 1)
                return unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 41)), 0));
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


Parser::Parser(const Tables& tables, const Opcodes& opcodes, unsigned subset_no, const Subset& current_subset)
    : DDSInterpreter(tables, opcodes), current_subset(current_subset), associated_field(*tables.btable)
{
    TRACE("parser: start on subset %u\n", subset_no);
}

Parser::~Parser() {}

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
        const Var& var = define_semantic_variable(info);
        associated_field.significance = var.enq(63);
        ++bitmaps.next_bitmap_anchor_point;
    }
    associated_field.bit_count = WR_VAR_Y(code);
}

void Parser::define_substituted_value(unsigned pos)
{
    // Use the details of the corrisponding variable for decoding
    Varinfo info = current_subset[pos].info();
    do_attr(info, pos, info->code);
}

void Parser::define_attribute(Varinfo info, unsigned pos)
{
    do_attr(info, pos, info->code);
}

BaseParser::BaseParser(Bulletin& bulletin, unsigned subset_no)
    : Parser(bulletin.tables, bulletin.datadesc, subset_no, bulletin.subset(subset_no)), bulletin(bulletin), current_subset_no(0)
{
    current_subset_no = subset_no;
    current_var = 0;
}

Var& BaseParser::get_var()
{
    Var& res = get_var(current_var);
    ++current_var;
    return res;
}

Var& BaseParser::get_var(unsigned var_pos) const
{
    unsigned max_var = current_subset.size();
    if (var_pos >= max_var)
        error_consistency::throwf("requested variable #%u out of a maximum of %u in subset %u",
                var_pos, max_var, current_subset_no);
    return bulletin.subsets[current_subset_no][var_pos];
}

void BaseParser::define_bitmap(Varcode rep_code, Varcode delayed_code, const Opcodes& ops)
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    bitmaps.define(var, current_subset);
}

}
}
