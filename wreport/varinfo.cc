#include "varinfo.h"
#include "error.h"
#include <cstring>
#include <cmath>
#include <climits>
#include <cctype>
#include "config.h"

using namespace std;

namespace wreport {

const char* vartype_format(Vartype type)
{
    switch (type)
    {
        case Vartype::Binary: return "binary";
        case Vartype::String: return "string";
        case Vartype::Integer: return "integer";
        case Vartype::Decimal: return "decimal";
    }
    return "unknown";
}

static int intexp10(unsigned x)
{
	switch (x)
	{
		case  0: return 1;
		case  1: return 10;
		case  2: return 100;
		case  3: return 1000;
		case  4: return 10000;
		case  5: return 100000;
		case  6: return 1000000;
		case  7: return 10000000;
		case  8: return 100000000;
		case  9: return 1000000000;
		default:
			error_domain::throwf("%u^10 would not fit in 32 bits", x);
	}
}

Varcode varcode_parse(const char* entry)
{
    if (!entry)
        throw error_consistency("cannot parse a Varcode out of a NULL");
    if (!entry[0])
        throw error_consistency("cannot parse a Varcode out of an empty string");

    Varcode res = 0;
    switch (entry[0])
    {
        case 'B':
        case '0':
            res = 0; break;
        case 'R':
        case '1':
            res = WR_VAR_F(1); break;
        case 'C':
        case '2':
            res = WR_VAR_F(2); break;
        case 'D':
        case '3':
            res = WR_VAR_F(3); break;
    }

    // Ensure that B is followed by 5 integers
    for (unsigned i = 1; i < 6; ++i)
        if (entry[i] and !isdigit(entry[i]))
            error_consistency::throwf("cannot parse a Varcode out of '%s'", entry);

    return res | WR_STRING_TO_VAR(entry + 1);
}

std::string varcode_format(Varcode code)
{
	static const char* fcodes = "BRCD";
	char buf[8];
	snprintf(buf, 8, "%c%02d%03d", fcodes[WR_VAR_F(code)], WR_VAR_X(code), WR_VAR_Y(code));
	return buf;
}

void _Varinfo::set_bufr(Varcode code,
                   const char* desc,
                   const char* unit,
                   int scale, unsigned len,
                   int bit_ref, int bit_len)
{
    this->code = code;
    strncpy(this->desc, desc, 64);
    strncpy(this->unit, unit, 24);
    this->scale = scale;
    this->len = len;
    this->bit_ref = bit_ref;
    this->bit_len = bit_len;
    if (strcmp(unit, "CCITTIA5") == 0)
        this->type = Vartype::String;
    else if (scale == 0)
        this->type = Vartype::Integer;
    else
        this->type = Vartype::Decimal;
    compute_range();
}

void _Varinfo::set_crex(Varcode code,
                   const char* desc,
                   const char* unit,
                   int scale, unsigned len)
{
    this->code = code;
    strncpy(this->desc, desc, 64);
    strncpy(this->unit, unit, 24);
    this->scale = scale;
    this->len = len;
    this->bit_ref = 0;
    this->bit_len = 0;
    if (strcmp(unit, "CHARACTER") == 0)
        this->type = Vartype::String;
    else if (scale == 0)
        this->type = Vartype::Integer;
    else
        this->type = Vartype::Decimal;
    compute_range();
}

void _Varinfo::set_string(Varcode code, const char* desc, unsigned len)
{
    this->code = code;
    strncpy(this->desc, desc, 64);
    strncpy(this->unit, "CCITTIA5", 24);
    this->scale = 0;
    this->len = len;
    this->bit_ref = 0;
    this->bit_len = len * 8;
    this->type = Vartype::String;
    compute_range();
}

void _Varinfo::set_binary(Varcode code, const char* desc, unsigned bit_len)
{
    this->code = code;
    strncpy(this->desc, desc, 64);
    strncpy(this->unit, "UNKNOWN", 24);
    this->scale = 0;
    this->len = ceil(bit_len / 8.0);
    this->bit_ref = 0;
    this->bit_len = bit_len;
    this->type = Vartype::Binary;
    compute_range();
}


void _Varinfo::compute_range()
{
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
            } else if (bit_len == 0) {
                // Ignore binary encoding if we do not have information for it

                // We subtract 2 because 10^len-1 is the
                // CREX missing value
                imin =  -(intexp10(len) - 1.0);
                imax = (intexp10(len) - 2.0);
            } else {
                int bit_min = bit_ref;
                int bit_max = exp2(bit_len) + bit_ref;
                // We subtract 2 because 2^bit_len-1 is the
                // BUFR missing value.
                // We cannot subtract 2 from the delayed replication
                // factors because RADAR BUFR messages have 255
                // subsets, and the delayed replication field is 8
                // bits, so 255 is the missing value, and if we
                // disallow it here we cannot import radars anymore.
                if (WR_VAR_X(code) != 31)
                    bit_max -= 2;
                // We subtract 2 because 10^len-1 is the
                // CREX missing value
                int dec_min = -(intexp10(len) - 1.0);
                int dec_max = (intexp10(len) - 2.0);

                imin = max(bit_min, dec_min);
                imax = min(bit_max, dec_max);
            }
            dmin = decode_decimal(imin);
            dmax = decode_decimal(imax);
            break;
    }
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
        error_consistency::throwf("cannot decode %01d%02d%03d from binary, because the information needed is missing from the B table in use",
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
        error_consistency::throwf("cannot encode %01d%02d%03d to binary, because the information needed is missing from the B table in use",
                WR_VAR_FXY(code));
    double res;
    if (scale > 0)
        res = rint((fval * scales[scale]) - bit_ref);
    else if (scale < 0)
        res = rint((fval / scales[-scale] - bit_ref));
    else
        res = rint(fval - bit_ref);
    if (res < 0)
        error_consistency::throwf("Cannot encode %01d%02d%03d %f to %d bits using scale %d and ref %d: encoding gives negative value %f",
                WR_VAR_FXY(code), fval, bit_len, scale, bit_ref, res);
    return (unsigned)res;
}

}
