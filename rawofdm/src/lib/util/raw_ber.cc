/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_ber.h>
#include <gr_io_signature.h>

raw_ber_avg_sptr
raw_make_ber_avg (size_t vlen, size_t numv)
{
  return raw_ber_avg_sptr(new raw_ber_avg(vlen, numv));
}

raw_ber_avg::raw_ber_avg (size_t vlen, size_t numv)
  : gr_sync_decimator ("ber_avg",
      gr_make_io_signature (1, 1, vlen*sizeof (unsigned char)),
      gr_make_io_signature (1, 1, vlen*sizeof (float)),
      numv),
    d_vlen(vlen), d_numv(numv)
{}

static int Bitcnt[] = {
 0, 1, 1, 2, 1, 2, 2, 3,
 1, 2, 2, 3, 2, 3, 3, 4,
 1, 2, 2, 3, 2, 3, 3, 4,
 2, 3, 3, 4, 3, 4, 4, 5,
 1, 2, 2, 3, 2, 3, 3, 4,
 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 1, 2, 2, 3, 2, 3, 3, 4,
 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6,
 4, 5, 5, 6, 5, 6, 6, 7,
 1, 2, 2, 3, 2, 3, 3, 4,
 2, 3, 3, 4, 3, 4, 4, 5,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6,
 4, 5, 5, 6, 5, 6, 6, 7,
 2, 3, 3, 4, 3, 4, 4, 5,
 3, 4, 4, 5, 4, 5, 5, 6,
 3, 4, 4, 5, 4, 5, 5, 6,
 4, 5, 5, 6, 5, 6, 6, 7,
 3, 4, 4, 5, 4, 5, 5, 6,
 4, 5, 5, 6, 5, 6, 6, 7,
 4, 5, 5, 6, 5, 6, 6, 7,
 5, 6, 6, 7, 6, 7, 7, 8,
};

int
raw_ber_avg::work(
    int noutput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *) input_items[0];
  float *out = (float *) output_items[0];

  for (int i = 0; i < noutput_items; ++i) {
    for (int k = 0; k < d_vlen; ++k) {
      out[k] = 0.0f;
    }
    for (int j = 0; j < d_numv; ++j) {
      for (int k = 0; k < d_vlen; ++k) {
        out[k] += Bitcnt[in[k]];
      }
      in+= d_vlen;
    }
    for (int k = 0; k < d_vlen; ++k) {
      out[k] /= d_numv;
    }
    out+= d_vlen;
  }
  return noutput_items;
}

