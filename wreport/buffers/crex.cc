#include "crex.h"
#include "config.h"
#include "wreport/var.h"
#include <cstdarg>

/*
// #define TRACE_INTERPRETER

#ifdef TRACE_INTERPRETER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif
*/

using namespace std;

namespace wreport {
namespace buffers {

CrexInput::CrexInput(const std::string& in, const char* fname, size_t offset)
    : data(in.c_str()), data_len(in.size()), fname(fname), offset(offset),
      cur(data), has_check_digit(false)
{
    for (int i = 0; i < 5; ++i)
        sec[i] = 0;
}

bool CrexInput::eof() const { return cur >= data + data_len; }

unsigned CrexInput::remaining() const { return data + data_len - cur; }

void CrexInput::parse_error(const char* fmt, ...) const
{
    char* context;
    char* message;

    va_list ap;
    va_start(ap, fmt);
    if (vasprintf(&message, fmt, ap) == -1)
        message = nullptr;
    va_end(ap);

    if (asprintf(&context, "%s:%zu+%d: %s", fname, offset, (int)(cur - data),
                 message ? message : fmt) == -1)
        context = nullptr;

    string msg(context ? context : fmt);
    free(context);
    free(message);
    throw error_parse(msg);
}

void CrexInput::check_eof(const char* expected) const
{
    if (cur >= data + data_len)
        parse_error("end of CREX message while looking for %s", expected);
}

void CrexInput::check_available_data(unsigned datalen,
                                     const char* expected) const
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

void CrexInput::parse_value(int len, int is_signed, const char** d_start,
                            const char** d_end)
{
    // TRACE("crex_decoder_parse_value(%d, %s): ", len, is_signed ? "signed" :
    // "unsigned");

    /* Check for 2 more because we may have extra sign and check digit */
    check_available_data(len + 2, "end of data descriptor section");

    if (has_check_digit)
    {
        if ((*cur - '0') != expected_check_digit)
            parse_error("check digit mismatch: expected %d, found %d, rest of "
                        "message: %.*s",
                        expected_check_digit, (*cur - '0'), (int)remaining(),
                        cur);

        expected_check_digit = (expected_check_digit + 1) % 10;
        ++cur;
    }

    /* Set the value to start after the check digit (if present) */
    *d_start = cur;

    /* Cope with one extra character in case the sign is present */
    if (is_signed && *cur == '-')
        ++len;

    /* Go to the end of the message */
    cur += len;

    /* Set the end value, removing trailing spaces */
    for (*d_end = cur; *d_end > *d_start && isspace(*(*d_end - 1)); (*d_end)--)
        ;

    /* Skip trailing spaces */
    skip_spaces();

    // TRACE("%.*s\n", *d_end - *d_start, *d_start);
}

void CrexInput::debug_dump_next(const char* desc) const
{
    fputs(desc, stderr);
    fputs(": ", stderr);
    for (size_t i = 0; i < 30 && cur + i < data + data_len; ++i)
    {
        switch (*(cur + i))
        {
            case '\r': fputs("\\r", stderr); break;
            case '\n': fputs("\\n", stderr); break;
            default:   putc(*(cur + i), stderr); break;
        }
    }
    if (cur + 30 < data + data_len)
        fputs("…", stderr);
    putc('\n', stderr);
}

CrexOutput::CrexOutput(std::string& buf)
    : buf(buf), has_check_digit(0), expected_check_digit(0)
{
}

void CrexOutput::raw_append(const char* str, int len) { buf.append(str, len); }

void CrexOutput::raw_appendf(const char* fmt, ...)
{
    char sbuf[256];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(sbuf, 255, fmt, ap);
    va_end(ap);

    buf.append(sbuf, len);
}

void CrexOutput::encode_check_digit()
{
    if (!has_check_digit)
        return;

    char c = '0' + expected_check_digit;
    raw_append(&c, 1);
    expected_check_digit = (expected_check_digit + 1) % 10;
}

void CrexOutput::append_missing(Varinfo info)
{
    // TRACE("encode_b missing len: %d\n", info->len);
    for (unsigned i = 0; i < info->len; i++)
        raw_append("/", 1);
}

void CrexOutput::append_var(Varinfo info, const Var& var)
{
    if (!var.isset())
        return append_missing(info);

    int len = info->len;
    raw_append(" ", 1);
    encode_check_digit();

    switch (info->type)
    {
        case Vartype::String:
            raw_appendf("%-*.*s", len, len, var.enqc());
            // TRACE("encode_b string len: %d val %-*.*s\n", len, len, len,
            // var.value());
            break;
        case Vartype::Binary:
            throw error_unimplemented(
                "cannot encode a binary variable into a CREX message");
        case Vartype::Integer:
        case Vartype::Decimal: {
            int val = var.enqi();

            /* FIXME: here goes handling of active C table modifiers */

            if (val < 0)
                ++len;

            raw_appendf("%0*d", len, val);
            // TRACE("encode_b num len: %d val %0*d\n", len, len, val);
            break;
        }
    }
}

} // namespace buffers
} // namespace wreport
