#ifndef WREPORT_VARINFO_H
#define WREPORT_VARINFO_H

#include <cstdint>
#include <iosfwd>
#include <string>
#include <wreport/fwd.h>

namespace wreport {

/** @file
 * @ingroup core
 * Implement fast access to information about WMO variables.
 *
 * The measured value of a physical quantity has little meaning without
 * specifying what quantity it represents, what units are used to measure it,
 * and how many digits are significant for the value.
 *
 * This module provides access to all this metadata:
 *
 * \li \b wreport::Varcode represents what is the quantity measured, and takes
 *    values from the WMO B tables used for BUFR and CREX encodings.
 *    The ::WR_VAR macro can be used to construct wreport::Varcode values, and
 * the
 *    ::WR_VAR_F, ::WR_VAR_X and ::WR_VAR_Y macros can be used to access the
 *    various parts of the dba_varcode.
 * \li \b wreport::Varinfo contains all the expanded information about a
 * variable: its wreport::Varcode, description, measurement units, significant
 * digits, minimum and maximum values it can have and other information useful
 * for serialisation and deserialisation of values.
 *
 * There are many B tables with slight differences used by different
 * meteorological centre or equipment.  This module allows to access
 * different vartables using dba_vartable_create().
 *
 * wreport::Vartable and wreport::Varinfo have special memory management: they
 * are never deallocated.  This is a precise design choice to speed up passing
 * and copying wreport::Varinfo values, that are used very intensely as they
 * accompany all the physical values processed by wreport. This behaviour should
 * not be a cause of memory leaks, since a software would only need to access a
 * limited amount of B tables during its lifetime.
 *
 * To construct a wreport::Varcode value one needs to provide three numbers: F,
 * X and Y.
 *
 * \li \b F (2 bits) identifies the type of table entry represented by the
 * dba_varcode, and is always 0 for B tables.  Different values are only used
 * during encoding and decoding of BUFR and CREX messages and are not in use in
 * other parts of wreport.
 * \li \b X (6 bits) identifies a section of the table.
 * \li \b Y (8 bits) identifies the value within the section.
 *
 * The normal text representation of a wreport::Varcode for a WMO B table uses
 * the format Bxxyyy.
 */

/**
 * Holds the WMO variable code of a variable
 */
typedef uint16_t Varcode;

/// Format a varcode into a string
std::string varcode_format(Varcode code);

/**
 * Create a WMO variable code from its F, X and Y components.
 */
#define WR_VAR(f, x, y)                                                        \
    (static_cast<wreport::Varcode>((static_cast<unsigned>(f) << 14) |          \
                                   (static_cast<unsigned>(x) << 8) |           \
                                   static_cast<unsigned>(y)))

/**
 * Convert a XXYYY string to a WMO variable code.
 *
 * This is useful only in rare cases, such as when parsing tables; use
 * descriptor_code() to parse proper entry names such as "B01003" or "D21301".
 */
#define WR_STRING_TO_VAR(str)                                                  \
    static_cast<wreport::Varcode>(                                             \
        ((((str)[0] - '0') * 10 + ((str)[1] - '0')) << 8) |                    \
        (((str)[2] - '0') * 100 + ((str)[3] - '0') * 10 + ((str)[4] - '0')))

/// Get the F part of a WMO variable code.
#define WR_VAR_F(code) (((code) >> 14) & 0x3)

/// Get the X part of a WMO variable code.
#define WR_VAR_X(code) ((code) >> 8 & 0x3f)

/// Get the Y part of a WMO variable code.
#define WR_VAR_Y(code) ((code) & 0xff)

/**
 * Expands to WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code).
 *
 * This is intended as a convenient shortcut to pass a broken down varcode to
 * functions like printf, but not much more than that. Of course it evaluates
 * its argument multiple times.
 */
#define WR_VAR_FXY(code) WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code)

/**
 * Convert a FXXYYY string descriptor code into its short integer
 * representation.
 *
 * @param desc
 *   The 6-byte string descriptor as FXXYYY
 *
 * @return
 *   The short integer code that can be queried with the WR_GET_* macros
 */
Varcode varcode_parse(const char* desc);

/// Variable type
enum class Vartype : unsigned {
    // Integer value
    Integer,
    // Floating point value
    Decimal,
    // String value
    String,
    // Opaque binary value
    Binary,
};

/// Return a string description of a Vartype
const char* vartype_format(Vartype type);

/// Return a Vartype from its string description
Vartype vartype_parse(const char* s);

std::ostream& operator<<(std::ostream& out, const Vartype& t);

/**
 * Information about a variable.
 *
 * The normal value of a variable is considered expressed in unit
 */
struct _Varinfo
{
    /// Variable code, as in WMO BUFR/CREX table B
    Varcode code;

