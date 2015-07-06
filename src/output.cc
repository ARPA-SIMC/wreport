/*
 * output - output bulletin contents
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

#include <wreport/bulletin.h>
#include "options.h"
#include <cstring>

struct PrintContents : public BulletinFullHandler
{
    FILE* out;
    PrintContents(FILE* out=stderr) : out(out) {}

    /// Dump the contents of a message
    void handle(wreport::Bulletin& b) override
    {
        b.print(out);
    }
};

struct PrintStructure : public BulletinFullHandler
{
    FILE* out;
    PrintStructure(FILE* out=stderr) : out(out) {}

    /// Dump the contents of a message, with structure
    void handle(wreport::Bulletin& b) override
    {
        b.print_structured(out);
    }
};

struct PrintDDS : public BulletinHeadHandler
{
    FILE* out;
    PrintDDS(FILE* out=stderr) : out(out) {}

    /// Dump the contents of the Data Descriptor Section a message
    void handle(wreport::Bulletin& b) override
    {
        b.print_datadesc(out);
    }
};

struct PrintTables : public BulletinHeadHandler
{
    FILE* out;
    bool header_printed;
    PrintTables(FILE* out=stderr) : out(out), header_printed(false) {}

    /// Dump the contents of the Data Descriptor Section a message
    void handle(wreport::Bulletin& b) override
    {
        if (const BufrBulletin* m = dynamic_cast<const BufrBulletin*>(&b))
        {
            if (!header_printed)
            {
                fprintf(out, "%-*s\tOffset\tCentre\tSubc.\tMaster\tLocal\n", (int)strlen(b.fname), "Filename");
                header_printed = true;
            }
            fprintf(out, "%s\t%zd\t%d\t%d\t%d\t%d\n",
                    b.fname, b.offset,
                    m->centre, m->subcentre,
                    m->master_table, m->local_table);
        }
        else if (const CrexBulletin* m = dynamic_cast<const CrexBulletin*>(&b))
        {
            if (!header_printed)
            {
                fprintf(out, "Filename\tOffset\tMaster\tEdition\tTable\n");
                header_printed = true;
            }
            fprintf(out, "%s\t%zd\t%d\t%d\t%d\n",
                    b.fname, b.offset,
                    m->master_table_number, m->edition, m->table);
        }
        else
        {
            fprintf(out, "%s\t%zd\tunknown message type\n",
                    b.fname, b.offset);
        }
    }
};
