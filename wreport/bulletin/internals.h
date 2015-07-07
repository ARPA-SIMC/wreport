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

#ifndef WREPORT_BULLETIN_INTERNALS_H
#define WREPORT_BULLETIN_INTERNALS_H

#include <wreport/varinfo.h>
#include <wreport/opcode.h>
#include <wreport/bulletin/interpreter.h>
#include <vector>
#include <memory>
#include <cmath>

namespace wreport {
struct Var;
struct Subset;
struct Bulletin;

namespace bulletin {

/**
 * Abstract interface for classes that can be used as targets for the Bulletin
 * Data Descriptor Section interpreters.
 */
struct Parser : public bulletin::DDSInterpreter
{
    /// Current subset (used to refer to past variables)
    const Subset& current_subset;

    Parser(const Tables& tables, const Opcodes& opcodes, unsigned subset_no, const Subset& current_subset);
    virtual ~Parser();
};

/**
 * Common bulletin::Parser base for visitors that modify the bulletin.
 *
 * This assumes a fully decoded bulletin.
 */
struct BaseParser : public Parser
{
    /// Bulletin being visited
    Bulletin& bulletin;
    /// Index of the subset being visited
    unsigned current_subset_no;
    /// Index of the next variable to be visited
    unsigned current_var;

    /// Create visitor for the given bulletin
    BaseParser(Bulletin& bulletin, unsigned subset_idx);

    /// Get the next variable
    Var& get_var();
    /// Get the variable at the given position
    Var& get_var(unsigned var_pos) const;

    void define_bitmap(Varcode rep_code, Varcode delayed_code, const Opcodes& ops) override;
};

}
}

#endif
