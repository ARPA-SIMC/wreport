/*
 * iterate - iterate bulletin contents
 *
 * Copyright (C) 2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

namespace {

struct PrintVars : public BulletinFullHandler
{
    FILE* out;
    const std::vector<wreport::Varcode>& codes;

    PrintVars(const std::vector<wreport::Varcode>& codes, FILE* out = stdout)
        : out(out), codes(codes)
    {
    }

    const Var* find_varcode(const wreport::Subset& subset, Varcode code)
    {
        for (size_t i = 0; i < subset.size(); ++i)
            if (subset[i].code() == code)
                return &subset[i];
        return NULL;
    }

    /// Dump the contents of a message
    void handle(wreport::Bulletin& b) override
    {
        for (size_t sset = 0; sset < b.subsets.size(); ++sset)
        {
            fprintf(out, "%s:%zu:", b.fname.c_str(), sset + 1);
            for (size_t i = 0; i < codes.size(); ++i)
            {
                const Var* var = find_varcode(b.subsets[sset], codes[i]);
                if (var)
                {
                    string formatted = var->format();
                    fprintf(out, "\t%s", formatted.c_str());
                }
            }
            putc('\n', out);
        }
    }
};

} // namespace
