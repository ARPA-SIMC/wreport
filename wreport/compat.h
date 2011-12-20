#ifndef WIBBLE_COMPAT_H
#define WIBBLE_COMPAT_H

#include "config.h"
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <cmath>

#if USE_OWN_VASPRINTF
static int vasprintf (char **result, const char *format, va_list args)
{
  const char *p = format;
  /* Add one to make sure that it is never zero, which might cause malloc
     to return NULL.  */
  int total_width = strlen (format) + 1;
  va_list ap;

  memcpy ((void *)&ap, (void *)&args, sizeof (va_list));

  while (*p != '\0')
    {
      if (*p++ == '%')
	{
	  while (strchr ("-+ #0", *p))
	    ++p;
	  if (*p == '*')
	    {
	      ++p;
	      total_width += abs (va_arg (ap, int));
	    }
	  else
	    total_width += strtoul (p, (char **) &p, 10);
	  if (*p == '.')
	    {
	      ++p;
	      if (*p == '*')
		{
		  ++p;
		  total_width += abs (va_arg (ap, int));
		}
	      else
	      total_width += strtoul (p, (char **) &p, 10);
	    }
	  while (strchr ("hlL", *p))
	    ++p;
	  /* Should be big enough for any format specifier except %s and floats.  */
	  total_width += 30;
	  switch (*p)
	    {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
	    case 'c':
	      (void) va_arg (ap, int);
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      (void) va_arg (ap, double);
	      /* Since an ieee double can have an exponent of 307, we'll
		 make the buffer wide enough to cover the gross case. */
	      total_width += 307;
	      break;
	    case 's':
	      total_width += strlen (va_arg (ap, char *));
	      break;
	    case 'p':
	    case 'n':
	      (void) va_arg (ap, char *);
	      break;
	    }
	  p++;
	}
    }
  *result = (char*)malloc (total_width);
  if (*result != NULL) {
    return vsprintf (*result, format, args);}
  else {
    return 0;
  }
}
#endif

#if USE_OWN_EXP10
#define exp10(x) pow((double)10.0, (double)(x))
#endif

#if USE_OWN_LOG10
/* Assuming x is finite and > 0:
   Return an approximation for n with 10^n <= x < 10^(n+1).
   The approximation is usually the right n, but may be off by 1 sometimes.  */
static int log10 (double x)
{
  int exp;
  double y;
  double z;
  double l;

  /* Split into exponential part and mantissa.  */
  y = frexp (x, &exp);
  if (!(y >= 0.0 && y < 1.0))
    abort ();
  if (y == 0.0)
    return INT_MIN;
  if (y < 0.5)
    {
      while (y < (1.0 / (1 << (GMP_LIMB_BITS / 2)) / (1 << (GMP_LIMB_BITS / 2))))
        {
          y *= 1.0 * (1 << (GMP_LIMB_BITS / 2)) * (1 << (GMP_LIMB_BITS / 2));
          exp -= GMP_LIMB_BITS;
        }
      if (y < (1.0 / (1 << 16)))
        {
          y *= 1.0 * (1 << 16);
          exp -= 16;
        }
      if (y < (1.0 / (1 << 8)))
        {
          y *= 1.0 * (1 << 8);
          exp -= 8;
        }
      if (y < (1.0 / (1 << 4)))
        {
          y *= 1.0 * (1 << 4);
          exp -= 4;
        }
      if (y < (1.0 / (1 << 2)))
        {
          y *= 1.0 * (1 << 2);
          exp -= 2;
        }
      if (y < (1.0 / (1 << 1)))
        {
          y *= 1.0 * (1 << 1);
          exp -= 1;
        }
    }
  if (!(y >= 0.5 && y < 1.0))
    abort ();
  /* Compute an approximation for l = log2(x) = exp + log2(y).  */
  l = exp;
  z = y;
  if (z < 0.70710678118654752444)
    {
      z *= 1.4142135623730950488;
      l -= 0.5;
    }
  if (z < 0.8408964152537145431)
    {
      z *= 1.1892071150027210667;
      l -= 0.25;
    }
  if (z < 0.91700404320467123175)
    {
      z *= 1.0905077326652576592;
      l -= 0.125;
    }
  if (z < 0.9576032806985736469)
    {
      z *= 1.0442737824274138403;
      l -= 0.0625;
    }
  /* Now 0.95 <= z <= 1.01.  */
  z = 1 - z;
  /* log2(1-z) = 1/log(2) * (- z - z^2/2 - z^3/3 - z^4/4 - ...)
     Four terms are enough to get an approximation with error < 10^-7.  */
  l -= 1.4426950408889634074 * z * (1.0 + z * (0.5 + z * ((1.0 / 3) + z * 0.25)));
  /* Finally multiply with log(2)/log(10), yields an approximation for
     log10(x).  */
  l *= 0.30102999566398119523;
  /* Round down to the next integer.  */
  return (int) l + (l < 0 ? -1 : 0);
}
#endif

#endif
