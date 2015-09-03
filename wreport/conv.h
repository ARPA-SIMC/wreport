#ifndef WREPORT_CONV
#define WREPORT_CONV

/** @file
 * Unit conversion functions.
 */

namespace wreport {

/**
 * Convert between different units
 *
 * @param from
 *   Unit of the value to convert (see wreport::Varinfo)
 * @param to
 *   Unit to convert to (see wreport::Varinfo)
 * @param val
 *   Value to convert
 * @retval res
 *   Converted value
 * @returns
 *   The error indicator for the function (See @ref error.h)
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
 * Convert wind direction (in octants) to degrees
 */
double convert_octants_to_degrees(int from);

/**
 * Convert wind direction (in degrees) to octancts
 */
int convert_degrees_to_octants(double from);

/**
 * Convert vertical sounding significance from the AOF encoding to BUFR code
 * table 08001.
 */
unsigned convert_AOFVSS_to_BUFR08042(unsigned from);

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
unsigned convert_BUFR08001_to_BUFR08042(unsigned from);
/** Vertical sounding significance */
unsigned convert_BUFR08042_to_BUFR08001(unsigned from);
/* @} */

/**
 * Get the multiplier used in the given conversion
 *
 * @param from
 *   Unit of the value to convert (see wreport::Varinfo)
 * @param to
 *   Unit to convert to (see wreport::Varinfo)
 * @returns
 *   Multiplier factor used in the conversion
 */
double convert_units_get_mul(const char* from, const char* to);

/**
 * Check if conversion is possible among the given units
 *
 * @param from
 *   Unit of the value to convert (see wreport::Varinfo)
 * @param to
 *   Unit to convert to (see wreport::Varinfo)
 * @returns
 *   True if conversion is supported, else false.
 */
bool convert_units_allowed(const char* from, const char* to);
}

#endif
