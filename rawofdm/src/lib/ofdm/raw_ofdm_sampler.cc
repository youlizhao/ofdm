/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_ofdm_sampler.h>
#include <gr_io_signature.h>
#include <gr_expj.h>
#include <iostream>
#include <cstring>

raw_ofdm_sampler_sptr
raw_make_ofdm_sampler (unsigned int fft_length,
                       unsigned int symbol_length,
                       unsigned int timeout)
{
  return raw_ofdm_sampler_sptr (
    new raw_ofdm_sampler (fft_length, symbol_length, timeout)
  );
}

raw_ofdm_sampler::raw_ofdm_sampler (unsigned int fft_length,
                                    unsigned int symbol_length,
                                    unsigned int timeout)
  : gr_block ("ofdm_sampler",
      gr_make_io_signature2 (2, 2,
        sizeof (gr_complex),
        sizeof(char)),
      gr_make_io_signature2 (2, 2,
        sizeof (gr_complex)*fft_length,
        sizeof(char))),
    d_timeout_max(timeout),
    d_fft_length(fft_length),
    d_symbol_length(symbol_length),
    d_state(STATE_NO_SIG)
{
}

void
raw_ofdm_sampler::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
  int nreqd = noutput_items * d_symbol_length;
  unsigned ninputs = ninput_items_required.size();
  for (unsigned i = 0; i < ninputs; i++)
    ninput_items_required[i] = nreqd;
}


int
raw_ofdm_sampler::general_work (int noutput_items,
                                gr_vector_int &ninput_items,
                                gr_vector_const_void_star &input_items,
                                gr_vector_void_star &output_items)
{
  const gr_complex *in = (const gr_complex *) input_items[0];
  const char *trigger = (const char *) input_items[1];

  gr_complex *out = (gr_complex *) output_items[0];
  char *outsig = (char *) output_items[1];

  int cp_length = d_symbol_length - d_fft_length;

// trigger[t] != 0 indicates that the input should be sampled here.
// Next input should be sampled d_symbol_length later.

  int nin = 0;
  int nout = 0;

  outsig[nout] == 0;

  while((nin < ninput_items[0] - d_symbol_length)
     && (nout < noutput_items)) {

    if (d_state == STATE_SIGNAL) {
      if(--d_timeout == 0) {
#if 0
        std::cerr << "TIMEOUT" << std::endl;
#endif
        d_state = STATE_NO_SIG;
      } else {
        // copy symbol to output
        // (it could be a spurious last symbol, but we'd better be safe and include it)
        memcpy(out + nout * d_fft_length,
               in + nin,
               d_fft_length*sizeof(gr_complex));
        ++nout;
        outsig[nout] = 0;
      }
    }

    nin += d_symbol_length;

    // look for trigger
    for(int j = nin - d_symbol_length; j < nin; ++j) {
      if(trigger[j]) {
        if (j + d_symbol_length <= nin + cp_length) {
          // we don't allow symbols shorter than cp_length
          if (nout) {
            if (outsig[nout-1]) // haven't we just copied it out?
              continue; // FIXME: this is edge programming!! :-(
            --nout; // overwrite the most recent output symbol
          }
        }
        // set new sampling offset
        nin = j;
        outsig[nout] = 1; // indicate preamble
        d_state = STATE_SIGNAL;
        d_timeout = d_timeout_max;
        break;
      }
    }
  }
  consume_each(nin);
  return nout;
}
