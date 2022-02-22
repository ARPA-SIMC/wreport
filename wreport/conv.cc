#include "conv.h"
#include "error.h"
#include "codetables.h"
#include <vector>
#include <cmath>
#include <cstring>
#include <functional>
#include <algorithm>

using namespace std;

namespace wreport {

namespace {

struct Convert
{
    virtual ~Convert() {}
    virtual double convert(double val) const = 0;
};

struct ConvertIdent : public Convert
{
    double convert(double val) const override { return val; }
};

struct ConvertLinear : public Convert
{
    double mul;
    double add;

    ConvertLinear(double mul, double add) : mul(mul), add(add) {}

    double convert(double val) const override { return val * mul + add; }
};

struct ConvertFunction : public Convert
{
    std::function<double(double)> conv;

    ConvertFunction(std::function<double(double)> conv) : conv(conv) {}

    double convert(double val) const override { return conv(val); }
};

struct Conv
{
    const char* from;
    const char* to;
    const Convert* conv;

    Conv(const char* from, const char* to, const Convert* conv) : from(from), to(to), conv(conv) {}
    Conv(const Conv& o) = delete;
    Conv(Conv&& o)
        : from(o.from), to(o.to), conv(o.conv) { o.conv = nullptr; }
    ~Conv() { delete conv; }

    Conv& operator=(const Conv&) = delete;
    Conv& operator=(Conv&& o)
    {
        if (this == &o) return *this;
        from = o.from;
        to = o.to;
        delete conv;
        conv = o.conv;
        o.conv = nullptr;
        return *this;
    }

    int compare(const char* ofrom, const char* oto) const
    {
        if (int res = strcmp(from, ofrom)) return res;
        if (int res = strcmp(to, oto)) return res;
        return 0;
    }

    int compare(const Conv& o) const
    {
        if (int res = strcmp(from, o.from)) return res;
        if (int res = strcmp(to, o.to)) return res;
        return 0;
    }

    bool operator<(const Conv& o) const
    {
        return compare(o) < 0;
    }

    bool operator==(const Conv& o) const
    {
        return compare(o) == 0;
    }
};

struct ConvertRepository
{
    std::vector<Conv> repo;

