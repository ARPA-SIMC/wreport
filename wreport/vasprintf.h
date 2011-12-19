#ifndef WIBBLE_VASPRINTF_H
#define WIBBLE_VASPRINTF_H

#if _WIN32 || __xlC__
int vasprintf (char **result, const char *format, va_list args);
#else
#include <stdio.h>
#endif

#endif
