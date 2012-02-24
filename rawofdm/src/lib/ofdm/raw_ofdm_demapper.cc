/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_ofdm_demapper.h>
#include <gr_io_signature.h>
#include <gr_expj.h>
#include <gr_math.h>
#include <cmath>
#include <stdexcept>
#include <iostream>


raw_ofdm_demapper_sptr
raw_make_ofdm_demapper(std::vector<int> carrier_map,
                      float phase_gain,
                      float freq_gain,
                      float eq_gain)
{
  return raw_ofdm_demapper_sptr(
      new raw_ofdm_demapper(carrier_map, phase_gain, freq_gain, eq_gain)
   );
}


raw_ofdm_demapper::raw_ofdm_demapper( std::vector<int> carrier_map,
                                      float phase_gain,
                                      float freq_gain,
                                      float eq_gain)
  : gr_sync_block ("ofdm_demapper",
      gr_make_io_signature2 (2, 2,
        sizeof(gr_complex)*carrier_map.size(),
        sizeof(char)),
      gr_make_io_signature3 (1, 3,
        sizeof(gr_complex)*std::count(carrier_map.begin(), carrier_map.end(), 1),
        sizeof(float),
        sizeof(gr_complex)*carrier_map.size())),
    d_num_carriers(carrier_map.size()),
    d_phase_gain(phase_gain),d_freq_gain(freq_gain),d_eq_gain(eq_gain)
{

  for (unsigned int i = 0; i < d_num_carriers; ++i) {
    switch(carrier_map[i]) {
      case 0: break;
      case 1: d_data_carriers.push_back(i); break;
      case 2: d_pilot_carriers.push_back(i); break;
      default:
        throw std::invalid_argument("raw_ofdm_mapper: carrier_map must include only 0,1,2");
    }
  }

  d_dfe.resize(d_pilot_carriers.size());
  fill(d_dfe.begin(), d_dfe.end(), gr_complex(1.0,0.0));
}


inline void
raw_ofdm_demapper::reset()
{
  // reset PLL
  d_freq = 0.0;
  d_phase = 0.0;
  fill(d_dfe.begin(), d_dfe.end(), gr_complex(1.0,0.0));
}