    ConvertRepository()
    {
        add_linear("K",           "C", 1, -273.15001);
        add_linear("K",           "C/10", 10, -2731.5001);
        add_linear("C",           "C/10", 10, 0);
        add_linear("minuti",      "S", 60, 0);
        add_linear("MINUTE",      "S", 60, 0);
        add_linear("S",           "MINUTE", 1.0/60, 0);
        add_linear("G/M**3",      "KG/M**3", 0.001, 0);
        add_linear("KG/M**3",     "G/M**3", 1000, 0);
        add_linear("ug/m**3",     "KG/M**3", 0.000000001, 0);
        add_linear("KG/M**3",     "ug/m**3", 1000000000, 0);
        add_linear("mg/l",        "KG/M**3", 0.001, 0);
        add_linear("KG/M**3",     "mg/l", 1000, 0);
        add_linear("PA",          "KPA", 0.001, 0);
        add_linear("KPA",         "PA", 1000, 0);
        add_linear("M",           "MM", 1000, 0);
        add_linear("MM",          "M", 0.001, 0);
        add_linear("M",           "cm", 100, 0);
        add_linear("cm",          "M", 0.01, 0);
        add_linear("M",           "KM", 0.001, 0);
        add_linear("KM",          "M", 1000, 0);
        add_linear("%",           "PERCENT", 1, 0);
        add_linear("PERCENT",     "%", 1, 0);
        add_linear("M",           "FT", 3.2808, 0);
        add_linear("FT",          "M", 0.3048, 0);
        add_linear("cal/cm**2",   "J/M**2", 41868, 0);
        add_linear("J/M**2",      "cal/cm**2", 0.000023885, 0);
        add_linear("J/M**2",      "MJ/M**2", 0.000001, 0);
        add_linear("MJ/M**2",     "J/M**2", 1000000, 0);
        add_linear("m/s/10",      "M/S", 0.1, 0);
        add_linear("M/S",         "m/s/10", 10, 0);
        add_linear("nodi",        "M/S", 0.51444, 0);
        add_linear("M/S",         "nodi", 1.94384, 0);
        add_linear("PA",          "mBar", 0.01, 0);
        add_linear("mBar",        "PA", 100, 0);
        add_linear("PA",          "hPa", 0.01, 0);
        add_linear("hPa",         "PA", 100, 0);
        add_linear("PA",          "Bar", 0.00001, 0);
        add_linear("Bar",         "PA", 100000, 0);
        add_linear("PA",          "NBAR", 0.0001, 0 );
        add_linear("NBAR",        "PA", 10000, 0);
        add_linear("hm",          "M", 100, 0);
        add_linear("M",           "hm", 0.01, 0);
        add_linear("mm",          "M", 0.001, 0);
        add_linear("M",           "mm", 1000, 0);
        add_linear("mV",          "V", 0.001, 0);
        add_linear("V",           "mV", 1000, 0);
        add_linear("1/8",         "%", 12.5, 0);
        add_linear("%",           "1/8", 0.08, 0);
        add_linear("mm/10",       "KG/M**2", 0.1, 0);
        add_linear("GPM",         "m**2/s**2", 9.80665, 0);
        add_linear("GPM",         "M**2/S**2", 9.80665, 0);
        add_linear("MGP",         "m**2/s**2", 9.80665, 0);
        add_linear("MGP",         "M**2/S**2", 9.80665, 0);
        add_linear("m**2/s**2",   "GPM", 0.101971621, 0);
        add_linear("M**2/S**2",   "GPM", 0.101971621, 0);
        add_linear("m**2/s**2",   "MGP", 0.101971621, 0);
        add_linear("M**2/S**2",   "MGP", 0.101971621, 0);
        add_linear("cal/s/cm**2", "W/M**2", 41868, 0);
        add_linear("cal/h/cm**2", "W/M**2", 11.63, 0);
        add_linear("Mj/m**2",     "J/M**2", 1000000, 0);
        add_linear("RATIO",       "%", 100, 0);
        add_linear("%",           "RATIO", 0.01, 0);
        add_linear("ms/cm",       "S/M", 0.1, 0);
        add_linear("S/M",         "ms/cm", 10, 0);
        add_linear("mS/cm",       "S/M", 0.1, 0);
        add_linear("S/M",         "mS/cm", 10, 0);
        add_ident("A",            "YEAR");
        add_ident("YEARS",        "YEAR");
        add_ident("MON",          "MONTH");
        add_ident("MONTHS",       "MONTH");
        add_ident("D",            "DAY");
        add_ident("DAYS",         "DAY");
        add_ident("H",            "HOUR");
        add_ident("HOURS",        "HOUR");
        add_ident("MIN",          "MINUTE");
        add_ident("MINUTES",      "MINUTE");
        add_ident("SECONDS",      "SECOND");
        add_ident("SECOND",       "S");
        add_ident("sec",          "S");
        add_ident("G/G",          "KG/KG");
        add_ident("m**(2/3)/S",   "M**(2/3)/S");
        add_ident("DEGREE**2",    "DEGREE2");
        add_ident("KG/M**2",      "KGM-2");
        add_ident("KG/M**2",      "KG M-2");
        add_ident("J/M**2",       "JM-2");
        add_ident("Bq/L",         "BQ L-1");
        add_ident("DOBSON",       "DU");
        add_ident("LOG(1/M**2)",  "LOG (M-2)");
        add_ident("DEGREE",       "DEG");
        add_ident("DEGREE TRUE",  "DEG");
        add_ident("DEGREE TRUE",  "gsess");
        add_ident("m/sec",        "M/S");
        add_ident("m",            "M");
        add_ident("mm",           "KG/M**2");
        add_ident("degree true",  "DEGREE TRUE");
        add_ident("GPM",          "MGP");
        add_ident("W/m**2",       "W/M**2");
        add_ident("J M-2",        "J/M**2");
        add_ident("ppt",          "PART PER THOUSAND");
        add_function("octants",   "DEGREE TRUE", convert_octants_to_degrees, convert_degrees_to_octants);

        sort(repo.begin(), repo.end());
    }
    ~ConvertRepository()
    {
    }

    void add_ident(const char* from, const char* to)
    {
        repo.emplace_back(from, to, new ConvertIdent);
        repo.emplace_back(to, from, new ConvertIdent);

    }

    void add_linear(const char* from, const char* to, double mul, double add)
    {
        repo.emplace_back(from, to, new ConvertLinear(mul, add));
        repo.emplace_back(to, from, new ConvertLinear(1/mul, -add));
    }

    void add_function(const char* from, const char* to,
                      std::function<double(double)> forward,
                      std::function<double(double)> backward)
    {
        repo.emplace_back(from, to, new ConvertFunction(forward));
        repo.emplace_back(to, from, new ConvertFunction(backward));
    }

