/*
 * options - wrep runtime configuration
 *
 * Copyright (C) 2011--2015  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "options.h"
#include <wreport/bulletin.h>
#include <cstring>

using namespace wreport;

void Options::init_varcodes(const char* str)
{
    varcodes.clear();
    while (str && *str && strlen(str) >= 6)
    {
        varcodes.push_back(descriptor_code(str));
        str = strchr(str, ',');
        if (str && *str) ++str;
    }
}

void BulletinHeadHandler::handle_raw_bufr(const std::string& raw_data, const char* fname, long offset)
{
    // Decode the raw data. fname and offset are optional and we pass
    // them just to have nicer error messages
    auto bulletin = BufrBulletin::decode_header(raw_data, fname, offset);

    // Do something with the decoded information
    handle(*bulletin);
}

void BulletinHeadHandler::handle_raw_crex(const std::string& raw_data, const char* fname, long offset)
{
    // Decode the raw data. fname and offset are optional and we pass
    // them just to have nicer error messages
    auto bulletin = CrexBulletin::decode(raw_data, fname, offset);

    // Do something with the decoded information
    handle(*bulletin);
}

void BulletinFullHandler::handle_raw_bufr(const std::string& raw_data, const char* fname, long offset)
{
    // Decode the raw data. fname and offset are optional and we pass
    // them just to have nicer error messages
    auto bulletin = BufrBulletin::decode(raw_data, fname, offset);

    // Do something with the decoded information
    handle(*bulletin);
}

void BulletinFullHandler::handle_raw_crex(const std::string& raw_data, const char* fname, long offset)
{
    // Decode the raw data. fname and offset are optional and we pass
    // them just to have nicer error messages
    auto bulletin = CrexBulletin::decode(raw_data, fname, offset);

    // Do something with the decoded information
    handle(*bulletin);
}
