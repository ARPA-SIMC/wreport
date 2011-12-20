#ifndef WIBBLE_COMPAT_H
#define WIBBLE_COMPAT_H

#include "config.h"

extern "C" {

#if USE_OWN_VASPRINTF
int vasprintf(char **result, const char *format, va_list args);
#endif

}

#if USE_OWN_EXP10
#define exp10(x) pow(10.0, (x))
#endif

#endif