    const Convert* find(const char* from, const char* to)
    {
        int begin, end;

        // Binary search
        begin = -1, end = repo.size();
        while (end - begin > 1)
        {
            int cur = (end + begin) / 2;
            if (repo[cur].compare(from, to) > 0)
                end = cur;
            else
                begin = cur;
        }
        if (begin == -1 || repo[begin].compare(from, to) != 0)
            return nullptr;
        else
            return repo[begin].conv;
    }
};

}


double convert_units(const char* from, const char* to, double val)
{
    static ConvertRepository* repo = nullptr;
    if (strcmp(from, to) == 0)
        return val;
    if (!repo) repo = new ConvertRepository;
    const Convert* conv = repo->find(from, to);
    if (!conv)
        error_unimplemented::throwf("conversion from \"%s\" to \"%s\" is not implemented", from, to);
    return conv->convert(val);
}

/*
Cloud type VM		Cloud type 20012
WMO code 0509
Ch	0			10
Ch	1			11
Ch	...			...
Ch	9			19
Ch	/			60

WMO code 0515
Cm	0			20
Cm	1			21
Cm	...			...
Cm	9			29
Cm	/			61

WMO code 0513
Cl	0			30
Cl	1			31
Cl	...			...
Cl	9			39
Cl	/			62

				missing value: 63

WMO code 0500
Per i cloud type nei 4 gruppi ripetuti del synop:
	0..9 -> 0..9
	/ -> 59
*/
int convert_WMO0500_to_BUFR20012(int from)
{
	if (from >= 0 && from <= 9)
		return from;
	else if (from == -1) /* FIXME: check what is the value for '/' */
		return 59;
	else
		error_domain::throwf("value %d not found in WMO code table 0500", from);
}

int convert_BUFR20012_to_WMO0500(int from)
{
	if (from >= 0 && from <= 9)
		return from;
	else if (from == 59)
		return -1; /* FIXME: check what is the value for '/' */
	else
		error_domain::throwf(
				"BUFR 20012 value %d cannot be represented with WMO code table 0500",
				from);
}

int convert_WMO0509_to_BUFR20012(int from)
{
	if (from >= 0 && from <= 9)
		return from + 10;
	else if (from == -1) /* FIXME: check what is the value for '/' */
		return 60;
	else
		error_domain::throwf("value %d not found in WMO code table 0509", from);
}

int convert_BUFR20012_to_WMO0509(int from)
{
	if (from >= 10 && from <= 19)
		return from - 10;
	else if (from == 60)
		return -1; /* FIXME: check what is the value for '/' */
	else
		error_domain::throwf(
				"BUFR 20012 value %d cannot be represented with WMO code table 0509",
				from);
}

int convert_WMO0515_to_BUFR20012(int from)
{
	if (from >= 0 && from <= 9)
		return from + 20;
	else if (from == -1) /* FIXME: check what is the value for '/' */
		return 61;
	else
		error_domain::throwf("value %d not found in WMO code table 0515", from);
}

int convert_BUFR20012_to_WMO0515(int from)
{
	if (from >= 20 && from <= 29)
		return from - 20;
	else if (from == 61)
		return -1; /* FIXME: check what is the value for '/' */
	else
		error_domain::throwf(
				"BUFR 20012 value %d cannot be represented with WMO code table 0515",
				from);
}

int convert_WMO0513_to_BUFR20012(int from)
{
	if (from >= 0 && from <= 9)
		return from + 30;
	else if (from == -1) /* FIXME: check what is the value for '/' */
		return 62;
	else
		error_domain::throwf("value %d not found in WMO code table 0513", from);
}

int convert_BUFR20012_to_WMO0513(int from)
{
	if (from >= 30 && from <= 39)
		return from - 30;
	else if (from == 62)
		return -1; /* FIXME: check what is the value for '/' */
	else
		error_domain::throwf(
				"BUFR 20012 value %d cannot be represented with WMO code table 0513",
				from);
}

int convert_WMO4677_to_BUFR20003(int from)
{
	if (from <= 99)
		return from;
	else
		error_domain::throwf("cannot handle WMO4677 present weather (%d) values above 99", from);
}

int convert_BUFR20003_to_WMO4677(int from)
{
	if (from <= 99)
		return from;
	else
		error_domain::throwf("cannot handle BUFR 20003 present weather (%d) values above 99", from);
}

int convert_WMO4561_to_BUFR20004(int from)
{
	if (from <= 9)
		return from;
	else
		error_domain::throwf("cannot handle WMO4561 past weather (%d) values above 9", from);
}

int convert_BUFR20004_to_WMO4561(int from)
{
	if (from <= 9)
		return from;
	else
		error_domain::throwf("cannot handle BUFR 20004 present weather (%d) values above 9", from);
}

unsigned convert_BUFR08001_to_BUFR08042(unsigned from)
{
    // Handle missing value
    if (from & BUFR08001::MISSING)
        return BUFR08042::ALL_MISSING;

    int res = 0;
    if (from & BUFR08001::SIGWIND) res |= BUFR08042::SIGWIND;
    if (from & BUFR08001::SIGTH)   res |= BUFR08042::SIGTEMP | BUFR08042::SIGHUM;
    if (from & BUFR08001::MAXWIND) res |= BUFR08042::MAXWIND;
    if (from & BUFR08001::TROPO)   res |= BUFR08042::TROPO;
    if (from & BUFR08001::STD)     res |= BUFR08042::STD;
    if (from & BUFR08001::SURFACE) res |= BUFR08042::SURFACE;
    return res;
}

unsigned convert_BUFR08042_to_BUFR08001(unsigned from)
{
    if (from & BUFR08042::MISSING)
        return BUFR08001::ALL_MISSING;

    int res = 0;
    if (from & BUFR08042::SIGWIND) res |= BUFR08001::SIGWIND;
    if (from & BUFR08042::SIGHUM)  res |= BUFR08001::SIGTH;
    if (from & BUFR08042::SIGTEMP) res |= BUFR08001::SIGTH;
    if (from & BUFR08042::MAXWIND) res |= BUFR08001::MAXWIND;
    if (from & BUFR08042::TROPO)   res |= BUFR08001::TROPO;
    if (from & BUFR08042::STD)     res |= BUFR08001::STD;
    if (from & BUFR08042::SURFACE) res |= BUFR08001::SURFACE;
    return res;
}

double convert_icao_to_press(double from)
{
	static const double ZA = 5.252368255329;
	static const double ZB = 44330.769230769;
	static const double ZC = 0.000157583169442;
	static const double P0 = 1013.25;
	static const double P11 = 226.547172;

	if (from <= 11000)
		/* We are below 11 km */
		return P0 * pow(1 - from / ZB, ZA);
	else
		/* We are above 11 km */
		return P11 * exp(-ZC * (from - 11000));
}

double convert_press_to_icao(double from)
{
	throw error_unimplemented("converting pressure to ICAO height is not implemented");
}

double convert_octants_to_degrees(int from)
{
  if (from < 0 || from > 8)
    error_domain::throwf("cannot handle octants (%d) values below 0 or above 8", from);
  return from * 45.0;
}

int convert_degrees_to_octants(double from)
{
  if (from < 0 || from > 360)
    error_domain::throwf("cannot handle  degrees (%f) values below 0.0 or above 360.0", from);
  // 0 degrees = undefined direction (wind calm)
  if (from == 0)
    return 0;
  // North
  else if (from > 337.5 || from <= 22.5)
    return 8;
  else if (from >  22.5 && from <=  67.5)
    return 1;
  else if (from >  67.5 && from <= 112.5)
    return 2;
  else if (from > 112.5 && from <= 157.5)
    return 3;
  else if (from > 157.5 && from <= 202.5)
    return 4;
  else if (from > 202.5 && from <= 247.5)
    return 5;
  else if (from > 247.5 && from <= 292.5)
    return 6;
  else // if (from > 292.5 && from <= 337.5)
    // We really covered all steps, but gcc cannot detect it. I work around it
    // by making this an else instead of an else if.
    return 7;
}

unsigned convert_AOFVSS_to_BUFR08042(unsigned from)
{
    unsigned res = 0;
	if (from & (1 << 0))	// Maximum wind level
		res |= BUFR08042::MAXWIND;
	if (from & (1 << 1))	// Tropopause
		res |= BUFR08042::TROPO;
	/* Skipped */		// Part D, non-standard level data, p < 100hPa
	if (from & (1 << 3))	// Part C, standard level data, p < 100hPa
		res |= BUFR08042::STD;
	/* Skipped */		// Part B, non-standard level data, p > 100hPa
	if (from & (1 << 5))	// Part A, standard level data, p > 100hPa
		res |= BUFR08042::STD;
	if (from & (1 << 6))	// Surface
		res |= BUFR08042::SURFACE;
	if (from & (1 << 7))	// Significant wind level
		res |= BUFR08042::SIGWIND;
	if (from & (1 << 8))	// Significant temperature level
		res |= BUFR08042::SIGTEMP;
	return res;
}

}
