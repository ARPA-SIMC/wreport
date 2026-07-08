#ifndef WREPORT_INTERNALS_VARINFO_H
#define WREPORT_INTERNALS_VARINFO_H

#include <wreport/error.h>
#include <wreport/fwd.h>
#include <wreport/varinfo.h>

namespace wreport::varinfo {

constexpr int intexp10(unsigned x)
{
    switch (x)
    {
        case 0:  return 1;
        case 1:  return 10;
        case 2:  return 100;
        case 3:  return 1000;
        case 4:  return 10000;
        case 5:  return 100000;
        case 6:  return 1000000;
        case 7:  return 10000000;
        case 8:  return 100000000;
        case 9:  return 1000000000;
        default: error_domain::throwf("%u^10 would not fit in 32 bits", x);
    }
}

/// Return the number of digits in the given number
constexpr unsigned count_digits(uint32_t val)
{
    return (val >= 1000000000)  ? 10
           : (val >= 100000000) ? 9
           : (val >= 10000000)  ? 8
           : (val >= 1000000)   ? 7
           : (val >= 100000)    ? 6
           : (val >= 10000)     ? 5
           : (val >= 1000)      ? 4
           : (val >= 100)       ? 3
           : (val >= 10)        ? 2
                                : 1;
}

/// Set all the base Varinfo fields for a BUFR entry
void set_bufr(_Varinfo& info, Varcode code, const char* desc, const char* unit,
              unsigned bit_len, int32_t bit_ref = 0, int scale = 0);

/// Set all the base Varinfo fields for a CREX entry
void set_crex(_Varinfo& info, Varcode code, const char* desc, const char* unit,
              unsigned len, int scale = 0);

/**
 * Set all the fields to represent a string variable.
 *
 * @param code the variable code
 * @param desc the variable description
 * @param len the maximum string length
 */
void set_string(_Varinfo& info, Varcode code, const char* desc, unsigned len);

/**
 * Set all the fields to represent an opaque binary variable.
 *
 * @param code the variable code
 * @param desc the variable description
 * @param bit_len the variable length in bits
 */
void set_binary(_Varinfo& info, Varcode code, const char* desc,
                unsigned bit_len);

/// Compute the entry type
void compute_type(_Varinfo& info);

} // namespace wreport::varinfo

#endif
