#include "bufr.h"
#include "wreport/var.h"
#include <cstdarg>
#include "config.h"

// #define TRACE_INTERPRETER

#ifdef TRACE_INTERPRETER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace {

// Return a value with bitlen bits set to 1
static inline uint32_t all_ones(int bitlen)
{
    return ((1 << (bitlen - 1))-1) | (1 << (bitlen - 1));
}

}

namespace wreport {
namespace buffers {


BufrOutput::BufrOutput(std::string& out)
    : out(out), pbyte(0), pbyte_len(0)
{
}

void BufrOutput::add_bits(uint32_t val, int n)
{
    /* Mask for reading data out of val */
    uint32_t mask = 1 << (n - 1);
    int i;

    for (i = 0; i < n; i++) 
    {
        pbyte <<= 1;
        pbyte |= ((val & mask) != 0) ? 1 : 0;
        val <<= 1;
        pbyte_len++;

        if (pbyte_len == 8) 
            flush();
    }
#if 0
    IFTRACE {
        /* Prewrite it when tracing, to allow to dump the buffer as it's
         * written */
        while (e->out->len + 1 > e->out->alloclen)
            DBA_RUN_OR_RETURN(dba_rawmsg_expand_buffer(e->out));
        e->out->buf[e->out->len] = e->pbyte << (8 - e->pbyte_len);
    }
#endif
}

void BufrOutput::append_string(const Var& var, unsigned len_bits)
{
    append_string(var.enqc(), len_bits);
}

void BufrOutput::append_string(const char* val, unsigned len_bits)
{
    unsigned i, bi;
    bool eol = false;
    for (i = 0, bi = 0; bi < len_bits; ++i)
    {
        //TRACE("append_string:len: %d, i: %d, bi: %d, eol: %d\n", len_bits, i, bi, (int)eol);
        if (!eol && !val[i])
            eol = true;

        /* Strings are space-padded in BUFR */
        if (len_bits - bi >= 8)
        {
            append_byte(eol ? ' ' : val[i]);
            bi += 8;
        }
        else
        {
            /* Pad with zeros if writing strings with a number of bits
             * which is not multiple of 8.  It's not worth to implement
             * writing partial bytes at the moment and it's better to fail
             * gracefully, as my understanding is that this case should
             * never happen anyway. */
            add_bits(0, len_bits - bi);
            bi = len_bits;
        }
    }
}

void BufrOutput::append_binary(const unsigned char* val, unsigned len_bits)
{
    unsigned i, bi;
    for (i = 0, bi = 0; bi < len_bits; ++i)
    {
        /* Strings are space-padded in BUFR */
        if (len_bits - bi >= 8)
        {
            append_byte(val[i]);
            bi += 8;
        }
        else
        {
            add_bits(val[i], len_bits - bi);
            bi = len_bits;
        }
    }
}

void BufrOutput::append_var(Varinfo info, const Var& var)
{
    if (!var.isset())
    {
        append_missing(info->bit_len);
        return;
    }
    switch (info->type)
    {
        case Vartype::String:
            append_string(var.enqc(), info->bit_len);
            break;
        case Vartype::Binary:
            append_binary((const unsigned char*)var.enqc(), info->bit_len);
            break;
        case Vartype::Integer:
        case Vartype::Decimal:
            unsigned ival = info->encode_binary(var.enqd());
            add_bits(ival, info->bit_len);
            break;
    }
}

void BufrOutput::append_missing(Varinfo info)
{
    append_missing(info->bit_len);
}

void BufrOutput::flush()
{
    if (pbyte_len == 0) return;

    while (pbyte_len < 8)
    {
        pbyte <<= 1;
        pbyte_len++;
    }

    out.append((const char*)&pbyte, 1);
    pbyte_len = 0;
    pbyte = 0;
}


}
}
