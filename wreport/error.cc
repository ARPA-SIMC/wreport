/*
 * wreport/error - wreport exceptions
 *
 * Copyright (C) 2005,2006  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "error.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <errno.h>
#include <regex.h>
#include <sstream>
#include "config.h"
#include "internals/compat.h"

namespace wreport {

static const char* err_desc[] = {
/*  0 */    "no error",
/*  1 */    "item not found",
/*  2 */    "wrong variable type",
/*  3 */    "cannot allocate memory",
/*  4 */    "ODBC error",
/*  5 */    "handle management error",
/*  6 */    "buffer is too short to fit data",
/*  7 */    "error reported by the system",
/*  8 */    "consistency check failed",
/*  9 */    "parse error",
/* 10 */    "write error",
/* 11 */    "regular expression error",
/* 12 */    "feature not implemented",
/* 13 */    "value outside valid domain"
};

const char* error::strerror(ErrorCode code)
{
	return err_desc[code];
}

#define MAKE_THROWF(errorname) \
    void errorname::throwf(const char* fmt, ...) { \
        /* Format the arguments */ \
        va_list ap; \
        va_start(ap, fmt); \
        char* cmsg; \
        if (vasprintf(&cmsg, fmt, ap) == -1) \
           cmsg = nullptr; \
        va_end(ap); \
        /* Convert to string */ \
        std::string msg(cmsg ? cmsg : fmt); \
        free(cmsg); \
        throw errorname(msg); \
    }

MAKE_THROWF(error_notfound)
MAKE_THROWF(error_type)
MAKE_THROWF(error_handles)
MAKE_THROWF(error_toolong)

error_system::error_system(const std::string& msg)
    : StringBase(msg + ": " + ::strerror(errno))
{
}

error_system::error_system(const std::string& msg, int errno_val)
    : StringBase(msg + ": " + ::strerror(errno_val))
{
}

MAKE_THROWF(error_system)

MAKE_THROWF(error_consistency)

static std::string build_parse_error(const char* file, int line, const std::string& msg)
{
    std::stringstream str;
    str << file << ":" << line << ": " << msg;
    return str.str();
}

error_parse::error_parse(const char* file, int line, const std::string& msg)
    : StringBase(build_parse_error(file, line, msg))
{
}

void error_parse::throwf(const char* file, int line, const char* fmt, ...)
{
	// Format the arguments
	va_list ap;
	va_start(ap, fmt);
	char* cmsg;
	vasprintf(&cmsg, fmt, ap);
	va_end(ap);

	// Convert to string
	std::string msg(cmsg);
	free(cmsg);
	throw error_parse(file, line, msg);
}

static std::string build_regexp_error(int code, void* re, const std::string& msg)
{
    char details[512];
    regerror(code, (regex_t*)re, details, 512);
    return msg + ": " + details;
}

error_regexp::error_regexp(int code, void* re, const std::string& msg)
    : StringBase(build_regexp_error(code, re, msg))
{
}

void error_regexp::throwf(int code, void* re, const char* fmt, ...)
{
	// Format the arguments
	va_list ap;
	va_start(ap, fmt);
	char* cmsg;
	vasprintf(&cmsg, fmt, ap);
	va_end(ap);

	// Convert to string
	std::string msg(cmsg);
	free(cmsg);
	throw error_regexp(code, re, msg);
}

MAKE_THROWF(error_unimplemented)
MAKE_THROWF(error_domain)

}
