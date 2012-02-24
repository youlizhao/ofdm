/*
 * Copyright 2010 Szymon Jakubczak
 * Copyright Phil Karn, KA9Q
 * Copyright 2010 Project Spiral
 */

#include <stdlib.h>
#include <memory.h>

#include "viterbi_internal.h"

static inline void renormalize(COMPUTETYPE* X, COMPUTETYPE threshold) {
  if (X[0] > threshold) {
    COMPUTETYPE min = X[0];
    for(int i=0; i < NUMSTATES; i++)
      if (min > X[i])
        min = X[i];
    for(int i=0; i < NUMSTATES; i++)
      X[i]-= min;
  }
}

/* C-language butterfly */
static void BFLY(int i, int s, COMPUTETYPE * syms, struct v * vp, decision_t * d) {
  COMPUTETYPE metric,m0,m1,m2,m3;

  metric = 0;
  for (int j=0; j < RATE; j++)
    metric += (Branchtab[i+j*NUMSTATES/2] ^ syms[s*RATE+j])>>METRICSHIFT ;
  metric = metric>>PRECISIONSHIFT;

  const COMPUTETYPE max = ((RATE*((256 -1)>>METRICSHIFT))>>PRECISIONSHIFT);

  m0 = vp->old_metrics->t[i] + metric;
  m1 = vp->old_metrics->t[i+NUMSTATES/2] + (max - metric);
  m2 = vp->old_metrics->t[i] + (max - metric);
  m3 = vp->old_metrics->t[i+NUMSTATES/2] + metric;

  int decision0 = (signed int)(m0-m1) > 0;
  int decision1 = (signed int)(m2-m3) > 0;

  vp->new_metrics->t[2*i] = decision0 ? m1 : m0;
  vp->new_metrics->t[2*i+1] = decision1 ? m3 : m2;

  d->w[i/(sizeof(unsigned int)*8/2)+s*(sizeof(decision_t)/sizeof(unsigned int))] |=
    (decision0|decision1<<1) << ((2*i)&(sizeof(unsigned int)*8-1));
}


/* Update decoder with a block of demodulated symbols
 * Note that nbits is the number of decoded data bits, not the number
 * of symbols!
 */
static void viterbi_update_blk_GENERIC(struct v *vp, COMPUTETYPE *syms, int nbits) {
  decision_t *d = (decision_t *)vp->decisions;

  for (int s = 0; s < nbits; s++)
    memset(d+s,0,sizeof(decision_t));

  for (int s = 0; s < nbits; s++) {
    void *tmp;
    for(int i = 0; i < NUMSTATES/2; i++) {
      BFLY(i, s, syms, vp, vp->decisions);
    }

    renormalize(vp->new_metrics->t, RENORMALIZE_THRESHOLD);

    ///     Swap pointers to old and new metrics
    tmp = vp->old_metrics;
    vp->old_metrics = vp->new_metrics;
    vp->new_metrics = tmp;
  }
}

void viterbi_generic(struct v *vp) {
  vp->update_blk = viterbi_update_blk_GENERIC;
}

