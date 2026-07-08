#include "varinfo.h"
#include "config.h"
#include "error.h"
#include "internals/varinfo.h"
#include <cctype>
#include <climits>
#include <cmath>
#include <cstring>
#include <iostream>

using namespace std;
using namespace wreport::varinfo;

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

void _Varinfo::set_bufr(Varcode code, const char* desc, const char* unit,
                        int scale, unsigned len, int bit_ref, int bit_len)
{
    varinfo::set_bufr(*this, code, desc, unit, bit_len, bit_ref, scale);
}

Varinfo varinfo_create_bufr(Varcode code, const char* desc, const char* unit,
                            unsigned bit_len, uint32_t bit_ref, int scale)
{
    auto res = new _Varinfo;
    varinfo::set_bufr(*res, code, desc, unit, bit_len, bit_ref, scale);
    return res;
}

void varinfo_delete(Varinfo&& info)
{
    delete info;
    info = nullptr;
}

} // namespace wreport
