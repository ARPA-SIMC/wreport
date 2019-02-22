#ifndef WREPORT_BUFR_TRACE_H
#define WREPORT_BUFR_TRACE_H

// #define TRACE_DECODER

#ifdef TRACE_DECODER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

#endif
