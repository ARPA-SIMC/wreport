/*
 * wreport/conv - Unit conversions
 *
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "conv.h"
#include "error.h"
#include "codetables.h"

#include <math.h>

namespace wreport {

/*
Cloud type VM		Cloud type 20012
WMO code 0509
Ch	0			10
Ch	1			11
Ch	...			...
Ch	9			19
Ch	/			60

WMO code 0515
Cm	0			20
Cm	1			21
Cm	...			...
Cm	9			29
Cm	/			61

WMO code 0513
Cl	0			30
Cl	1			31
Cl	...			...
Cl	9			39
Cl	/			62

				missing value: 63

WMO code 0500
Per i cloud type nei 4 gruppi ripetuti del synop:
	0..9 -> 0..9
	/ -> 59
*/

int convert_WMO0500_to_BUFR20012(int from)
{
	if (from >= 0 && from <= 9)
		return from;
	else if (from == -1) /* FIXME: check what is the value for '/' */
		return 59;
	else
		error_domain::throwf("value %d not found in WMO code table 0500", from);
}

int convert_BUFR20012_to_WMO0500(int from)
{
	if (from >= 0 && from <= 9)
		return from;
	else if (from == 59)
		return -1; /* FIXME: check what is the value for '/' */
	else
		error_domain::throwf(
				"BUFR 20012 value %d cannot be represented with WMO code table 0500",
				from);
}

int convert_WMO0509_to_BUFR20012(int from)
{
	if (from >= 0 && from <= 9)
		return from + 10;
	else if (from == -1) /* FIXME: check what is the value for '/' */
		return 60;
	else
		error_domain::throwf("value %d not found in WMO code table 0509", from);
}

int convert_BUFR20012_to_WMO0509(int from)
{
	if (from >= 10 && from <= 19)
		return from - 10;
	else if (from == 60)
		return -1; /* FIXME: check what is the value for '/' */
	else
		error_domain::throwf(
				"BUFR 20012 value %d cannot be represented with WMO code table 0509",
				from);
}

int convert_WMO0515_to_BUFR20012(int from)
{
	if (from >= 0 && from <= 9)
		return from + 20;
	else if (from == -1) /* FIXME: check what is the value for '/' */
		return 61;
	else
		error_domain::throwf("value %d not found in WMO code table 0515", from);
}

int convert_BUFR20012_to_WMO0515(int from)
{
	if (from >= 20 && from <= 29)
		return from - 20;
	else if (from == 61)
		return -1; /* FIXME: check what is the value for '/' */
	else
		error_domain::throwf(
				"BUFR 20012 value %d cannot be represented with WMO code table 0515",
				from);
}

int convert_WMO0513_to_BUFR20012(int from)
{
	if (from >= 0 && from <= 9)
		return from + 30;
	else if (from == -1) /* FIXME: check what is the value for '/' */
		return 62;
	else
		error_domain::throwf("value %d not found in WMO code table 0513", from);
}

int convert_BUFR20012_to_WMO0513(int from)
{
	if (from >= 30 && from <= 39)
		return from - 30;
	else if (from == 62)
		return -1; /* FIXME: check what is the value for '/' */
	else
		error_domain::throwf(
				"BUFR 20012 value %d cannot be represented with WMO code table 0513",
				from);
}

int convert_WMO4677_to_BUFR20003(int from)
{
	if (from <= 99)
		return from;
	else
		error_domain::throwf("cannot handle WMO4677 present weather (%d) values above 99", from);
}

int convert_BUFR20003_to_WMO4677(int from)
{
	if (from <= 99)
		return from;
	else
		error_domain::throwf("cannot handle BUFR 20003 present weather (%d) values above 99", from);
}

int convert_WMO4561_to_BUFR20004(int from)
{
	if (from <= 9)
		return from;
	else
		error_domain::throwf("cannot handle WMO4561 past weather (%d) values above 9", from);
}

int convert_BUFR20004_to_WMO4561(int from)
{
	if (from <= 9)
		return from;
	else
		error_domain::throwf("cannot handle BUFR 20004 present weather (%d) values above 9", from);
}

int convert_BUFR08001_to_BUFR08042(int from)
{
    // Handle missing value
    if (from & BUFR08001::MISSING)
        return BUFR08042::ALL_MISSING;

    int res = 0;
    if (from & BUFR08001::SIGWIND) res |= BUFR08042::SIGWIND;
    if (from & BUFR08001::SIGTH)   res |= BUFR08042::SIGTEMP | BUFR08042::SIGHUM;
    if (from & BUFR08001::MAXWIND) res |= BUFR08042::MAXWIND;
    if (from & BUFR08001::TROPO)   res |= BUFR08042::TROPO;
    if (from & BUFR08001::STD)     res |= BUFR08042::STD;
    if (from & BUFR08001::SURFACE) res |= BUFR08042::SURFACE;
    return res;
}

int convert_BUFR08042_to_BUFR08001(int from)
{
    if (from & BUFR08042::MISSING)
        return BUFR08001::ALL_MISSING;

    int res = 0;
    if (from & BUFR08042::SIGWIND) res |= BUFR08001::SIGWIND;
    if (from & BUFR08042::SIGHUM)  res |= BUFR08001::SIGTH;
    if (from & BUFR08042::SIGTEMP) res |= BUFR08001::SIGTH;
    if (from & BUFR08042::MAXWIND) res |= BUFR08001::MAXWIND;
    if (from & BUFR08042::TROPO)   res |= BUFR08001::TROPO;
    if (from & BUFR08042::STD)     res |= BUFR08001::STD;
    if (from & BUFR08042::SURFACE) res |= BUFR08001::SURFACE;
    return res;
}

double convert_icao_to_press(double from)
{
	static const double ZA = 5.252368255329;
	static const double ZB = 44330.769230769;
	static const double ZC = 0.000157583169442;
	static const double P0 = 1013.25;
	static const double P11 = 226.547172;

	if (from <= 11000)
		/* We are below 11 km */
		return P0 * pow(1 - from / ZB, ZA);
	else
		/* We are above 11 km */
		return P11 * exp(-ZC * (from - 11000));
}

double convert_press_to_icao(double from)
{
	throw error_unimplemented("converting pressure to ICAO height is not implemented");
}

int convert_AOFVSS_to_BUFR08001(int from)
{
	int res = 0;
	if (from & (1 << 0))	/* Maximum wind level	*/
		res |= 8;
	if (from & (1 << 1))	/* Tropopause		*/
		res |= 16;
	/* Skipped */		/* Part D, non-standard level data, p < 100hPa */
	if (from & (1 << 3))	/* Part C, standard level data, p < 100hPa */
		res |= 32;
	/* Skipped */		/* Part B, non-standard level data, p > 100hPa */
	if (from & (1 << 5))	/* Part A, standard level data, p > 100hPa */
		res |= 32;
	if (from & (1 << 6))	/* Surface */
		res |= 64;
	if (from & (1 << 7))	/* Significant wind level */
		res |= 2;
	if (from & (1 << 8))	/* Significant temperature level */
		res |= 4;
	return res;
}

}
