#include "var.h"
#include "notes.h"
#include "options.h"
#include "vartable.h"
#include "fast.h"
#include "conv.h"
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include <iostream>

using namespace std;

namespace wreport {

Var::Var(Varinfo info)
    : m_info(info), m_value(nullptr), m_attrs(nullptr)
{
}

Var::Var(Varinfo info, int val)
    : m_info(info), m_value(nullptr), m_attrs(nullptr)
{
    seti(val);
}

Var::Var(Varinfo info, double val)
    : m_info(info), m_value(nullptr), m_attrs(nullptr)
{
    setd(val);
}

Var::Var(Varinfo info, const char* val)
    : m_info(info), m_value(nullptr), m_attrs(nullptr)
{
    setc(val);
}

Var::Var(const Var& var)
    : m_info(var.m_info), m_value(nullptr), m_attrs(nullptr)
{
    /* Copy the value */
    if (var.m_value != nullptr)
    {
        if (var.m_info->is_binary())
            set_binary((const unsigned char*)var.m_value);
        else
            setc(var.m_value);
    }

    /* Copy the attributes */
    if (var.m_attrs)
        m_attrs = new Var(*var.m_attrs);
}

Var::Var(Var&& var)
    : m_info(var.m_info), m_value(var.m_value), m_attrs(var.m_attrs)
{
    // Unset the source variable
    var.m_value = nullptr;
    var.m_attrs = nullptr;
}

#if 0
Var::Var(const Var& var, bool with_attrs)
    : m_info(var.m_info), m_value(NULL), m_attrs(NULL)
{
    /* Copy the value */
    if (var.m_value != NULL)
        setc(var.m_value);

    /* Copy the attributes */
    if (with_attrs && var.m_attrs)
        m_attrs = new Var(*var.m_attrs);
}
#endif

Var::Var(Varinfo info, const Var& var)
    : m_info(info), m_value(nullptr), m_attrs(nullptr)
{
	copy_val(var);
}

Var& Var::operator=(const Var& var)
{
	// Deal with a = a;
	if (&var == this) return *this;

	// Copy info
	m_info = var.m_info;

    // Copy value
    if (m_value != nullptr)
    {
        delete[] m_value;
        m_value = nullptr;
    }
    if (var.m_value != nullptr)
        setc(var.m_value);

	// Copy attributes
	copy_attrs(var);
	return *this;
}

Var& Var::operator=(Var&& var)
{
    if (&var == this) return *this;
    delete[] m_value;
    delete m_attrs;
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
    if (!m_value && !var.m_value) return true;
    if (!m_value || !var.m_value) return false;

    // Compare value
    if (m_info->is_binary())
        error_unimplemented::throwf("comparing opaque variable data is still not implemented");
    else if (m_info->is_string() || m_info->scale == 0)
        return strcmp(m_value, var.m_value) == 0;
    else
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

Varcode Var::code() const throw ()
{
    return m_info->code;
}

Varinfo Var::info() const throw ()
{
	return m_info;
}

bool Var::isset() const throw ()
{
	return m_value != NULL;
}

void Var::clear_attrs()
{
    delete m_attrs;
    m_attrs = 0;
}

static inline void fail_if_undef(const char* what, const Varinfo& info, const char* val)
{
    if (!val)
        error_notfound::throwf("%s: %01d%02d%03d (%s) is not defined",
                what, WR_VAR_FXY(info->code), info->desc);
}

// Ensure that we're working with a numeric value that is not undefined
static inline void want_number(const char* what, Varinfo info, const char* val)
{
    if (!val)
        error_notfound::throwf("%s: %01d%02d%03d (%s) is not defined",
                what, WR_VAR_FXY(info->code), info->desc);
    if (info->is_string())
        error_type::throwf("%s: %01d%02d%03d (%s) is a string",
                what, WR_VAR_FXY(info->code), info->desc);
    if (info->is_binary())
        error_type::throwf("%s: %01d%02d%03d (%s) is an opaque binary",
                what, WR_VAR_FXY(info->code), info->desc);
}

// Ensure that we're working with a numeric value
static inline void want_number(const char* what, Varinfo info)
{
    if (info->is_string())
        error_type::throwf("%s: %01d%02d%03d (%s) is a string",
                what, WR_VAR_FXY(info->code), info->desc);
    if (info->is_binary())
        error_type::throwf("%s: %01d%02d%03d (%s) is an opaque binary",
                what, WR_VAR_FXY(info->code), info->desc);
}

int Var::enqi() const
{
    want_number("enqi", m_info, m_value);
    return strtol(m_value, 0, 10);
}

double Var::enqd() const
{
    want_number("enqd", m_info, m_value);
    return m_info->decode_decimal(strtol(m_value, 0, 10));
}

const char* Var::enqc() const
{
    fail_if_undef("enqc", m_info, m_value);
    return m_value;
}

void Var::seti(int val)
{
    want_number("seti", m_info);

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
    if (!m_value && !(m_value = new char[m_info->len + 2]))
        throw error_alloc("allocating space for Var value");

    // Set the value
	// FIXME: not thread safe, and why are we not just telling itoa to write on m_value??
	/* We add 1 to the length to cope with the '-' sign */
	strcpy(m_value, itoa(val, m_info->len + 1));
	/*snprintf(var->value, var->info->len + 2, "%d", val);*/
}

void Var::setd(double val)
{
    want_number("setd", m_info);

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

	/* Set the value */
	if (m_value == NULL && 
		(m_value = new char[m_info->len + 2]) == NULL)
		throw error_alloc("allocating space for Var value");

#warning FIXME: not thread safe
    strcpy(m_value, itoa(m_info->encode_decimal(val), m_info->len + 1));
    /*snprintf(var->value, var->info->len + 2, "%ld", (long)dba_var_encode_int(val, var->info));*/
}

void Var::setc(const char* val)
{
	/* Set the value */
	if (m_value == NULL &&
		(m_value = new char[m_info->len + 2]) == NULL)
		throw error_alloc("allocating space for Var value");

	/* Guard against overflows */
	unsigned len = strlen(val);
	/* Tweak the length to account for the extra leading '-' allowed for
	 * negative numeric values */
	if (!m_info->is_string() && val[0] == '-')
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
}

void Var::set_binary(const unsigned char* val)
{
    /* Set the value */
    if (m_value == NULL &&
        (m_value = new char[m_info->len + 2]) == NULL)
        throw error_alloc("allocating space for Var value");
    memcpy(m_value, val, m_info->len);
}

void Var::setc_truncate(const char* val)
{
    /* Set the value */
    if (m_value == NULL &&
            (m_value = new char[m_info->len + 2]) == NULL)
        throw error_alloc("allocating space for Var value");

    /* Guard against overflows */
    unsigned len = strlen(val);
    /* Tweak the length to account for the extra leading '-' allowed for
     * negative numeric values */
    if (!m_info->is_string() && val[0] == '-')
        --len;
    strncpy(m_value, val, m_info->len);
    m_value[m_info->len] = 0;
    if (len > m_info->len)
        m_value[m_info->len - 1] = '>';
}

void Var::set_from_formatted(const char* val)
{
    // NULL or empty string, unset()
    if (val == NULL || val[0] == 0)
    {
        unset();
        return;
    }

    Varinfo i = info();

    // If we're a string, it's easy
    if (i->is_string())
    {
        setc(val);
        return;
    }

    // Else use strtod
    setd(strtod(val, NULL));
}

void Var::unset()
{
	if (m_value != NULL) delete[] m_value;
	m_value = NULL;
}

const Var* Var::enqa(Varcode code) const
{
	for (const Var* cur = m_attrs; cur != NULL && cur->code() <= code; cur = cur->m_attrs)
		if (cur->code() == code)
			return cur;
	return NULL;
}

const Var* Var::enqa_by_associated_field_significance(unsigned significance) const
{
    switch (significance)
    {
        case 1:
        case 8:
            return enqa(WR_VAR(0, 33, 2));
	    break;
        case 2:
	    return enqa(WR_VAR(0, 33, 3));
	    break;
	case 3:
	case 4:
	case 5:
            // Reserved: ignored
            notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            break;
        case 6: return enqa(WR_VAR(0, 33, 50)); break;
        case 7: return enqa(WR_VAR(0, 33, 40)); break;
        case 21: return enqa(WR_VAR(0, 33, 41)); break;
        case 63:
            /*
             * Ignore quality information if B31021 is missing.
             * The Guide to FM94-BUFR says:
             *   If the quality information has no meaning for some
             *   of those following elements, but the field is
             *   still there, there is at present no explicit way
             *   to indicate "no meaning" within the currently
             *   defined meanings. One must either redefine the
             *   meaning of the associated field in its entirety
             *   (by including 0 31 021 in the message with a data
             *   value of 63 - "missing value") or remove the
             *   associated field bits by the "cancel" operator: 2
             *   04 000.
             */
            break;
        default:
	    if (significance >= 9 and significance <= 20)
		    // Reserved: ignored
		    notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
			    significance);
	    else if (significance >= 22 and significance <= 62)
		    notes::logf("Ignoring B31021=%d, which is documented as 'reserved for local use'\n",
				    significance);
	    else
		    error_unimplemented::throwf("C04 modifiers with B31021=%d are not supported", significance);
	    break;
    }
    return 0;
}

void Var::seta(const Var& attr)
{
	seta(unique_ptr<Var>(new Var(attr)));
}

void Var::seta(unique_ptr<Var>&& attr)
{
	// Ensure that the attribute does not have attributes of its own
	attr->clear_attrs();

	if (m_attrs == NULL || m_attrs->code() > attr->code())
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
		old_attrs->m_attrs = NULL;
		delete old_attrs;
	}
    else
        // Recursively proceed along the chain
        m_attrs->seta(move(attr));
}

