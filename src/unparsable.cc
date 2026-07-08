/*
 * unparsable - write to standard output only the unparsable bulletins
 *
 * Copyright (C) 2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

struct CopyUnparsable : public RawHandler
{
    FILE* out;
    FILE* log;
    unsigned unparsed;

    CopyUnparsable(FILE* out, FILE* log = 0) : out(out), log(log), unparsed(0)
    {
    }

    void handle_raw_bufr(const std::string& raw_data, const char* fname,
                         long offset) override
    {
        try
        {
            BufrBulletin::decode(raw_data, fname, offset);
        }
        catch (std::exception& e)
        {
            if (log)
                fprintf(log, "%s\n", e.what());
            fwrite(raw_data.data(), raw_data.size(), 1, out);
            ++unparsed;
        }
    }

    void handle_raw_crex(const std::string& raw_data, const char* fname,
                         long offset) override
    {
        try
        {
            CrexBulletin::decode(raw_data, fname, offset);
        }
        catch (std::exception& e)
        {
            if (log)
                fprintf(log, "%s\n", e.what());
            fwrite(raw_data.data(), raw_data.size(), 1, out);
            ++unparsed;
        }
    }
};
