/*
 * Copyright 2010 Szymon Jakubczak
 * Copyright Phil Karn, KA9Q
 * Copyright 2010 Project Spiral
 */

#include <stdlib.h>
#include <math.h>
#include <memory.h>

#include <pmmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <mmintrin.h>

#include "parity.h"
#include "viterbi_internal.h"


// nasty global state
COMPUTETYPE Branchtab[NUMSTATES/2*RATE] __attribute__ ((aligned (16)));


/* Initialize Viterbi decoder for start of new frame */
void viterbi_init(struct v *vp, int starting_state) {
  for(int i=0; i < NUMSTATES; i++)
    vp->metrics1.t[i] = 63;

  vp->old_metrics = &vp->metrics1;
  vp->new_metrics = &vp->metrics2;
  vp->old_metrics->t[starting_state & (NUMSTATES-1)] = 0; /* Bias known start state */
}

/* Create a new instance of a Viterbi decoder */
struct v *viterbi_alloc(int len) {
  struct v *vp;
  static int Init = 0;

  if (!Init) {
    int state, i;
    int polys[RATE] = POLYS;
    for (state=0;state < NUMSTATES/2;state++) {
      for (i=0; i<RATE; i++) {
        Branchtab[i*NUMSTATES/2+state] = (polys[i] < 0) ^ parity((2*state) & abs(polys[i])) ? 255 : 0;
      }
    }
    Init++;
  }

  if (posix_memalign((void**)&vp, 16,sizeof(struct v)))
    return NULL;

  // NOTE: a frame-worth of decisions!
  if (posix_memalign((void**)&vp->decisions, 16,(len+(K-1))*sizeof(decision_t))) {
    free(vp);
    return NULL;
  }
  viterbi_init(vp, 0);

  return vp;
}

/* Viterbi chainback */
static void viterbi_chainback(struct v *vp,
      unsigned char *data, /* Decoded output data */
      unsigned int nbits, /* Number of data bits */
      unsigned int endstate) { /* Terminal encoder state */

  decision_t *d = vp->decisions;

  /* ADDSHIFT and SUBSHIFT make sure that the thing returned is a byte. */
#if (K-1<8)
#define ADDSHIFT (8-(K-1))
#define SUBSHIFT 0
#elif (K-1>8)
#define ADDSHIFT 0
#define SUBSHIFT ((K-1)-8)
#else
#define ADDSHIFT 0
#define SUBSHIFT 0
#endif

  /* Make room beyond the end of the encoder register so we can
   * accumulate a full byte of decoded data
   */
  endstate = (endstate % NUMSTATES) << ADDSHIFT;

  /* The store into data[] only needs to be done every 8 bits.
   * But this avoids a conditional branch, and the writes will
   * combine in the cache anyway
   */
  d += (K-1); /* Look past tail */
  while (nbits-- != 0) {
    int k = (d[nbits].w[(endstate>>ADDSHIFT)/32] >> ((endstate>>ADDSHIFT)%32)) & 1;
    endstate = (endstate >> 1) | (k << (K-2+ADDSHIFT));
    data[nbits>>3] = endstate>>SUBSHIFT;
  }

#undef ADDSHIRT
#undef SUBSHIFT
}

/* Delete instance of a Viterbi decoder */
void viterbi_free(struct v *vp) {
  if (vp != NULL) {
    free(vp->decisions);
    free(vp);
  }
}

void viterbi_decode(struct v *vp, const COMPUTETYPE *symbols, unsigned char *data, int nbits) {
  // vp = viterbi decoder
  // data = decoded
  // symbols = signal

  /* Decode it and make sure we get the right answer */
  /* Initialize Viterbi decoder */
  viterbi_init(vp, 0);

  /* Decode block */
  vp->update_blk(vp, symbols, nbits+(K-1));

  /* Do Viterbi chainback */
  viterbi_chainback(vp, data, nbits, 0);
}


