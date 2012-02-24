/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_intrlv.h>
#include <gr_io_signature.h>
#include <string.h>

struct BitInterleave {
  unsigned d_bpsc; // coded bits per subcarrier
  unsigned d_cbps; // coded bits per symbol
  static const int d_num_chunks = 16;

  BitInterleave(int ncarriers, int nbits)
    : d_bpsc(nbits), d_cbps(nbits * ncarriers) {}

  unsigned index(unsigned k) {
    // see 17.3.5.6 in 802.11a-1999, floor is implicit
    assert (k < d_cbps);
    unsigned s = std::max(d_bpsc / 2, (unsigned)1);
    unsigned i = (d_cbps / d_num_chunks) * (k % d_num_chunks) + (k / d_num_chunks);
    unsigned j = s * (i / s) + (i + d_cbps - (d_num_chunks * i / d_cbps)) % s;
    assert (j < d_cbps);
    return j;
  }

  void fill(std::vector<unsigned> &v, bool inverse) {
    v.resize(d_cbps);
    if (!inverse) {
      for (unsigned i = 0; i < d_cbps; ++i) {
        v[i] = index(i);
      }
    } else {

      for (unsigned i = 0; i < d_cbps; ++i) {
        v[index(i)] = i;
      }
    }
  }
};

raw_intrlv_bit_sptr
 raw_make_intrlv_bit (int ncarriers, int nbits, bool inverse) {
   return raw_intrlv_bit_sptr (new raw_intrlv_bit(ncarriers, nbits, inverse));
}

raw_intrlv_bit::raw_intrlv_bit (int ncarriers, int nbits, bool inverse)
  : gr_sync_block (inverse ? "intrlv_bit_dec" : "intrlv_bit_enc",
                       gr_make_io_signature (1, 1, sizeof(char)),
                       gr_make_io_signature (1, 1, sizeof(char)))
{
  BitInterleave bi(ncarriers, nbits);
  bi.fill(d_map, inverse);
  set_output_multiple(d_map.size());
}


int raw_intrlv_bit::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  char unsigned *out = (char unsigned *)output_items[0];

  for (unsigned i = 0; i < noutput_items; i+= d_map.size()) {
    for (unsigned j = 0; j < d_map.size(); ++j) {
      out[i + d_map[j]] = in[i + j];
    }
  }
  return noutput_items;
}



raw_intrlv_byte_sptr
 raw_make_intrlv_byte (int nrows, int slope, bool inverse) {
   return raw_intrlv_byte_sptr (new raw_intrlv_byte(nrows, slope, inverse));
}

// TODO: integrate intrlv_byte with stream_to_vector
// const int XXXX = 6*17;

raw_intrlv_byte::raw_intrlv_byte (int nrows, int slope, bool inverse)
  : gr_sync_block (inverse ? "intrlv_byte_dec" : "intrlv_byte_enc",
                  gr_make_io_signature (1, 1, nrows*slope*sizeof(char)),
                  gr_make_io_signature (1, 1, nrows*slope*sizeof(char))),
    d_length(nrows * slope), d_head(nrows-1)
{
  // load up fifos
  d_fifos.resize(nrows);
  for (int i = 0; i < nrows; ++i) {
    memset(d_fifos[i].buf, 0, d_length);
  }
  if (inverse) {
    for (int i = 0; i < nrows; ++i) {
      d_fifos[i].n = slope * (nrows - 1 - i);
    }
  } else {
    for (int i = 0; i < nrows; ++i) {
      d_fifos[i].n = slope * i;
    }
  }
}

int raw_intrlv_byte::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  char unsigned *out = (char unsigned *)output_items[0];

  int nrows = d_fifos.size();

  // TODO: don't output first d_head items

  for (unsigned i = 0; i < noutput_items; ++i, in+= d_length, out+= d_length) {
    for (unsigned j = 0; j < d_length; ++j) {
      out[j] = d_fifos[j % nrows].push(in[j]);
    }
  }

  return noutput_items;
}
