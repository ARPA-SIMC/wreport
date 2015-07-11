#include "var.h"
#include "notes.h"
#include "options.h"
#include "vartable.h"
#include "conv.h"
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include <iostream>

using namespace std;

namespace {

// Compute the number of digits of a 32bit unsigned integer
// From http://stackoverflow.com/questions/1489830/efficient-way-to-determine-number-of-digits-in-an-integer
unsigned count_digits(uint32_t x)
{
    if (x >= 10000) {
        if (x >= 10000000) {
            if (x >= 100000000) {
                if (x >= 1000000000)
                    return 10;
                return 9;
            }
            return 8;
        }
        if (x >= 100000) {
            if (x >= 1000000)
                return 7;
            return 6;
        }
        return 5;
    }
    if (x >= 100) {
        if (x >= 1000)
            return 4;
        return 3;
    }
    if (x >= 10)
        return 2;
    return 1;
}

// Adapted from http://tia.mat.br/blog/html/2014/06/23/integer_to_string_conversion.html
size_t uint32_to_str(uint32_t value, unsigned value_digits, char *dst)
{
    static const char digits[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";
    size_t const length = value_digits;
    size_t next = length - 1;
    while (value >= 100) {
        auto const i = (value % 100) * 2;
        value /= 100;
        dst[next] = digits[i + 1];
        dst[next - 1] = digits[i];
        next -= 2;
    }
    // Handle last 1-2 digits
    if (value < 10) {
        dst[next] = '0' + uint32_t(value);
    } else {
        auto i = uint32_t(value) * 2;
        dst[next] = digits[i + 1];
        dst[next - 1] = digits[i];
    }
    return length;
}

// From http://stackoverflow.com/questions/16826422/c-most-efficient-way-to-convert-string-to-int-faster-than-atoi
unsigned str_to_unsigned(const char *str)
{
    unsigned val = 0;
    while (*str)
        val = val * 10 + (*str++ - '0');
    return val;
}

}

namespace wreport {

Var::Var(Varinfo info)
    : m_info(info), m_isset(false), m_value(nullptr), m_attrs(nullptr)
{
}

Var::Var(Varinfo info, int val)
    : m_info(info), m_isset(false), m_value(nullptr), m_attrs(nullptr)
{
    seti(val);
}

Var::Var(Varinfo info, double val)
    : m_info(info), m_isset(false), m_value(nullptr), m_attrs(nullptr)
{
    setd(val);
}

Var::Var(Varinfo info, const char* val)
    : m_info(info), m_isset(false), m_value(nullptr), m_attrs(nullptr)
{
    setc(val);
}

Var::Var(Varinfo info, const std::string& val)
    : m_info(info), m_isset(false), m_value(nullptr), m_attrs(nullptr)
{
    sets(val);
}

Var::Var(const Var& var)
    : m_info(var.m_info), m_isset(var.m_isset), m_value(nullptr), m_attrs(nullptr)
{
    // We are initialized as unset
    if (m_isset)
    {
        // We share the same varinfo, so we can just copy m_value as it is
        allocate();
        memcpy(m_value, var.m_value, m_info->len + 2);
    }
    setattrs(var);
}

Var::Var(Var&& var)
    : m_info(var.m_info), m_isset(var.m_isset), m_value(var.m_value), m_attrs(var.m_attrs)
{
    // Unset the source variable
    var.m_isset = false;
    var.m_value = nullptr;
    var.m_attrs = nullptr;
}

Var::Var(Varinfo info, const Var& var)
    : m_info(info), m_isset(false), m_value(nullptr), m_attrs(nullptr)
{
    setval(var);
    setattrs(var);
}

Var& Var::operator=(const Var& var)
{
    if (&var == this) return *this;

	// Copy info
	m_info = var.m_info;

    // Copy value
    m_isset = var.m_isset;
    if (m_isset)
    {
        allocate();
        // Since we share the Varinfo, we can just copy the buffer
        memcpy(m_value, var.m_value, m_info->len + 1);
    }

    // Copy attributes
    setattrs(var);
    return *this;
}

Var& Var::operator=(Var&& var)
{
    if (&var == this) return *this;
    delete[] m_value;
    delete m_attrs;
    m_isset = var.m_isset;
    var.m_isset = false;
    m_value = var.m_value;
    var.m_value = nullptr;
    m_attrs = var.m_attrs;
    var.m_attrs = nullptr;
    return *this;
}

Var::~Var()
{
    delete[] m_value;
    delete m_attrs;
}

bool Var::operator==(const Var& var) const
{
    if (code() != var.code()) return false;
    if (!value_equals(var)) return false;

    // Compare attrs
    if (!m_attrs && !var.m_attrs) return true;
    if (!m_attrs || !var.m_attrs) return false;
    return *m_attrs == *var.m_attrs;
}

bool Var::value_equals(const Var& var) const
{
    if (!m_isset && !var.m_isset) return true;
    if (!m_isset || !var.m_isset) return false;

    // Compare value
    switch (m_info->type)
    {
        case Vartype::Binary:
            return memcmp(m_value, var.m_value, m_info->len) == 0;
        case Vartype::String:
            return strcmp(m_value, var.m_value) == 0;
        case Vartype::Integer:
        case Vartype::Decimal:
        {
            // In FC12 [g++ (GCC) 4.4.4 20100630 (Red Hat 4.4.4-10)], for obscure
            // reasons we cannot compare the two enqd()s directly: the test fails
            // even if they have the same values.  Assigning them to doubles first
            // works. WTH?
            double a = enqd();
            double b = var.enqd();
            return a == b;
                //(enqd() == var.enqd())
        }
    }
    error_consistency::throwf("unknown variable type %d", (int)m_info->type);
}

void Var::clear_attrs()
{
    delete m_attrs;
    m_attrs = 0;
}

int Var::enqi() const
{
    if (!m_isset)
        error_notfound::throwf("enqi: %01d%02d%03d (%s) is not defined",
                WR_VAR_FXY(m_info->code), m_info->desc);
    switch (m_info->type)
    {
        case Vartype::String:
            error_type::throwf("enqi: %01d%02d%03d (%s) is a string",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Binary:
            error_type::throwf("enqi: %01d%02d%03d (%s) is an opaque binary",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Integer:
        case Vartype::Decimal:
            if (m_value[0] == '-')
                return -str_to_unsigned(m_value + 1);
            else
                return str_to_unsigned(m_value);
    }
    error_consistency::throwf("unknown variable type %d", (int)m_info->type);
}

double Var::enqd() const
{
    if (!m_isset)
        error_notfound::throwf("enqd: %01d%02d%03d (%s) is not defined",
                WR_VAR_FXY(m_info->code), m_info->desc);
    switch (m_info->type)
    {
        case Vartype::String:
            error_type::throwf("enqd: %01d%02d%03d (%s) is a string",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Binary:
            error_type::throwf("enqd: %01d%02d%03d (%s) is an opaque binary",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Integer:
            if (m_value[0] == '-')
                return -str_to_unsigned(m_value + 1);
            else
                return str_to_unsigned(m_value);
        case Vartype::Decimal:
        {
            int dec;
            if (m_value[0] == '-')
                dec = -str_to_unsigned(m_value + 1);
            else
                dec = str_to_unsigned(m_value);
            return m_info->decode_decimal(dec);
        }
    }
    error_consistency::throwf("unknown variable type %d", (int)m_info->type);
}

const char* Var::enqc() const
{
    if (!m_isset)
        error_notfound::throwf("enqc: %01d%02d%03d (%s) is not defined",
                WR_VAR_FXY(m_info->code), m_info->desc);
    return m_value;
}

void Var::allocate()
{
    if (!m_value && !(m_value = new char[m_info->len + 2]))
        throw error_alloc("allocating space for Var value");
}

void Var::seti(int val)
{
    switch (m_info->type)
    {
        case Vartype::String:
            error_type::throwf("seti: %01d%02d%03d (%s) is a string",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Binary:
            error_type::throwf("seti: %01d%02d%03d (%s) is an opaque binary",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Integer:
        case Vartype::Decimal:
            // Guard against overflows
            if (val < m_info->imin || val > m_info->imax)
            {
                unset();
                if (options::var_silent_domain_errors)
                    return;
                error_domain::throwf("Value %i is outside the range [%i,%i] for %01d%02d%03d (%s)",
                        val, m_info->imin, m_info->imax, WR_VAR_FXY(m_info->code), m_info->desc);
            }

            // Ensure that we have a buffer allocated
            allocate();

            // Set the value
            if (val < 0)
            {
                uint32_t dec = -val;
                unsigned digits = count_digits(dec);
                if (digits > m_info->len)
                    error_consistency::throwf("Value %d to be encoded in %01d%02d%03d does not fit in %d digits",
                            val, WR_VAR_FXY(m_info->code), m_info->len);
                m_value[0] = '-';
                uint32_to_str(dec, digits, m_value + 1);
                m_value[digits + 1] = 0;
            } else {
                uint32_t dec = val;
                unsigned digits = count_digits(dec);
                if (digits > m_info->len)
                    error_consistency::throwf("Value %d to be encoded in %01d%02d%03d does not fit in %d digits",
                            val, WR_VAR_FXY(m_info->code), m_info->len);
                uint32_to_str(dec, digits, m_value);
                m_value[digits] = 0;
            }
            m_isset = true;
            /*snprintf(var->value, var->info->len + 2, "%d", val);*/
            break;
    }
}

void Var::setd(double val)
{
    switch (m_info->type)
    {
        case Vartype::String:
            error_type::throwf("seti: %01d%02d%03d (%s) is a string",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Binary:
            error_type::throwf("seti: %01d%02d%03d (%s) is an opaque binary",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Integer:
            // TODO: shortcut without encode_decimal instead of falling into
            // the Decimal case
        case Vartype::Decimal:
            /* Guard against NaNs */
            if (std::isnan(val))
            {
                unset();
                if (options::var_silent_domain_errors)
                    return;
                error_domain::throwf("Value %g is outside the range [%g,%g] for B%02d%03d (%s)",
                        val, m_info->dmin, m_info->dmax,
                        WR_VAR_X(m_info->code), WR_VAR_Y(m_info->code), m_info->desc);
            }

            /* Guard against overflows */
            if (val < m_info->dmin || val > m_info->dmax)
            {
                unset();
                if (options::var_silent_domain_errors)
                    return;
                error_domain::throwf("Value %g is outside the range [%g,%g] for B%02d%03d (%s)",
                        val, m_info->dmin, m_info->dmax,
                        WR_VAR_X(m_info->code), WR_VAR_Y(m_info->code), m_info->desc);
            }

            // Ensure that we have a buffer allocated
            allocate();

            // Set the value
            int sdec = m_info->encode_decimal(val);
            if (sdec < 0)
            {
                uint32_t dec = -sdec;
                unsigned digits = count_digits(dec);
                if (digits > m_info->len)
                    error_consistency::throwf("Value %g gets encoded in %01d%02d%03d as %d, which does not fit in %d digits",
                            val, WR_VAR_FXY(m_info->code), sdec, m_info->len);
                m_value[0] = '-';
                uint32_to_str(dec, digits, m_value + 1);
                m_value[digits + 1] = 0;
            } else {
                uint32_t dec = sdec;
                unsigned digits = count_digits(dec);
                if (digits > m_info->len)
                    error_consistency::throwf("Value %g gets encoded in %01d%02d%03d as %d, which does not fit in %d digits",
                            val, WR_VAR_FXY(m_info->code), sdec, m_info->len);
                uint32_to_str(dec, digits, m_value);
                m_value[digits] = 0;
            }
            m_isset = true;
            /*snprintf(var->value, var->info->len + 2, "%ld", (long)dba_var_encode_int(val, var->info));*/
            break;
    }
}

void Var::setc(const char* val)
{
    // Allocate storage for the value
    allocate();

    switch (m_info->type)
    {
        case Vartype::String:
        case Vartype::Integer:
        case Vartype::Decimal: {
            // Guard against overflows
            unsigned len = strlen(val);
            /* Tweak the length to account for the extra leading '-' allowed for
            * negative numeric values */
            if (m_info->type == Vartype::String && m_info->type != Vartype::Binary && val[0] == '-')
                --len;
            if (len > m_info->len)
            {
                unset();
                if (options::var_silent_domain_errors)
                    return;
                error_domain::throwf("Value \"%s\" is too long for B%02d%03d (%s): maximum length is %d",
                        val, WR_VAR_X(m_info->code), WR_VAR_Y(m_info->code), m_info->desc, m_info->len);
            }
            strncpy(m_value, val, m_info->len + 1);
            m_value[m_info->len + 1] = 0;
            m_isset = true;
            break;
        }
        case Vartype::Binary:
            memcpy(m_value, val, m_info->len);
            if (m_info->bit_len % 8)
                m_value[m_info->len - 1] &= (1 << (m_info->bit_len % 8)) - 1;
            m_isset = true;
            break;
    }
}

void Var::setc_truncate(const char* val)
{
    switch (m_info->type)
    {
        case Vartype::Integer:
            error_type::throwf("setc_truncate: %01d%02d%03d (%s) is an integer",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::Decimal:
            error_type::throwf("setc_truncate: %01d%02d%03d (%s) is a decimal",
                    WR_VAR_FXY(m_info->code), m_info->desc);
        case Vartype::String:
        {
            // Allocate space for the value
            allocate();

            /* Guard against overflows */
            unsigned len = strlen(val);
            /* Tweak the length to account for the extra leading '-' allowed for
            * negative numeric values */
            if (m_info->type == Vartype::String && m_info->type != Vartype::Binary && val[0] == '-')
                --len;
            strncpy(m_value, val, m_info->len);
            m_value[m_info->len] = 0;
            if (len > m_info->len)
                m_value[m_info->len - 1] = '>';
            m_isset = true;
            break;
        }
        case Vartype::Binary:
        {
            // Allocate space for the value
            allocate();

            // Copy binary data normally
            memcpy(m_value, val, m_info->len);
            if (m_info->bit_len % 8)
                m_value[m_info->len - 1] &= (1 << (m_info->bit_len % 8)) - 1;
            m_isset = true;
            break;
        }
    }
}

void Var::sets(const std::string& val)
{
    // Allocate storage for the value
    allocate();

    switch (m_info->type)
    {
        case Vartype::String:
        case Vartype::Integer:
        case Vartype::Decimal: {
            // Guard against overflows
            size_t len = val.size();
            /* Tweak the length to account for the extra leading '-' allowed for
             * negative numeric values */
            if (m_info->type == Vartype::String && m_info->type != Vartype::Binary && val[0] == '-')
                --len;
            if (len > m_info->len)
            {
                unset();
                if (options::var_silent_domain_errors)
                    return;
                error_domain::throwf("Value \"%s\" is too long for B%02d%03d (%s): maximum length is %d",
                        val.c_str(), WR_VAR_X(m_info->code), WR_VAR_Y(m_info->code), m_info->desc, m_info->len);
            }
            strncpy(m_value, val.c_str(), m_info->len + 1);
            m_value[m_info->len + 1] = 0;
            m_isset = true;
            break;
        }
        case Vartype::Binary:
            if (val.size() < m_info->len)
            {
                // If val is too short, copy it and zero pad the rest
                memcpy(m_value, val.data(), val.size());
                for (unsigned i = val.size(); i < m_info->len; ++i)
                    m_value[i] = 0;
            } else {
                memcpy(m_value, val.data(), m_info->len);
                if (m_info->bit_len % 8)
                    m_value[m_info->len - 1] &= (1 << (m_info->bit_len % 8)) - 1;
            }
            m_isset = true;
            break;
    }
}

void Var::setf(const char* val)
{
    // NULL or empty string, unset()
    if (val == NULL || val[0] == 0)
    {
        unset();
        return;
    }

    switch (m_info->type)
    {
        case Vartype::String:
            // If we're a string, the formatted value is just the string itself
            setc(val);
            break;
        case Vartype::Binary:
            // If we are a binary, we need to convert from hex to binary first
#warning TODO: implement this
            throw error_unimplemented("hex to binary not yet implemented");
            break;
        case Vartype::Integer:
        case Vartype::Decimal:
            // For numeric values, the formatted value is just the stringified
            // result of enqd, and we can just parse it with strtod
            setd(strtod(val, NULL));
            break;
    }
}

void Var::unset()
{
    m_isset = false;
}

const Var* Var::enqa(Varcode code) const
{
    for (const Var* cur = m_attrs; cur && cur->code() <= code; cur = cur->m_attrs)
        if (cur->code() == code)
            return cur;
    return nullptr;
}

void Var::seta(const Var& attr)
{
	seta(unique_ptr<Var>(new Var(attr)));
}

void Var::seta(Var&& attr)
{
    seta(unique_ptr<Var>(new Var(attr)));
}

void Var::seta(unique_ptr<Var>&& attr)
{
    // Ensure that the attribute does not have attributes of its own
    attr->clear_attrs();

    if (!m_attrs || m_attrs->code() > attr->code())
    {
        // Append / insert
        attr->m_attrs = m_attrs;
        m_attrs = attr.release();
    }
    else if (m_attrs->code() == attr->code())
    {
        // Replace existing
        attr->m_attrs = m_attrs->m_attrs;
        Var* old_attrs = m_attrs;
        m_attrs = attr.release();
        old_attrs->m_attrs = nullptr;
        delete old_attrs;
    }
    else
        // Recursively proceed along the chain
        m_attrs->seta(move(attr));
}

void Var::unseta(Varcode code)
{
    if (!m_attrs || m_attrs->code() > code)
        // Past the end, nothing to do
        return;
    else if (m_attrs->code() == code)
    {
        // Got the right item, unlink and delete it
        Var* old_attrs = m_attrs;
        m_attrs = m_attrs->m_attrs;
        old_attrs->m_attrs = nullptr;
        delete old_attrs;
    }
    else
        // Recursively proceed along the chain
        m_attrs->unseta(code);
}

const Var* Var::next_attr() const
{
	return m_attrs;
}

void Var::setval(const Var& src)
{
    if (!src.isset())
    {
        unset();
        return;
    }
    switch (m_info->type)
    {
        case Vartype::String:
            if (src.info()->len < m_info->len)
            {
                allocate();
                // Fill only the first src.info()->len bytes, and 0-pad the
                // rest
                memcpy(m_value, src.m_value, src.info()->len + 2);
                for (unsigned i = src.info()->len; i < m_info->len; ++i)
                    m_value[i] = 0;
                m_isset = true;
            } else
                setc_truncate(src.m_value);
            break;
        case Vartype::Binary:
            if (src.info()->len < m_info->len)
            {
                allocate();
                memcpy(m_value, src.m_value, src.m_info->len);
                for (unsigned i = src.m_info->len; i < m_info->len; ++i)
                    m_value[i] = 0;
                m_isset = true;
            }
            else
                setc_truncate(src.m_value);
            break;
        case Vartype::Integer:
        case Vartype::Decimal:
            /// Convert and set the new value
            setd(convert_units(src.info()->unit, m_info->unit, src.enqd()));
            break;
    }
}

void Var::setattrs(const Var& src)
{
	clear_attrs();
	if (src.m_attrs)
		m_attrs = new Var(*src.m_attrs);
}

std::string Var::format(const char* ifundef) const
{
    if (!isset())
        return ifundef;
    switch (m_info->type)
    {
        case Vartype::Binary: {
            string res;
            for (unsigned i = 0; i < info()->len; ++i)
            {
                char buf[4];
                snprintf(buf, 4, "%02X", ((const unsigned char*)m_value)[i]);
                res += buf;
            }
            return res;
        }
        case Vartype::String:
            return m_value;
        case Vartype::Integer:
        case Vartype::Decimal: {
            Varinfo i = info();
            char buf[30];
            snprintf(buf, 20, "%.*f", i->scale > 0 ? i->scale : 0, enqd());
            return buf;
        }
    }
    error_consistency::throwf("unknown variable type %d", (int)m_info->type);
}


void Var::print_without_attrs(FILE* out) const
{
	// Print info
	fprintf(out, "%d%02d%03d %-.64s(%s): ",
			WR_VAR_F(m_info->code), WR_VAR_X(m_info->code), WR_VAR_Y(m_info->code),
			m_info->desc, m_info->unit);

    // Print value
    if (!isset())
    {
        fprintf(out, "(undef)\n");
        return;
    }
    switch (m_info->type)
    {
        case Vartype::Binary:
#warning TODO: implement this
            fprintf(out, "(printing binary values not yet implemented)\n");
            break;
        case Vartype::String:
        case Vartype::Integer:
            fprintf(out, "%s\n", m_value);
            break;
        case Vartype::Decimal:
            fprintf(out, "%.*f\n", m_info->scale > 0 ? m_info->scale : 0, enqd());
            break;
    }
}

void Var::print_without_attrs(std::ostream& out) const
{
    // Print info
    out << varcode_format(m_info->code) << " " << m_info->desc << "(" << m_info->unit << "): ";

    // Print value
    out << format("(undef)") << endl;
}

void Var::print(FILE* out) const
{
	print_without_attrs(out);

	// Print attrs
	for (const Var* a = next_attr(); a; a = a->next_attr())
	{
		fputs("           ", out);
		a->print_without_attrs(out);
	}
}

void Var::print(std::ostream& out) const
{
    print_without_attrs(out);

    // Print attrs
    for (const Var* a = next_attr(); a; a = a->next_attr())
    {
        out << "           " << endl;
        a->print_without_attrs(out);
    }
}

unsigned Var::diff(const Var& var) const
{
    if (code() != var.code())
    {
        notes::logf("varcodes differ: first is %d%02d%03d'%s', second is %d%02d%03d'%s'\n",
                WR_VAR_F(m_info->code), WR_VAR_X(m_info->code), WR_VAR_Y(m_info->code), m_info->desc,
                WR_VAR_F(var.info()->code), WR_VAR_X(var.info()->code), WR_VAR_Y(var.info()->code), var.info()->desc);
        return 1;
    }
    if (!isset() && !var.isset())
        return 0;
    if (!isset())
    {
        notes::logf("[%d%02d%03d %s] first value is NULL, second value is %s\n",
                WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                var.m_value);
        return 1;
    }
    if (!var.isset())
    {
        notes::logf("[%d%02d%03d %s] first value is %s, second value is NULL\n",
                WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                m_value);
        return 1;
    }
    switch (m_info->type)
    {
        case Vartype::Binary:
            if (m_info->bit_len != var.info()->bit_len)
            {
                notes::logf("[%d%02d%03d %s] binary values differ: first is %u bits, second is %u bits\n",
                        WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                        m_info->bit_len, var.info()->bit_len);
                return 1;
            }
            if (memcmp(m_value, var.m_value, m_info->len) != 0)
            {
                string dump1 = format();
                string dump2 = var.format();
                notes::logf("[%d%02d%03d %s] binary values differ: first is \"%s\", second is \"%s\"\n",
                        WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc, dump1.c_str(), dump2.c_str());
                return 1;
            }
            break;
        case Vartype::String:
            if (strcmp(m_value, var.m_value) != 0)
            {
                notes::logf("[%d%02d%03d %s] values differ: first is \"%s\", second is \"%s\"\n",
                        WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                        m_value, var.m_value);
                return 1;
            }
            break;
        case Vartype::Integer:
        case Vartype::Decimal:
            double val1 = enqd(), val2 = var.enqd();
            if (val1 != val2)
            {
                notes::logf("[%d%02d%03d %s] values differ: first is %f, second is %f\n",
                        WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                        val1, val2);
                return 1;
            }
            break;
    }

    if ((m_attrs != 0) != (var.m_attrs != 0))
    {
        if (m_attrs)
        {
            notes::logf("[%d%02d%03d %s] attributes differ: first has attributes, second does not\n",
                    WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()),
                    m_info->desc);
            return 1;
        }
        else
        {
            notes::logf("[%d%02d%03d %s] attributes differ: first does not have attributes, second does\n",
                    WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()),
                    m_info->desc);
            return 1;
        }
    } else {
        int count1 = 0, count2 = 0;

        for (const Var* a = next_attr(); a; a = a->next_attr()) ++count1;
        for (const Var* a = var.next_attr(); a; a = a->next_attr()) ++count2;

        if (count1 != count2)
        {
            notes::logf("[%d%02d%03d %s] attributes differ: first has %d, second has %d\n",
                WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()),
                m_info->desc, count1, count2);
            return abs(count1 - count2);
        } else {
            /* Check attributes */
            const Var* a1 = next_attr();
            const Var* a2 = var.next_attr();
            for ( ; a1 && a2; a1 = a1->next_attr(), a2 = a2->next_attr())
            {
                Varcode extracode = 0;
                if (a1->code() < a2->code())
                    extracode = a1->code();
                else if (a2->code() < a1->code())
                    extracode = a2->code();
                if (extracode)
                {
                    notes::logf("[%d%02d%03d %s] attributes differ: attribute %d%02d%03d exists only on first\n",
                        WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()),
                        m_info->desc, 
                        WR_VAR_F(extracode), WR_VAR_X(extracode), WR_VAR_Y(extracode));
                    return 1;
                }

                unsigned diff = a1->diff(*a2);
                if (diff)
                {
                    notes::logf("  comparing attr of variable ");
                    print(notes::log());
                    return diff;
                }
            }
        }
    }
    return 0;
}

}
