/*
 * wreport/vartable - Load variable information from on-disk tables
 *
 * Copyright (C) 2005--2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

/*
 * TODO:
 *  - Future optimizations for dba_vartable can make use of string tables to
 *    store varinfo descriptions and units instead of long fixed-length
 *    records.
 *     - The string table cannot grow dynamically or it will invalidate the
 *       string pointers
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>		/* malloc, strtod, getenv */
#include <string.h>		/* strncmp */
#include <unistd.h>		/* access */
#include <ctype.h>		/* isspace */
#include <math.h>		/* rint */
#include <assert.h>		/* assert */
#include <limits.h>		/* PATH_MAX, INT_MIN, INT_MAX */

#include "vartable.h"
#include "error.h"

#include <map>

// #define TRACE_LOADER

#ifdef TRACE_LOADER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {

// Using a pointer: static constructors don't seem to work with old versions of xlC
static std::map<string, Vartable>* tables = 0;

// see http://stackoverflow.com/questions/1068849/how-do-i-determine-the-number-of-decimal-digits-of-an-integer-in-c
// for benchmarks
static int countdigits(int n)
{
	if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	/*      2147483647 is 2^31-1 - add more ifs as needed
	 *          and adjust this final return as well. */
	return 10;
}

Vartable::Vartable() {}
Vartable::~Vartable() {}

Varinfo Vartable::query(Varcode var) const
{
	int begin, end;

	// Binary search
	begin = -1, end = size();
	while (end - begin > 1)
	{
		int cur = (end + begin) / 2;
		if ((*this)[cur].var > var)
			end = cur;
		else
			begin = cur;
	}
	if (begin == -1 || (*this)[begin].var != var)
		error_notfound::throwf(
				"missing variable %d%02d%03d in table %s",
				WR_VAR_F(var), WR_VAR_X(var), WR_VAR_Y(var), m_id.c_str());
	else
		return Varinfo(&(*this)[begin]);
}

bool Vartable::contains(Varcode code) const
{
    int begin, end;

    // Binary search
    begin = -1, end = size();
    while (end - begin > 1)
    {
        int cur = (end + begin) / 2;
        if ((*this)[cur].var > code)
            end = cur;
        else
            begin = cur;
    }
    return begin != -1 && (*this)[begin].var == code;
}

Varinfo Vartable::query_altered(Varcode var, int scale, unsigned bit_len) const
{
    /* Get the normal variable */
    Varinfo start = query(var);

    /* Look for an existing alteration */
    const _Varinfo* current = NULL;
    const _Varinfo* i = start.impl();
    for ( ; i->alterations != NULL; i = i->alterations)
    {
        if (i->scale == scale && i->bufr_scale == scale && i->bit_len == bit_len)
        {
            current = i;
            break;
        }
    }
    if (current) return Varinfo(current);

    /* Not found: we need to create it */

    /* Duplicate the original varinfo */
    _Varinfo* newvi = new _Varinfo(*start);
    i->alterations = newvi;

    newvi->_ref = 1;
    newvi->alterations = NULL;
    // Mark that it is an alteration
    newvi->alteration = 1;

#if 0
    fprintf(stderr, "Before alteration(w:%d,s:%d): bl %d len %d scale %d\n",
            WR_ALT_WIDTH(change), WR_ALT_SCALE(change),
            i->bit_len, i->len, i->scale);
#endif

    /* Apply the alterations */
    newvi->bit_len = bit_len;
    if (newvi->is_string())
        newvi->len = newvi->bit_len / 8;
    else
        newvi->len = countdigits(1 << newvi->bit_len);

    newvi->scale = scale;
    newvi->bufr_scale = scale;

#if 0
    fprintf(stderr, "After alteration(w:%d,s:%d): bl %d len %d scale %d\n",
            WR_ALT_WIDTH(change), WR_ALT_SCALE(change),
            i->bit_len, i->len, i->scale);
#endif

    /* Postprocess the data, filling in minval and maxval */
    newvi->compute_range();

    return Varinfo(newvi);
}

const Vartable* Vartable::get(const char* id)
{
	return get(find_table(id));
}
const Vartable* Vartable::get(const std::pair<std::string, std::string>& idfile)
{
	if (!tables) tables = new std::map<string, Vartable>;

	// Return it from cache if we have it
	std::map<string, Vartable>::const_iterator i = tables->find(idfile.first);
	if (i != tables->end())
		return &(i->second);

	// Else, instantiate it
	Vartable* res = &(*tables)[idfile.first];
	res->load(idfile);

	return res;
}

namespace {
struct fd_closer
{
	FILE* fd;
	fd_closer(FILE* fd) : fd(fd) {}
	~fd_closer() { fclose(fd); }
};

static long getnumber(char* str)
{
	while (*str && isspace(*str))
		++str;
	if (!*str) return 0;
	if (*str == '-')
	{
		++str;
		// Eat spaces after the - (oh my this makes me sad)
		while (*str && isspace(*str))
			++str;
		return -strtol(str, 0, 10);
	} else
		return strtol(str, 0, 10);
}
}