    /// Type of the value stored in the variable
    Vartype type;

    /// Freeform variable description
    char desc[64];

    /// Measurement unit of the variable, using the units defined in WMO
    /// BUFR/CREX table B
    char unit[24];

    /**
     * Scale of the variable, defining its decimal precision.
     *
     * The value of the variable can be encoded as a decimal integer
     * by computing value * exp10(scale).
     */
    int scale;

    /// Length in digits of the variable encoded as a decimal integer
    unsigned len;

    /**
     * Binary reference value for the variable.
     *
     * The value of the variable can be encoded as an unsigned binary value by
     * computing value * exp10(scale) + bit_ref.
     */
    int bit_ref;

    /// Length in bits of the variable when encoded as an unsigned binary value
    unsigned bit_len;

    /// Minimum unscaled decimal integer value the field can have
    int imin;

    /// Minimum unscaled decimal integer value the field can have
    int imax;

    /// Minimum value the field can have
    double dmin;

    /// Maximum value the field can have
    double dmax;

    /**
     * Encode a double value into a decimal integer value using Varinfo decimal
     * encoding informations (scale)
     *
     * @param fval
     *   Value to encode
     * @returns
     *   The double value encoded as an integer
     */
    int encode_decimal(double fval) const;

    /**
     * Round val so that it only fits the significant digits given in scale
     */
    double round_decimal(double val) const;

    /**
     * Encode a double value into a positive integer value using Varinfo binary
     * encoding informations (bit_ref and scale)
     *
     * @param fval
     *   Value to encode
     * @returns
     *   The double value encoded as an unsigned integer
     */
    uint32_t encode_binary(double fval) const;

    /**
     * Decode a double value from a decimal integer value using Varinfo
     * decimal encoding informations (scale)
     *
     * @param val
     *   Value to decode
     * @returns
     *   The decoded double value
     */
    double decode_decimal(int val) const;

    /**
     * Decode a double value from a decimal integer value using Varinfo
     * binary encoding informations (bit_ref and scale)
     *
     * @param val
     *   Value to decode
     * @returns
     *   The decoded double value
     */
    double decode_binary(uint32_t val) const;

    /**
     * Setup this variable as a BUFR variable.
     *
     * This is a compatibility method for existing old code. The functions used
     * to initialize _Varinfo structures are in internals/varinfo.h, and a new
     * API for creating _Varinfo structures outside of tables has not yet been
     * designed.
     *
     * @param len
     *   this is ignored and computed from the other arguments
     */
    [[deprecated("Use varinfo_create_bufr")]] void
    set_bufr(Varcode code, const char* desc, const char* unit, int scale = 0,
             unsigned len = 0, int bit_ref = 0, int bit_len = 0);
};

/**
 * Varinfo reference.
 *
 * Since the actual structures are allocated inside the Vartable objects and
 * never deallocated until the program quits, we do not need to track memory
 * allocation and we can just refer to variable information with const
 * pointers.
 */
typedef const _Varinfo* Varinfo;

/**
 * Allocate a new Varinfo to store BUFR values
 *
 * @returns
 *   The newly allocated Varinfo, to be deallocated with delete_varinfo()
 */
Varinfo varinfo_create_bufr(Varcode code, const char* desc, const char* unit,
                            unsigned bit_len, uint32_t bit_ref = 0,
                            int scale = 0);

/**
 * Deallocate a Varinfo created with create_bufr().
 *
 * Sets info to nullptr;
 */
void varinfo_delete(Varinfo&& info);

} // namespace wreport
#endif
