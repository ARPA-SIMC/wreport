#include "varinfo.h"
#include "config.h"
#include "error.h"
#include <cctype>
#include <climits>
#include <cmath>
#include <cstring>
#include <iostream>

using namespace std;

namespace wreport {

const char* vartype_format(Vartype type)
{
    switch (type)
    {
        case Vartype::Binary:  return "binary";
        case Vartype::String:  return "string";
        case Vartype::Integer: return "integer";
        case Vartype::Decimal: return "decimal";
        default:               return "unknown";
    }
}

Vartype vartype_parse(const char* s)
{
    if (strcmp(s, "string") == 0)
        return Vartype::String;
    if (strcmp(s, "decimal") == 0)
        return Vartype::Decimal;
    if (strcmp(s, "integer") == 0)
        return Vartype::Integer;
    if (strcmp(s, "binary") == 0)
        return Vartype::Binary;
    error_consistency::throwf("cannot parse Vartype '%s'", s);
}

std::ostream& operator<<(std::ostream& out, const Vartype& t)
{
    return out << vartype_format(t);
}

static int intexp10(unsigned x)
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

Varcode varcode_parse(const char* entry)
{
    if (!entry)
        throw error_consistency("cannot parse a Varcode out of a NULL");
    if (!entry[0])
        throw error_consistency(
            "cannot parse a Varcode out of an empty string");

    Varcode res;
    switch (entry[0])
    {
        case 'B':
        case '0': res = 0; break;
        case 'R':
        case '1': res = 1 << 14; break;
        case 'C':
        case '2': res = 2 << 14; break;
        case 'D':
        case '3': res = 3 << 14; break;
        default:  res = 0; break;
    }

    // Ensure that B is followed by 5 integers
    for (unsigned i = 1; i < 6; ++i)
        if (entry[i] and !isdigit(entry[i]))
            error_consistency::throwf("cannot parse a Varcode out of '%s'",
                                      entry);

    return res + WR_STRING_TO_VAR(entry + 1);
}

std::string varcode_format(Varcode code)
{
    static const char* fcodes = "BRCD";
    char buf[8];
    snprintf(buf, 8, "%c%02d%03d", fcodes[WR_VAR_F(code)], WR_VAR_X(code),
             WR_VAR_Y(code));
    return buf;
}

/// Return the number of digits in the given number
static unsigned count_digits(uint32_t val)
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

void _Varinfo::set_bufr(Varcode code, const char* desc, const char* unit,
                        int scale, int32_t bit_ref, unsigned bit_len)
{
    this->code = code;
    strncpy(this->desc, desc, 63);
    this->desc[63] = 0;
    strncpy(this->unit, unit, 23);
    this->unit[23] = 0;
    this->scale    = scale;
    this->bit_ref  = bit_ref;
    this->bit_len  = bit_len;
    compute_type();

    // Compute storage length and ranges
    if (type == Vartype::String or type == Vartype::Binary)
    {
        if (bit_len % 8)
            len = bit_len / 8 + 1;
        else
            len = bit_len / 8;
        imin = imax = 0;
        dmin = dmax = 0.0;
        return;
    }

    // Compute the decimal length as the maximum number of
    // digits needed to encode both the minimum and maximum *unscaled* values
    // in the variable domain
    imin = bit_ref;

    uint32_t maxval = bit_len == 31 ? 0x7fffffff : (1 << bit_len) - 1;
    // We need to subtract 1 because 2^bit_len-1 is the
    // BUFR missing value. However, we cannot do it with the delayed
    // replication factors (encoded in 8 bits) because RADAR BUFR messages have
    // 255 subsets, and the replication factor is not interpreted as missing.
    // We need to assume that the missing value does not apply to delayed
    // replication factors.
    if (WR_VAR_X(code) != 31)
        maxval -= 1;

    if (bit_len == 31 and bit_ref > 0)
    {
        len  = 10;
        imax = maxval;
        error_consistency::throwf(
            "%01d%02d%03d scaled value does not fit in a signed 32bit "
            "integer (%d bits with a base value of %d)",
            WR_VAR_FXY(code), bit_len, bit_ref);
    }

    if (bit_ref == 0)
    {
        imax = maxval;
        if (bit_len == 1)
            len = 1;
        else
            len = count_digits(maxval);
    }
    else if (bit_ref < 0)
    {
        imax = maxval + bit_ref;
        len  = max(count_digits(-bit_ref), count_digits(abs(imax)));
    }
    else
    {
        if ((0x7ffffffe - maxval) < (unsigned)bit_ref)
        {
            len  = 10;
            imax = maxval;
            error_consistency::throwf(
                "%01d%02d%03d scaled value does not fit in a signed "
                "32bit "
                "integer (%d bits with a base value of %d)",
                WR_VAR_FXY(code), bit_len, bit_ref);
        }
        imax = maxval + bit_ref;
        len  = max(count_digits(bit_ref), count_digits(imax));
    }

    dmin = decode_decimal(imin);
    dmax = decode_decimal(imax);
}

void _Varinfo::set_crex(Varcode code, const char* desc, const char* unit,
                        int scale, unsigned len)
{
    this->code = code;
    strncpy(this->desc, desc, 63);
    this->desc[63] = 0;
    strncpy(this->unit, unit, 23);
    this->unit[23] = 0;
    this->scale    = scale;
    this->len      = len;
    this->bit_ref  = 0;
    this->bit_len  = 0;
    compute_type();

    // Compute range
    switch (type)
    {
        case Vartype::String:
        case Vartype::Binary:
            imin = imax = 0;
            dmin = dmax = 0.0;
            break;
        case Vartype::Integer:
        case Vartype::Decimal:
            if (len >= 10)
            {
                imin = INT_MIN;
                imax = INT_MAX;
            }
            else
            {
                // Ignore binary encoding if we do not have information for
                // it

                // We subtract 2 because 10^len-1 is the
                // CREX missing value
                imin = static_cast<int>(-(intexp10(len) - 1.0));
                imax = static_cast<int>((intexp10(len) - 2.0));
            }
            dmin = decode_decimal(imin);
            dmax = decode_decimal(imax);
            break;
    }
}

void _Varinfo::set_string(Varcode code, const char* desc, unsigned len)
{
    this->code = code;
    strncpy(this->desc, desc, 63);
    this->desc[63] = 0;
    strncpy(this->unit, "CCITTIA5", 24);
    this->scale   = 0;
    this->len     = len;
    this->bit_ref = 0;
    this->bit_len = len * 8;
    this->type    = Vartype::String;
    imin = imax = 0;
    dmin = dmax = 0.0;
}

void _Varinfo::set_binary(Varcode code, const char* desc, unsigned bit_len)
{
    this->code = code;
    strncpy(this->desc, desc, 63);
    this->desc[63] = 0;
    strncpy(this->unit, "UNKNOWN", 24);
    this->scale   = 0;
    this->len     = static_cast<unsigned>(ceil(bit_len / 8.0));
    this->bit_ref = 0;
    this->bit_len = bit_len;
    this->type    = Vartype::Binary;
    imin = imax = 0;
    dmin = dmax = 0.0;
}

static const double scales[] = {
    1.0,
    10.0,
    100.0,
    1000.0,
    10000.0,
    100000.0,
    1000000.0,
    10000000.0,
    100000000.0,
    1000000000.0,
    10000000000.0,
    100000000000.0,
    1000000000000.0,
    10000000000000.0,
    100000000000000.0,
    1000000000000000.0,
    10000000000000000.0,
};

double _Varinfo::decode_decimal(int val) const
{
    if (scale > 0)
        return val / scales[scale];
    else if (scale < 0)
        return val * scales[-scale];
    else
        return val;
}

double _Varinfo::decode_binary(uint32_t ival) const
{
    if (bit_len == 0)
        error_consistency::throwf(
            "cannot decode %01d%02d%03d from binary, because the "
            "information "
            "needed is missing from the B table in use",
            WR_VAR_FXY(code));
    if (scale >= 0)
        return ((double)ival + bit_ref) / scales[scale];
    else
        return ((double)ival + bit_ref) * scales[-scale];
}

int _Varinfo::encode_decimal(double fval) const
{
    if (scale > 0)
        return (int)rint(fval * scales[scale]);
    else if (scale < 0)
        return (int)rint(fval / scales[-scale]);
    else
        return (int)rint(fval);
}

double _Varinfo::round_decimal(double val) const
{
    if (scale > 0)
        return round(val * scales[scale]) / scales[scale];
    else if (scale < 0)
        return round(val / scales[-scale]) * scales[-scale];
    else
        return round(val);
}

unsigned _Varinfo::encode_binary(double fval) const
{
    if (bit_len == 0)
        error_consistency::throwf(
            "cannot encode %01d%02d%03d to binary, because the information "
            "needed is missing from the B table in use",
            WR_VAR_FXY(code));
    double res;
    if (scale > 0)
        res = rint((fval * scales[scale]) - bit_ref);
    else if (scale < 0)
        res = rint((fval / scales[-scale] - bit_ref));
    else
        res = rint(fval - bit_ref);
    if (res < 0)
        error_consistency::throwf("Cannot encode %01d%02d%03d %f to %u "
                                  "bits using scale %d and ref "
                                  "%d: encoding gives negative value %f",
                                  WR_VAR_FXY(code), fval, bit_len, scale,
                                  bit_ref, res);
    return (unsigned)res;
}

void _Varinfo::compute_type()
{
    // Set the is_string flag based on the unit
    if (strcmp(unit, "CCITTIA5") == 0 or strcmp(unit, "CHARACTER") == 0)
        type = Vartype::String;
    else if (scale)
        type = Vartype::Decimal;
    else
        type = Vartype::Integer;
}

} // namespace wreport
