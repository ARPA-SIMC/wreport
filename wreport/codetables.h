/*
 * wreport/codetables - Code table constants
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

#ifndef WREPORT_CODETABLES
#define WREPORT_CODETABLES

/** @file
 * @ingroup conv
 * Unit conversion functions.
 */

namespace wreport {

// 7 bit, leftmost is bit 1
namespace BUFR08001 {
const unsigned int MISSING = 0x01; /* 7 */
const unsigned int SIGWIND = 0x02; /* 6 */
const unsigned int SIGTH   = 0x04; /* 5 */
const unsigned int MAXWIND = 0x08; /* 4 */
const unsigned int TROPO   = 0x10; /* 3 */
const unsigned int STD     = 0x20; /* 2 */
const unsigned int SURFACE = 0x40; /* 1 */
const unsigned int ALL_MISSING = 0x7f;
}

// 18 bit, leftmost is bit 1
namespace BUFR08042 {
const unsigned int MISSING  = 0x00001; /* 18 */
const unsigned int H2PRESS  = 0x00002; /* 17 */
const unsigned int RESERVED = 0x00004; /* 16 */
const unsigned int REGIONAL = 0x00008; /* 15 */
const unsigned int TOPWIND  = 0x00010; /* 14 */
const unsigned int ENDMISSW = 0x00020; /* 13 */
const unsigned int BEGMISSW = 0x00040; /* 12 */
const unsigned int ENDMISSH = 0x00080; /* 11 */
const unsigned int BEGMISSH = 0x00100; /* 10 */
const unsigned int ENDMISST = 0x00200; /*  9 */
const unsigned int BEGMISST = 0x00400; /*  8 */
const unsigned int SIGWIND  = 0x00800; /*  7 */
const unsigned int SIGHUM   = 0x01000; /*  6 */
const unsigned int SIGTEMP  = 0x02000; /*  5 */
const unsigned int MAXWIND  = 0x04000; /*  4 */
const unsigned int TROPO    = 0x08000; /*  3 */
const unsigned int STD      = 0x10000; /*  2 */
const unsigned int SURFACE  = 0x20000; /*  1 */
const unsigned int ALL_MISSING = 0x3ffff;
}

}

/* vim:set ts=4 sw=4: */
#endif
