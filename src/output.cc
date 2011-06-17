/*
 * options - BUFR and CREX I/O examples
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

struct PrintContents : public BulletinHandler
{
    /// Dump the contents of a message
    virtual void handle(const wreport::Bulletin& b)
    {
        b.print(stderr);
    }
};

struct PrintStructure : public BulletinHandler
{
    /// Dump the contents of a message, with structure
    virtual void handle(const wreport::Bulletin& b)
    {
        b.print_structured(stderr);
    }
};

struct PrintDDS : public BulletinHandler
{
    /// Dump the contents of the Data Descriptor Section a message
    virtual void handle(const wreport::Bulletin& b)
    {
        b.print_datadesc(stderr);
    }
};
