/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_ofdm_demapper_H
#define INCLUDED_RAW_ofdm_demapper_H

#include <gr_sync_block.h>
#include <gr_msg_queue.h>

class raw_ofdm_demapper;
typedef boost::shared_ptr<raw_ofdm_demapper> raw_ofdm_demapper_sptr;

raw_ofdm_demapper_sptr
raw_make_ofdm_demapper (std::vector<int> carrier_map,
                        float phase_gain=0.25,
                        float freq_gain=0.25*0.25/4.0,
                        float eq_gain=0.05);

/*!
 * \brief Takes an OFDM symbol in, demaps it into raw samples.
 * Uses pilots to track phase, corrects residual carrier frequency offset and
 * residual timing offset, equalizes.
 *
 * \ingroup sink_blk
 * \ingroup ofdm_blk
 *
 * input[0]: FFT signal (only occupied carriers)
 * input[1]: new frame indicator (to reset PLL)
 *
 * output[0]: demapped data
 * output[1] (optional): average symbol noise power estimate
 * output[2] (optional): current dfe for each bin
 */
class raw_ofdm_demapper : public gr_sync_block
{
  friend raw_ofdm_demapper_sptr
    raw_make_ofdm_demapper (std::vector<int> carrier_map,
                            float phase_gain,
                            float freq_gain,
                            float eq_gain);

private:
  // fixed params
  unsigned int     d_num_carriers;
  std::vector<int> d_data_carriers;
  std::vector<int> d_pilot_carriers;

  float            d_phase_gain;
  float            d_freq_gain;
  float            d_eq_gain;

  // dynamic state
  std::vector<gr_complex>  d_dfe; // one per pilot
  float            d_phase; // one for all
  float            d_freq; // one for all

 protected:
  raw_ofdm_demapper(std::vector<int> carrier_map,
                    float phase_gain,
                    float freq_gain,
                    float dfe_gain);

  void demap(const gr_complex *in, gr_complex *out, float *noise_out, gr_complex *eq_out);

  void reset();
 public:

  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

#endif /* INCLUDED_RAW_ofdm_demapper_H */