void Vartable::load(const std::pair<std::string, std::string>& idfile)
{
	const std::string& id = idfile.first;
	const std::string& file = idfile.second;
	FILE* in = fopen(file.c_str(), "rt");
	char line[200];
	int line_no = 0;
	Varcode last_code = 0;
	bool is_bufr = id.size() != 7;

	if (in == NULL)
		throw error_system("opening BUFR/CREX table file " + file);

	fd_closer closer(in); // Close `in' on exit

	while (fgets(line, 200, in) != NULL)
	{
		/* FMT='(1x,A,1x,A64,47x,A24,I3,8x,I3)' */
		int i;

		// Append a new entry;
		resize(size()+1);
		_Varinfo* entry = &back();

		entry->flags = 0;

		/*fprintf(stderr, "Line: %s\n", line);*/

		line_no++;

		if (strlen(line) < 119)
			throw error_parse(file.c_str(), line_no, "line too short");

		/* Read starting B code */
		/*fprintf(stderr, "Entry: B%05d\n", bcode);*/
		entry->var = WR_STRING_TO_VAR(line + 2);

		if (entry->var < last_code)
			throw error_parse(file.c_str(), line_no, "input file is not sorted");
		last_code = entry->var;

		/* Read the description */
		memcpy(entry->desc, line+8, 64);
		/* Zero-terminate the description */
		for (i = 63; i >= 0 && isspace(entry->desc[i]); i--)
			;
		entry->desc[i+1] = 0;
		
		/* Read the BUFR type */
		memcpy(entry->unit, line+73, 24);
		memcpy(entry->bufr_unit, line+73, 24);
		/* Zero-terminate the type */
		for (i = 23; i >= 0 && isspace(entry->unit[i]); i--)
			;
		entry->unit[i+1] = 0;
		entry->bufr_unit[i+1] = 0;

		if (
				strcmp(entry->unit, "CCITTIA5") == 0 /*||
				strncmp(entry->unit, "CODE TABLE", 10) == 08*/
		) entry->flags |= VARINFO_FLAG_STRING;

		entry->scale = getnumber(line+98);
		entry->bufr_scale = getnumber(line+98);
		entry->bit_ref = getnumber(line+102);
		entry->bit_len = getnumber(line+115);

		if (strlen(line) < 157 || is_bufr)
		{
			entry->ref = 0;
			if (entry->is_string())
			{
				entry->len = entry->bit_len / 8;
			} else {
				int len = 1 << entry->bit_len;
				for (entry->len = 0; len != 0; entry->len++)
					len /= 10;
			}
		} else {
			/* Read the CREX type */
			memcpy(entry->unit, line+119, 24);
			/* Zero-terminate the type */
			for (i = 23; i >= 0 && isspace(entry->unit[i]); i--)
				;
			entry->unit[i+1] = 0;

			entry->scale = getnumber(line+143);
			entry->ref = 0;
			entry->len = getnumber(line+149);

			bool crex_is_string = (
					strcmp(entry->unit, "CHARACTER") == 0 /* ||
					strncmp(entry->unit, "CODE TABLE", 10) == 0 */
			);

			if (entry->is_string() != crex_is_string)
				error_parse::throwf(file.c_str(), line_no,
						"CREX is_string (%d) is different than BUFR is_string (%d)",
						(int)crex_is_string, (int)entry->is_string());
		}

		/* Postprocess the data, filling in minval and maxval */
		entry->compute_range();

		entry->alteration = 0;
		entry->alterations = NULL;

		/*
		fprintf(stderr, "Debug: B%05d len %d scale %d type %s desc %s\n",
				bcode, entry->len, entry->scale, entry->type, entry->desc);
		*/
		entry->do_ref();
	}

	m_id = id;
}

std::pair<std::string, std::string> Vartable::find_table(const std::string& id)
{
	vector<string> ids;
	ids.push_back(id);
	return find_table(ids);
}

std::pair<std::string, std::string> Vartable::find_table(const std::vector<std::string>& ids)
{
	static const char* exts[] = { ".txt", ".TXT", 0 };

	// Build a list of table search paths
	const char* dirlist[2] = { 0, 0 };
	int envcount = 0;
	if (char* env = getenv("WREPORT_EXTRA_TABLES"))
		dirlist[envcount++] = env;
	if (char* env = getenv("WREPORT_TABLES"))
		dirlist[envcount++] = env;
	else
		dirlist[envcount++] = TABLE_DIR;

	// For each search path
	for (vector<string>::const_iterator id = ids.begin(); id != ids.end(); ++id)
		for (int i = 0; i < envcount && dirlist[i]; ++i)
		{
			// Split on :
			size_t beg = 0;
			while (dirlist[i][beg])
			{
				size_t len = strcspn(dirlist[i] + beg, ":;");
				if (len)
				{
					for (const char** ext = exts; *ext; ++ext)
					{
						string pathname = string(dirlist[i], beg, len) + "/" + *id + *ext;
						TRACE("Trying pathname %s: ", pathname.c_str());
						if (access(pathname.c_str(), R_OK) == 0)
						{
							TRACE("found.\n");
							return make_pair(*id, pathname);
						} else
							TRACE("not found.\n");
					}
				}
				beg = beg + len + strspn(dirlist[i] + beg + len, ":;");
			}
		}

	if (ids.empty())
		error_consistency::throwf("find_table called with no candidates");

	string list;
	if (ids.size() == 1)
		list = ids[0];
	else
	{
		list = "{" + ids[0];
		for (size_t i = 1; i < ids.size(); ++i)
			list += "," + ids[i];
		list += "}";
	}
	if (envcount == 1)
		error_notfound::throwf("Cannot find %s.{txt,TXT} after trying in %s",
				list.c_str(), dirlist[0]);
	else
		error_notfound::throwf("Cannot find %s.{txt,TXT} after trying in %s and %s",
				list.c_str(), dirlist[0], dirlist[1]);
}

}

/* vim:set ts=4 sw=4: */