void Var::unseta(Varcode code)
{
	if (m_attrs == NULL || m_attrs->code() > code)
		// Past the end, nothing to do
		return;
	else if (m_attrs->code() == code)
	{
		// Got the right item, unlink and delete it
		Var* old_attrs = m_attrs;
		m_attrs = m_attrs->m_attrs;
		old_attrs->m_attrs = NULL;
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

void Var::copy_val(const Var& src)
{
    copy_val_only(src);
    copy_attrs(src);
}

void Var::copy_val_only(const Var& src)
{
    if (src.m_value == NULL)
    {
        unset();
    } else {
        if (m_info->is_string())
        {
            if (src.info()->len > m_info->len)
                setc_truncate(src.m_value);
            else
                setc(src.m_value);
        } else {
            /* Convert and set the new value */
            setd(convert_units(src.info()->unit, m_info->unit, src.enqd()));
        }
    }
}

void Var::copy_attrs(const Var& src)
{
	clear_attrs();
	if (src.m_attrs)
		m_attrs = new Var(*src.m_attrs);
}

std::string Var::format(const char* ifundef) const
{
    if (m_value == NULL)
        return ifundef;
    else if (info()->is_binary())
    {
        string res;
        for (unsigned i = 0; i < info()->len; ++i)
        {
            char buf[4];
            snprintf(buf, 4, "%X", ((const unsigned char*)m_value)[i]);
            res += buf;
        }
        return res;
    }
    else if (info()->is_string())
        return m_value;
    else
    {
        Varinfo i = info();
        char buf[30];
        snprintf(buf, 20, "%.*f", i->scale > 0 ? i->scale : 0, enqd());
        return buf;
    }
}


void Var::print_without_attrs(FILE* out) const
{
	// Print info
	fprintf(out, "%d%02d%03d %-.64s(%s): ",
			WR_VAR_F(m_info->code), WR_VAR_X(m_info->code), WR_VAR_Y(m_info->code),
			m_info->desc, m_info->unit);

	// Print value
	if (m_value == NULL)
		fprintf(out, "(undef)\n");
	else if (m_info->is_string() || m_info->scale == 0)
		fprintf(out, "%s\n", m_value);
	else
		fprintf(out, "%.*f\n", m_info->scale > 0 ? m_info->scale : 0, enqd());
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
    if (m_value == NULL && var.m_value == NULL)
        return 0;
    if (m_value == NULL)
    {
        notes::logf("[%d%02d%03d %s] first value is NULL, second value is %s\n",
                WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                var.m_value);
        return 1;
    }
    if (var.m_value == NULL)
    {
        notes::logf("[%d%02d%03d %s] first value is %s, second value is NULL\n",
                WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                m_value);
        return 1;
    }
    if (m_info->is_binary())
    {
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
    } else if (m_info->is_string() || m_info->scale == 0) {
        if (strcmp(m_value, var.m_value) != 0)
        {
            notes::logf("[%d%02d%03d %s] values differ: first is \"%s\", second is \"%s\"\n",
                    WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                    m_value, var.m_value);
            return 1;
        }
    }
    else
    {
        double val1 = enqd(), val2 = var.enqd();
        if (val1 != val2)
        {
            notes::logf("[%d%02d%03d %s] values differ: first is %f, second is %f\n",
                    WR_VAR_F(code()), WR_VAR_X(code()), WR_VAR_Y(code()), m_info->desc,
                    val1, val2);
            return 1;
        }
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

/* vim:set ts=4 sw=4: */
