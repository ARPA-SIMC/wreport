#include "varinfo.h"
#include <climits>
#include <cmath>
#include <cstring>

namespace wreport::varinfo {

void set_bufr(_Varinfo& info, Varcode code, const char* desc, const char* unit,
              unsigned bit_len, int32_t bit_ref, int scale)
{
    info.code = code;
    strncpy(info.desc, desc, 63);
    info.desc[63] = 0;
    strncpy(info.unit, unit, 23);
    info.unit[23] = 0;
    info.scale    = scale;
    info.bit_ref  = bit_ref;
    info.bit_len  = bit_len;
    compute_type(info);

    // Compute storage length and ranges
    if (info.type == Vartype::String or info.type == Vartype::Binary)
    {
        if (bit_len % 8)
            info.len = bit_len / 8 + 1;
        else
            info.len = bit_len / 8;
        info.imin = info.imax = 0;
        info.dmin = info.dmax = 0.0;
        return;
    }

    // Compute the decimal length as the maximum number of
    // digits needed to encode both the minimum and maximum *unscaled*
    // values in the variable domain
    info.imin = bit_ref;

    uint32_t maxval = bit_len == 31 ? 0x7fffffff : (1 << bit_len) - 1;
    // We need to subtract 1 because 2^bit_len-1 is the
    // BUFR missing value. However, we cannot do it with the delayed
    // replication factors (encoded in 8 bits) because RADAR BUFR messages
    // have 255 subsets, and the replication factor is not interpreted as
    // missing. We need to assume that the missing value does not apply to
    // delayed replication factors.
    if (WR_VAR_X(code) != 31)
        maxval -= 1;

    if (bit_len == 31 and bit_ref > 0)
    {
        info.len  = 10;
        info.imax = maxval;
        error_consistency::throwf(
            "%01d%02d%03d scaled value does not fit in a signed 32bit "
            "integer (%d bits with a base value of %d)",
            WR_VAR_FXY(code), bit_len, bit_ref);
    }

    if (bit_ref == 0)
    {
        info.imax = maxval;
        if (bit_len == 1)
            info.len = 1;
        else
            info.len = count_digits(maxval);
    }
    else if (bit_ref < 0)
    {
        info.imax = maxval + bit_ref;
        info.len =
            std::max(count_digits(-bit_ref), count_digits(abs(info.imax)));
    }
    else
    {
        if ((0x7ffffffe - maxval) < (unsigned)bit_ref)
        {
            info.len  = 10;
            info.imax = maxval;
            error_consistency::throwf(
                "%01d%02d%03d scaled value does not fit in a signed "
                "32bit "
                "integer (%d bits with a base value of %d)",
                WR_VAR_FXY(code), bit_len, bit_ref);
        }
        info.imax = maxval + bit_ref;
        info.len  = std::max(count_digits(bit_ref), count_digits(info.imax));
    }

    info.dmin = info.decode_decimal(info.imin);
    info.dmax = info.decode_decimal(info.imax);
}

void set_crex(_Varinfo& info, Varcode code, const char* desc, const char* unit,
              unsigned len, int scale)
{
    info.code = code;
    strncpy(info.desc, desc, 63);
    info.desc[63] = 0;
    strncpy(info.unit, unit, 23);
    info.unit[23] = 0;
    info.scale    = scale;
    info.len      = len;
    info.bit_ref  = 0;
    info.bit_len  = 0;
    compute_type(info);

    // Compute range
    switch (info.type)
    {
        case Vartype::String:
        case Vartype::Binary:
            info.imin = info.imax = 0;
            info.dmin = info.dmax = 0.0;
            break;
        case Vartype::Integer:
        case Vartype::Decimal:
            if (len >= 10)
            {
                info.imin = INT_MIN;
                info.imax = INT_MAX;
            }
            else
            {
                // Ignore binary encoding if we do not have information for
                // it

                // We subtract 2 because 10^len-1 is the
                // CREX missing value
                info.imin = static_cast<int>(-(intexp10(len) - 1.0));
                info.imax = static_cast<int>((intexp10(len) - 2.0));
            }
            info.dmin = info.decode_decimal(info.imin);
            info.dmax = info.decode_decimal(info.imax);
            break;
    }
}

void set_string(_Varinfo& info, Varcode code, const char* desc, unsigned len)
{
    info.code = code;
    strncpy(info.desc, desc, 63);
    info.desc[63] = 0;
    strncpy(info.unit, "CCITTIA5", 24);
    info.scale   = 0;
    info.len     = len;
    info.bit_ref = 0;
    info.bit_len = len * 8;
    info.type    = Vartype::String;
    info.imin = info.imax = 0;
    info.dmin = info.dmax = 0.0;
}

void set_binary(_Varinfo& info, Varcode code, const char* desc,
                unsigned bit_len)
{
    info.code = code;
    strncpy(info.desc, desc, 63);
    info.desc[63] = 0;
    strncpy(info.unit, "UNKNOWN", 24);
    info.scale   = 0;
    info.len     = static_cast<unsigned>(ceil(bit_len / 8.0));
    info.bit_ref = 0;
    info.bit_len = bit_len;
    info.type    = Vartype::Binary;
    info.imin = info.imax = 0;
    info.dmin = info.dmax = 0.0;
}

void compute_type(_Varinfo& info)
{
    // Set the is_string flag based on the unit
    if (strcmp(info.unit, "CCITTIA5") == 0 or
        strcmp(info.unit, "CHARACTER") == 0)
        info.type = Vartype::String;
    else if (info.scale)
        info.type = Vartype::Decimal;
    else
        info.type = Vartype::Integer;
}

} // namespace wreport::varinfo
