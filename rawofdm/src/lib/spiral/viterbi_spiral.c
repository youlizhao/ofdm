/*
 * Copyright 2010 Szymon Jakubczak
 * Copyright Phil Karn, KA9Q
 * Copyright 2010 Project Spiral
 */

#include <memory.h>

#include "viterbi_internal.h"

extern void FULL_SPIRAL(int nbits, COMPUTETYPE *Y, COMPUTETYPE *X, const COMPUTETYPE *syms, DECISIONTYPE *dec, COMPUTETYPE *Branchtab);
/* Update decoder with a block of demodulated symbols
 * Note that nbits is the number of (padded) decoded data bits, not the number
 * of symbols!
 */
static void viterbi_update_blk_SPIRAL(struct v *vp, const COMPUTETYPE *syms, int nbits) {
  decision_t *d = (decision_t *)vp->decisions;

  for (int s = 0; s < nbits; s++)
    memset(d+s, 0, sizeof(decision_t));

  FULL_SPIRAL(nbits, vp->new_metrics->t, vp->old_metrics->t, syms, d->t, Branchtab);
}

void viterbi_spiral(struct v *vp) {
  vp->update_blk = viterbi_update_blk_SPIRAL;
}
