/*
 * Copyright 2010 Szymon Jakubczak
 * Copyright Phil Karn, KA9Q
 * Copyright 2010 Project Spiral
 */

#include "viterbi.h"

//decision_t is a BIT vector
typedef union {
  DECISIONTYPE   t[NUMSTATES/DECISIONTYPE_BITSIZE];
  unsigned int   w[NUMSTATES/32];
  unsigned short s[NUMSTATES/16];
  unsigned char  c[NUMSTATES/8];
} decision_t __attribute__ ((aligned (16)));

typedef union {
  COMPUTETYPE t[NUMSTATES];
} metric_t __attribute__ ((aligned (16)));


// nasty global state
extern COMPUTETYPE Branchtab[NUMSTATES/2*RATE] __attribute__ ((aligned (16)));

/* State info for instance of Viterbi decoder
 */
struct v {
  __attribute__ ((aligned (16))) metric_t metrics1; /* path metric buffer 1 */
  __attribute__ ((aligned (16))) metric_t metrics2; /* path metric buffer 2 */
  metric_t *old_metrics,*new_metrics; /* Pointers to path metrics, swapped on every bit */
  decision_t *decisions;   /* decisions */
  void (*update_blk)(struct v *vp, const COMPUTETYPE *syms, int nbits);
};
