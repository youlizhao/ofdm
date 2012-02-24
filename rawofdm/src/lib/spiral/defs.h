#include "config.h"

#if CODE == 121-91

#define K 7
#define RATE 2
#define POLYS { 121, 91 }
#define NUMSTATES 64
#define DECISIONTYPE unsigned char
#define DECISIONTYPE_BITSIZE 8
#define COMPUTETYPE unsigned char

// tests only
#define EBN0 3
#define TRIALS 10000
#define FRAMEBITS 2048

// GENERIC ONLY
#define METRICSHIFT 1
#define PRECISIONSHIFT 2
#define RENORMALIZE_THRESHOLD 137

#elif CODE == 133-171

#define K 8
#define RATE 2
#define POLYS { 133, 171 }
#define NUMSTATES 128
#define DECISIONTYPE unsigned char
#define DECISIONTYPE_BITSIZE 8
#define COMPUTETYPE unsigned char

#define EBN0 3
#define TRIALS 10000
#define FRAMEBITS 2048

#define METRICSHIFT 1
#define PRECISIONSHIFT 2
#define RENORMALIZE_THRESHOLD 156

#else

#error "Unknown code"

#endif

