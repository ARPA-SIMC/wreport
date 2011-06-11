/*
 * wreport/bulletin/buffers - Low-level I/O operations
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

#include <config.h>

#include "buffers.h"
//#include "opcode.h"
//#include "bulletin.h"
//#include "bulletin/dds-printer.h"
//#include "notes.h"
//
//#include <stddef.h>
//#include <stdlib.h>
//#include <string.h>
//#include <errno.h>
//#include <netinet/in.h>

#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

using namespace std;

namespace wreport {
namespace bulletin {

namespace {

const char* bufr_sec_names[] = {
    "Indicator section",
    "Identification section",
    "Optional section",
    "Data desription section",
    "Data section",
    "End section"
};


}

BufrInput::BufrInput(const std::string& in)
{
    reset(in);
}

void BufrInput::reset(const std::string& in)
{
    data = (const unsigned char*)in.data();
    data_len = in.size();
    fname = NULL;
    start_offset = 0;
    s4_cursor = 0;
    pbyte = 0;
    pbyte_len = 0;
    for (unsigned i = 0; i < sizeof(sec)/sizeof(sec[0]); ++i)
        sec[i] = 0;
}

void BufrInput::scan_section_length(unsigned sec_no)
{
    if (sec[sec_no] + 3 > data_len)
        parse_error(sec[sec_no], "section %d (%s) is too short to hold the section size indicator",
                sec_no, bufr_sec_names[sec_no]);

    sec[sec_no + 1] = sec[sec_no] + read_number(sec_no, 0, 3);

    if (sec[sec_no + 1] > data_len)
        parse_error(sec[sec_no], "section %d (%s) claims to end past the end of the BUFR message",
                sec_no, bufr_sec_names[sec_no]);
}

void BufrInput::scan_lead_sections()
{
    check_available_data(sec[0], 8, "section 0 of BUFR message (indicator section)");
    sec[1] = sec[0] + 8;
    scan_section_length(1);
}

void BufrInput::scan_other_sections(bool has_optional)
{
    if (has_optional)
    {
        scan_section_length(2);
    } else
        sec[3] = sec[2];

    for (unsigned i = 3; i < 5; ++i)
        scan_section_length(i);

    s4_cursor = sec[4] + 4;
}

int BufrInput::offset() const { return s4_cursor; }
int BufrInput::bits_left() const { return (data_len - s4_cursor) * 8 + pbyte_len; }

unsigned BufrInput::read_number(unsigned pos, unsigned byte_len) const
{
    unsigned res = 0;
    for (unsigned i = 0; i < byte_len; ++i)
    {
        res <<= 8;
        res |= data[pos + i];
    }
    return res;
}

uint32_t BufrInput::get_bits(unsigned n)
{
    uint32_t result = 0;

    if (s4_cursor == data_len)
        parse_error("end of buffer while looking for %d bits of bit-packed data", n);

    for (unsigned i = 0; i < n; i++) 
    {
        if (pbyte_len == 0) 
        {
            pbyte_len = 8;
            pbyte = data[s4_cursor++];
        }
        result <<= 1;
        if (pbyte & 0x80)
            result |= 1;
        pbyte <<= 1;
        pbyte_len--;
    }

    return result;
}

void BufrInput::debug_dump_next_bits(int count) const
{
    size_t cursor = s4_cursor;
    int pbyte = this->pbyte;
    int pbyte_len = this->pbyte_len;
    int i;

    for (i = 0; i < count; ++i) 
    {
        if (cursor == data_len)
            break;
        if (pbyte_len == 0) 
        {
            pbyte_len = 8;
            pbyte = data[cursor++];
            putc(' ', stderr);
        }
        putc((pbyte & 0x80) ? '1' : '0', stderr);
        pbyte <<= 1;
        --pbyte_len;
    }
}

void BufrInput::parse_error(const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&message, fmt, ap);
    va_end(ap);

    asprintf(&context, "%s:%zd+%u: %s", fname, start_offset, s4_cursor, message);
    free(message);

    string msg(context);
    free(context);

    throw error_parse(msg);
}

void BufrInput::parse_error(unsigned pos, const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&message, fmt, ap);
    va_end(ap);

    asprintf(&context, "%s:%zd+%u: %s", fname, start_offset, pos, message);
    free(message);

    string msg(context);
    free(context);
    throw error_parse(msg);
}

void BufrInput::parse_error(unsigned section, unsigned pos, const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&message, fmt, ap);
    va_end(ap);

    asprintf(&context, "%s:%zd+%u: %s (%db inside section %s)",
        fname, start_offset, sec[section] + pos, message,
        pos, bufr_sec_names[section]);
    free(message);

    string msg(context);
    free(context);
    throw error_parse(msg);
}

void BufrInput::check_available_data(unsigned pos, size_t datalen, const char* expected)
{
    if (pos + datalen > data_len)
        parse_error(pos, "end of BUFR message while looking for %s", expected);
}

void BufrInput::check_available_data(unsigned section, unsigned pos, size_t datalen, const char* expected)
{
    // TODO: check that sec[section] + pos + datalen > sec[section + 1] instead?
    // TODO: in that case, make a fake section 6 which starts at the end of
    // TODO: BUFR data
    if (sec[section] + pos + datalen > data_len)
        parse_error(section, pos, "end of BUFR message while looking for %s", expected);
}


CrexInput::CrexInput(const std::string& in)
    : data(in.c_str()), data_len(in.size()), fname(NULL), offset(0), cur(data)
{
    for (int i = 0; i < 5; ++i)
        sec[i] = 0;
}

bool CrexInput::eof() const
{
    return cur >= data + data_len;
}

unsigned CrexInput::remaining() const
{
    return data + data_len - cur;
}

void CrexInput::parse_error(const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&message, fmt, ap);
    va_end(ap);

    asprintf(&context, "%s:%zd+%d: %s", fname, offset, (int)(cur - data), message);

    string msg(context);
    free(context);
    free(message);
    throw error_parse(msg);
}

void CrexInput::check_eof(const char* expected) const
{
    if (cur >= data + data_len)
        parse_error("end of CREX message while looking for %s", expected);
}

void CrexInput::check_available_data(unsigned datalen, const char* expected) const
{
    if (cur + datalen > data + data_len)
        parse_error("end of CREX message while looking for %s", expected);
}

void CrexInput::skip_spaces()
{
    while (cur < data + data_len && isspace(*cur))
        ++cur;
}

void CrexInput::skip_data_and_spaces(unsigned datalen)
{
    cur += datalen;
    skip_spaces();
}

void CrexInput::mark_section_start(unsigned num)
{
    check_eof("start of section 1");
    if (cur >= data + data_len)
        parse_error("end of CREX message at start of section %u", num);
    sec[num] = cur - data;
}

void CrexInput::read_word(char* buf, size_t len)
{
    size_t i;
    for (i = 0; i < len - 1 && !eof() && !isspace(*cur); ++cur, ++i)
        buf[i] = *cur;
    buf[i] = 0;

    skip_spaces();
}

void CrexInput::debug_dump_next(const char* desc) const
{
    fputs(desc, stderr);
    fputs(": ", stderr);
    for (size_t i = 0; i < 30 && cur + i < data + data_len; ++i)
    {
        switch (*(cur + i))
        {
            case '\r':
                fputs("\\r", stderr);
            case '\n':
                fputs("\\n", stderr);
                break;
            default:
                putc(*(cur + i), stderr);
                break;
        }
    }
    if (cur + 30 < data + data_len)
        fputs("…", stderr);
    putc('\n', stderr);
}

}
}
