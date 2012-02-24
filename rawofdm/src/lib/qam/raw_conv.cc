/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_conv.h>
#include <gr_io_signature.h>
#include <limits.h>

#include <stdio.h>

#include <parity.h>
#include <viterbi.h>

raw_conv_enc_sptr
 raw_make_conv_enc () {
   return raw_conv_enc_sptr (new raw_conv_enc());
}

raw_conv_enc::raw_conv_enc ()
  : gr_sync_interpolator ("conv_enc",
                       gr_make_io_signature (1, 1, sizeof(char)),
                       gr_make_io_signature (1, 1, sizeof(char)),
                       CHAR_BIT * RATE),
  d_sr(0)
{
}

int raw_conv_enc::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  char unsigned *out = (char unsigned *)output_items[0];

  const int polys[RATE] = POLYS;
  int sr = d_sr;

  for (int i = 0; i < noutput_items/(CHAR_BIT * RATE); ++i) {
    // encode each byte to bits
    int b = in[i];
    for (int j = 0; j < CHAR_BIT; ++j) {
      int bit = (b >> (CHAR_BIT-1-j)) & 1;
      sr = (sr << 1) | bit;
      for(int k = 0; k < RATE; k++)
        *(out++) = parity(sr & polys[k]);
    }
  }
  d_sr = sr;
  return noutput_items;
}



raw_conv_dec_sptr
 raw_make_conv_dec (int length) {
   return raw_conv_dec_sptr (new raw_conv_dec(length));
}

raw_conv_dec::raw_conv_dec (int length)
  : gr_sync_decimator ("conv_dec",
                       gr_make_io_signature (1, 1, sizeof(char)),
                       gr_make_io_signature (1, 1, sizeof(char)),
                       CHAR_BIT * RATE),
    d_length(0), d_vp(NULL), d_frameno(0)
{
  set_length(length);
}

void raw_conv_dec::set_length (int length) {
  // NOTE: length includes padding
  if (length > d_length) {
    // must allocate more!
    if (d_vp) viterbi_free(d_vp);
    d_vp = (v *)viterbi_alloc(length-CHAR_BIT);
    viterbi_spiral(d_vp);
    assert (d_vp != NULL);
  }
  d_length = length;
  set_output_multiple(d_length/CHAR_BIT);
}

int raw_conv_dec::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  unsigned char *out = (unsigned char *)output_items[0];

  assert(noutput_items * CHAR_BIT >= d_length);

  // TODO: consider depadding right here!

  // decode the whole frame in one shot!
  viterbi_init(d_vp, 0);
  // NOTE: d_length includes padding
  viterbi_decode(d_vp, in, out, d_length-CHAR_BIT);

  return d_length/CHAR_BIT;
}



raw_conv_punc_sptr
 raw_make_conv_punc (int nc, int np, unsigned char zero = 0) {
   return raw_conv_punc_sptr (new raw_conv_punc(nc, np, zero));
}

raw_conv_punc::raw_conv_punc (int nc, int np, unsigned char zero)
  : gr_block ("conv_punc",
              gr_make_io_signature (1, 1, sizeof(char)),
              gr_make_io_signature (1, 1, sizeof(char))),
    d_nc(nc), d_np(np), d_zero(zero)
{
  set_relative_rate(float(np)/float(nc));
  set_output_multiple(np);
  set_fixed_rate(true);
}

void raw_conv_punc::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
  unsigned ninputs = ninput_items_required.size ();
  for (unsigned i = 0; i < ninputs; i++)
    ninput_items_required[i] = fixed_rate_noutput_to_ninput(noutput_items);
}

int raw_conv_punc::fixed_rate_noutput_to_ninput(int noutput_items)
{
  return noutput_items / d_np * d_nc + history() - 1;
}

int raw_conv_punc::fixed_rate_ninput_to_noutput(int ninput_items)
{
  return std::max(0, ninput_items - (int)history() + 1) * d_np / d_nc;
}

int raw_conv_punc::general_work (int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  unsigned char *out = (unsigned char *)output_items[0];

  // TODO: make it faster by unrolling the inner loop in a switch
  for (int i = 0; i < noutput_items; i+= d_np) {
    for (int j = 0; j < d_nc; ++j) {
      out[j] = in[j];
    }
    for (int j = d_nc; j < d_np; ++j) {
      out[j] = d_zero;
    }
    in+= d_nc;
    out+= d_np;
  }

  consume(0, in - (const unsigned char *)input_items[0]);
  return out - (unsigned char *)output_items[0];
}

