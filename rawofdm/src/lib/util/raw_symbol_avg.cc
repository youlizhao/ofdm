/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_symbol_avg.h>
#include <gr_io_signature.h>

raw_symbol_avg_sptr
raw_make_symbol_avg (size_t vlen, size_t numv)
{
  return raw_symbol_avg_sptr(new raw_symbol_avg(vlen, numv));
}

raw_symbol_avg::raw_symbol_avg (size_t vlen, size_t numv)
  : gr_sync_decimator ("symbol_avg",
      gr_make_io_signature (1, 1, vlen*sizeof (float)),
      gr_make_io_signature (1, 1, vlen*sizeof (float)),
      numv),
    d_vlen(vlen), d_numv(numv)
{}

raw_symbol_avg::~raw_symbol_avg() {}

int
raw_symbol_avg::work(
    int noutput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
  const float *in = (const float *) input_items[0];
  const char *signal_in = (const char *)input_items[1];
  float *out = (float *) output_items[0];

  for (int i = 0; i < noutput_items; ++i) {
    for (int k = 0; k < d_vlen; ++k) {
      out[k] = 0.0f;
    }
    for (int j = 0; j < d_numv; ++j) {
      for (int k = 0; k < d_vlen; ++k) {
        out[k] += in[k];
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

