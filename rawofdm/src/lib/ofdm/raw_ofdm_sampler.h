/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_OFDM_SAMPLER_H
#define INCLUDED_RAW_OFDM_SAMPLER_H

#include <gr_sync_block.h>

class raw_ofdm_sampler;
typedef boost::shared_ptr<raw_ofdm_sampler> raw_ofdm_sampler_sptr;

raw_ofdm_sampler_sptr raw_make_ofdm_sampler(unsigned int fft_length,
                                            unsigned int symbol_length,
                                            unsigned int timeout=1000);

/*!
 * \brief samples the signal at symbol boundaries
 * \ingroup ofdm_blk
 * Inputs:
 *  [0] signal to be sampled
 *  [1] indicator of where the preamble symbol starts (first sample)
 * Outputs:
 *  [0] sampled symbols (fft_length long)
 *  [1] indicator of which symbol is the preamble (1 per symbol)
 */
class raw_ofdm_sampler : public gr_block
{
  friend raw_ofdm_sampler_sptr raw_make_ofdm_sampler (
                unsigned int fft_length,
                unsigned int symbol_length,
                unsigned int timeout);

  raw_ofdm_sampler (unsigned int fft_length,
                    unsigned int symbol_length,
                    unsigned int timeout);

 private:
  // params
  unsigned int d_timeout_max;
  unsigned int d_fft_length;
  unsigned int d_symbol_length;

  // dynamic state
  enum state_t {STATE_NO_SIG, STATE_SIGNAL};
  state_t d_state;
  unsigned int d_timeout; // remaining count

 public:
  void forecast (int noutput_items, gr_vector_int &ninput_items_required);

  int general_work (int noutput_items,
                    gr_vector_int &ninput_items,
                    gr_vector_const_void_star &input_items,
                    gr_vector_void_star &output_items);
};

#endif
