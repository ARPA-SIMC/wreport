#include "notes.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include "internals/compat.h"

using namespace std;

namespace wreport {
namespace notes {

// streambuf that discards all data
struct null_streambuf : public std::streambuf
{
    int overflow(int c) { return c; }
};

thread_local ostream* target = 0;
null_streambuf* null_sb = 0;
ostream* null_stream = 0;

void set_target(std::ostream& out)
{
    target = &out;
}

std::ostream* get_target()
{
    return target;
}

bool logs() throw ()
{
    return target != 0;
}

std::ostream& log() throw ()
{
    // If there is a target, use it
    if (target) return *target;

    // If there is no target, return an ostream that discards all data
    if (!null_sb)
    {
        null_sb = new null_streambuf;
        null_stream = new ostream(null_sb);
    }
    return *null_stream;
}

void logf(const char* fmt, ...)
{
    if (!target) return;

    char *c;
    va_list ap;
    va_start(ap, fmt);
    if (vasprintf(&c, fmt, ap) == -1)
        (*target) << fmt;
    else
    {
        (*target) << c;
        free(c);
    }
}

}
}
