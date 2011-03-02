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

#ifndef WREPORT_CONV
#define WREPORT_CONV

/** @file
 * @ingroup conv
 * Unit conversion functions.
 */

namespace wreport {

/**
 * Convert between different units
 *
 * @param from
 *   Unit of the value to convert (see ::dba_varinfo)
 * @param to
 *   Unit to convert to (see ::dba_varinfo)
 * @param val
 *   Value to convert
 * @retval res
 *   Converted value
 * @returns
 *   The error indicator for the function (See @ref ::dba_err)
 */
double convert_units(const char* from, const char* to, double val);

/**
 * Convert ICAO height (in meters) to pressure (in hpa) and back
 */
double convert_icao_to_press(double from);

/**
 * Convert pressure (in hpa) to ICAO height (in meters)
 */
double convert_press_to_icao(double from);

/**
 * Convert vertical sounding significance from the AOF encoding to BUFR code
 * table 08001.
 */
int convert_AOFVSS_to_BUFR08042(int from);

/**
 * Conversion functions between various code tables
 * @{ */
/** Cloud type */
int convert_WMO0500_to_BUFR20012(int from);
/** Cloud type (CH) */
int convert_WMO0509_to_BUFR20012(int from);
/** Cloud type (CM) */
int convert_WMO0515_to_BUFR20012(int from);
/** Cloud type (CL) */
int convert_WMO0513_to_BUFR20012(int from);
/** Present weather */
int convert_WMO4677_to_BUFR20003(int from);
/** Past weather */
int convert_WMO4561_to_BUFR20004(int from);

/** Cloud type */
int convert_BUFR20012_to_WMO0500(int from);
/** Cloud type (CH) */
int convert_BUFR20012_to_WMO0509(int from);
/** Cloud type (CM) */
int convert_BUFR20012_to_WMO0515(int from);
/** Cloud type (CL) */
int convert_BUFR20012_to_WMO0513(int from);
/** Present weather */
int convert_BUFR20003_to_WMO4677(int from);
/** Past weather */
int convert_BUFR20004_to_WMO4561(int from);
/** Vertical sounding significance */
int convert_BUFR08001_to_BUFR08042(int from);
/** Vertical sounding significance */
int convert_BUFR08042_to_BUFR08001(int from);
/* @} */

/**
 * Get the multiplier used in the given conversion
 *
 * @param from
 *   Unit of the value to convert (see ::dba_varinfo)
 * @param to
 *   Unit to convert to (see ::dba_varinfo)
 * @returns
 *   Multiplier factor used in the conversion
 */
double convert_units_get_mul(const char* from, const char* to);

/**
 * Check if conversion is possible among the given units
 *
 * @param from
 *   Unit of the value to convert (see ::dba_varinfo)
 * @param to
 *   Unit to convert to (see ::dba_varinfo)
 * @returns
 *   True if conversion is supported, else false.
 */
bool convert_units_allowed(const char* from, const char* to);
}

/* vim:set ts=4 sw=4: */
#endif