/** process one symbol **/
void raw_ofdm_demapper::demap(const gr_complex *in, gr_complex *out,
                              float *noise_out, gr_complex *eq_out)
{
  gr_complex carrier = gr_expj(d_phase);
  gr_complex phase_error = 0.0;

  // update CFO PLL based on pilots
  // TODO: FIXME: 802.11a-1999 p.23 (25) defines p_{0..126v} which is cyclic
  int cur_pilot = 1;
  for (unsigned int i = 0; i < d_pilot_carriers.size(); ++i) {
    gr_complex pilot_sym(cur_pilot, 0.0);
    cur_pilot = -cur_pilot;
    int di = d_pilot_carriers[i];
    //gr_complex sigeq = in[di] * carrier * d_dfe[i];
    phase_error += in[di] * conj(pilot_sym);
  }

  // update phase equalizer
  float angle = arg(phase_error);
  d_freq = d_freq - d_freq_gain*angle;
  d_phase = d_phase + d_freq - d_phase_gain*angle;
  if (d_phase >= 2*M_PI) d_phase -= 2*M_PI;
  else if (d_phase <0) d_phase += 2*M_PI;

  carrier = gr_expj(-angle);

#if 0
  // TODO: add residual SFO tracking
  // simple linear regression of phase error
  double sumy = 0.0;
  double sumyn = 0.0;
  float phase_prev = 0.0f;
  float phase_unwrap = 0.0f; // unwrapped
  for(unsigned j = 0; j < d_occupied_carriers; j+= 2) {
    float phase_err = std::arg(d_known_symbol[j] * conj(symbol[best_d+j]));
    // NOTE, fmod preserve sign, so force it to be positive first
    float phase_diff = fmod(phase_err - phase_prev + 3*M_PI, 2*M_PI) - M_PI;
    phase_unwrap+= phase_diff;
    //std::cerr << phase_err << "  \t" << phase_diff << "  \t" << phase_unwrap << std::endl;
    sumy+= phase_unwrap;
    sumyn+= phase_unwrap * (j/2 + 1); // sum(y*x)
    phase_prev = phase_err;
  }
  int n = d_occupied_carriers/2;
  int sumn = n*(n+1)/2;
  int sumn2 = sumn*(2*n+1)/3; // = n*(n+1)*(2n+1)/6
  //std::cerr << "sy " << sumy << "\tsxy " << sumyn
            //<< "\tsx " << sumn << "\tsx2 " << sumn2
            //<< std::endl;
  d_time_offset = - (n * sumyn - sumn * sumy) / (n * sumn2 - sumn * sumn) / 2;
#endif

  // update DFE based on pilots
  float noise = 0.0f;
  cur_pilot = 1;
  for (unsigned int i = 0; i < d_pilot_carriers.size(); ++i) {
    gr_complex pilot_sym(cur_pilot, 0.0);
    cur_pilot = -cur_pilot;
    int di = d_pilot_carriers[i];
    gr_complex sigeq = in[di] * carrier * d_dfe[i];
    noise += norm(sigeq - pilot_sym);
    // FIX THE FOLLOWING STATEMENT
    if (norm(sigeq)> 0.001)
      d_dfe[i] += d_eq_gain * (pilot_sym/sigeq - d_dfe[i]);

    if (eq_out) eq_out[di] = d_dfe[i];
  }

  if (noise_out)
    *noise_out = noise / d_pilot_carriers.size();

  // TODO: spline interpolation?
  // requires solving NxN where N = # pilots

  // equalize all data using interpolated dfe and demap into bytes
  unsigned int pilot_index = 0;
  int pilot_carrier_lo = 0;
  int pilot_carrier_hi = d_pilot_carriers[0];
  gr_complex pilot_dfe_lo = d_dfe[0];
  gr_complex pilot_dfe_hi = d_dfe[0];

  float denom = 1.0 / (pilot_carrier_hi - pilot_carrier_lo);
  // TODO: create a map?
  // carrier index -> (low index, low weight, high index, high weight)
  for (unsigned int i = 0; i < d_data_carriers.size(); ++i) {
    int di = d_data_carriers[i];
    if (di > pilot_carrier_hi) {
      ++pilot_index;
      pilot_carrier_lo = pilot_carrier_hi;
      pilot_dfe_lo = pilot_dfe_hi;
      if (pilot_index < d_pilot_carriers.size()) {
        pilot_carrier_hi = d_pilot_carriers[pilot_index];
        pilot_dfe_hi = d_dfe[pilot_index];
      } else {
        pilot_carrier_hi = d_num_carriers;
      }
      denom = 1.0 / (pilot_carrier_hi - pilot_carrier_lo);
    }
    // FIXME?
    // smarter interpolation would be linear in polar coords (slerp!)
    float alpha = float(pilot_carrier_hi - di) * denom;
    //gr_complex dfe = alpha * pilot_dfe_lo + (1-alpha) * pilot_dfe_hi;
    gr_complex dfe = pilot_dfe_hi + alpha * (pilot_dfe_lo - pilot_dfe_hi);
    gr_complex sigeq = in[di] * carrier * dfe;

    if (eq_out) eq_out[di] = dfe; // output the equalizer value
    out[i] = sigeq;
  }
}

int
raw_ofdm_demapper::work (int noutput_items,
                          gr_vector_const_void_star &input_items,
                          gr_vector_void_star &output_items)
{
  const gr_complex *in = (const gr_complex *) input_items[0];
  const char *sig = (const char *) input_items[1]; // signal found

  gr_complex *out = (gr_complex *) output_items[0];
  float *noise_out = (output_items.size() > 1) ? (float *)output_items[1] : NULL;
  gr_complex *eq_out = (output_items.size() > 2) ? (gr_complex *)output_items[2] : NULL;

  for (int i = 0; i < noutput_items; ++i) {
    if (sig[0]) {
      reset();
    }
    // NOTE: make sure that this matches the behavior of frame_acquisition
    // only demod after getting the preamble signal
    demap(in, out, noise_out, eq_out);
    in += d_num_carriers;
    sig += 1;
    out += d_data_carriers.size();
    if (noise_out) {
      ++noise_out;
      if (eq_out)
        eq_out += d_num_carriers;
    }
  }
  return noutput_items;
}

