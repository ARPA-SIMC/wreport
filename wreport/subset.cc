/*
 * wreport/subset - Data subset for BUFR and CREX messages
 *
 * Copyright (C) 2005--2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include <config.h>

#include "subset.h"
#include "bulletin.h"
#include "notes.h"
#include "bulletin/internals.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

namespace wreport {

Subset::Subset(Bulletin* bulletin) : bulletin(bulletin)
{
    if (!bulletin->btable) throw error_consistency("B tables not loaded");
}
Subset::~Subset() {}

void Subset::store_variable(const Var& var)
{
	push_back(var);
}

void Subset::store_variable(Varcode code, const Var& var)
{
    Varinfo info = bulletin->btable->query(code);
    push_back(Var(info, var));
}

void Subset::store_variable_i(Varcode code, int val)
{
    Varinfo info = bulletin->btable->query(code);
    push_back(Var(info, val));
}

void Subset::store_variable_d(Varcode code, double val)
{
    Varinfo info = bulletin->btable->query(code);
    push_back(Var(info, val));
}

void Subset::store_variable_c(Varcode code, const char* val)
{
    Varinfo info = bulletin->btable->query(code);
    push_back(Var(info, val));
}

void Subset::store_variable_undef(Varcode code)
{
    Varinfo info = bulletin->btable->query(code);
    push_back(Var(info));
}

void Subset::append_c_with_dpb(Varcode ccode, int count, const char* bitmap)
{
    Varinfo info = bulletin->local_vartable->get_bitmap_entry(ccode, count);

	/* Create the Var with the bitmap */
	Var var(info, bitmap);

	/* Store the variable in the subset */
	store_variable(var);
}

int Subset::append_dpb(Varcode ccode, unsigned size, Varcode attr)
{
	char bitmap[size + 1];
	size_t src, dst;
	size_t count = 0;

	/* Scan first 'size' variables checking for the presence of 'attr' */
	for (src = 0, dst = 0; src < this->size() && dst < size; ++dst, ++src)
	{
		/* Skip extra, special vars */
		while (src < this->size() && WR_VAR_F((*this)[src].code()) != 0)
			++src;

#if 0
		dba_varcode code = dba_var_code(subset->vars[i]);
		/* Skip over special data like delayed repetition counts */
		if (WR_VAR_F(code) != 0 ||
		    (WR_VAR_F(code) == 0 && WR_VAR_X(code) == 31))
			continue;
#endif

		/* Check if the variable has the attribute we want */
		if ((*this)[src].enqa(attr) == NULL)
			bitmap[dst] = '-';
		else
		{
			bitmap[dst] = '+';
			++count;
		}
	}
	bitmap[size] = 0;

	// Append the bitmap to the message
	append_c_with_dpb(ccode, size, bitmap);

	return count;
}

void Subset::append_fixed_dpb(Varcode ccode, int size)
{
	char bitmap[size + 1];

	memset(bitmap, '+', size);
	bitmap[size] = 0;

	append_c_with_dpb(ccode, size, bitmap);
}

#if 0
dba_err bufrex_subset_append_attrs(bufrex_subset subset, int size, dba_varcode attr)
{
	int i;
	int repcount_idx;
	int added = 0;

	/* Add delayed repetition count with an initial value of 0, and mark its position */
	bufrex_subset_store_variable_i(subset, WR_VAR(0, 31, 2), 0);
	repcount_idx = subset->vars_count - 1;
	
	/* Scan first 'size' variables checking for the presence of 'attr' */
	for (i = 0; i < subset->vars_count && size > 0; i++)
	{
		dba_var var_attr;

#if 0
		/* Skip over special data like delayed repetition counts */
		if (WR_VAR_F(dba_var_code(subset->vars[i])) != 0)
			continue;
#endif

		/* Check if the variable has the attribute we want */
		DBA_RUN_OR_RETURN(dba_var_enqa(subset->vars[i], attr, &var_attr));
		if (var_attr != NULL)
		{
			DBA_RUN_OR_RETURN(bufrex_subset_store_variable_var(subset, attr, var_attr));
			added++;
		}
		size--;
	}

	/* Set the repetition count with the number of variables we added */
	DBA_RUN_OR_RETURN(dba_var_seti(subset->vars[repcount_idx], added));

	return dba_error_ok();
}
#endif

void Subset::print(FILE* out) const
{
	for (unsigned i = 0; i < size(); ++i)
	{
		fprintf(out, "%d ", i);
		(*this)[i].print(out);
	}
}

unsigned Subset::diff(const Subset& s2) const
{
    // Compare btables
    if (bulletin->btable->pathname() != s2.bulletin->btable->pathname())
    {
        notes::logf("B tables differ (first is %s, second is %s)\n",
                bulletin->btable->pathname().c_str(), s2.bulletin->btable->pathname().c_str());
        return 1;
    }

    // Compare vars
    if (size() != s2.size())
    {
        notes::logf("Number of variables differ (first is %zd, second is %zd)\n",
                size(), s2.size());
        return 1;
    }
    for (size_t i = 0; i < size(); ++i)
    {
        unsigned diff = (*this)[i].diff(s2[i]);
        if (diff > 0) return diff;
    }
    return 0;
}

}

/* vim:set ts=4 sw=4: */
