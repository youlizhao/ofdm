/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_scrambler_bb.h>
#include <gr_io_signature.h>


raw_scrambler_bb_sptr
raw_make_scrambler_bb (unsigned length, unsigned seed)
{
  return raw_scrambler_bb_sptr (new raw_scrambler_bb (length, seed));
}

raw_scrambler_bb::raw_scrambler_bb (unsigned length, unsigned seed)
  : gr_block ("scrambler_bb",
                  gr_make_io_signature2 (1, 2, sizeof (char), sizeof (int)),
                  gr_make_io_signature (1, 1, sizeof (char))),
    d_length(length), d_reg(seed), d_count(0)
{
  // this aids scheduling
  set_output_multiple(d_length);
}

void
raw_scrambler_bb::forecast(int noutput_items, gr_vector_int &ninput_items_required)
{
  unsigned ninputs = ninput_items_required.size();
  ninput_items_required[0] = noutput_items;
  if (ninput_items_required.size() > 1) {
    ninput_items_required[1] = ((noutput_items-1) / d_length) + 1;
  }
}

/**
 * advance one byte
 * state holds the scrambling byte
 */
static inline void advance(unsigned &state) {
  for (int i = 0; i < 8; ++i) {
    /* x[-7] + x[-4] + 1 */
    state <<= 1;
    state |= ((state >> 7) & 1) ^ ((state >> 4) & 1);
  }
}

int
raw_scrambler_bb::general_work (int noutput_items,
                   gr_vector_int &ninput_items,
                   gr_vector_const_void_star &input_items,
                   gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *) input_items[0];
  const unsigned int *seed = input_items.size() > 1 ? (const unsigned int *) input_items[1] : 0;
  unsigned char *out = (unsigned char *) output_items[0];

  unsigned state = d_reg;

  // TODO: if d_length == 1, it slows us down to check the count each time

  unsigned nseeds = 0;
  for (int i = 0; i < noutput_items; ++i) {
    if (seed) {
      if (d_count == 0) {
        if (seed[nseeds]) {
          state = seed[nseeds] & 0x7F;
          // seed must always be non zero
          if (!state) state = 0x1;
          advance(state);
        }
        ++nseeds;
        d_count = d_length;
      }
      --d_count;
    }
    out[i] = in[i] ^ (state & 0xFF);
    advance(state);
  }
  d_reg = state;
  consume(0, noutput_items);
  if (seed) {
    consume(1, nseeds);
  }
  return noutput_items;
}
