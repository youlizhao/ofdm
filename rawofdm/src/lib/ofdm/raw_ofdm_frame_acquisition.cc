/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_ofdm_frame_acquisition.h>
#include <gr_io_signature.h>
#include <gr_expj.h>
#include <gr_math.h>
#include <iostream>

#define VERBOSE 0
#define M_TWOPI (2*M_PI)
#define MAX_NUM_SYMBOLS 1000

raw_ofdm_frame_acquisition_sptr
raw_make_ofdm_frame_acquisition(unsigned int fft_length,
                                unsigned int cplen,
                                const std::vector<std::vector<gr_complex> > &preamble)
{
  return raw_ofdm_frame_acquisition_sptr (
    new raw_ofdm_frame_acquisition (fft_length, cplen, preamble)
  );
}

raw_ofdm_frame_acquisition::raw_ofdm_frame_acquisition (
          unsigned int fft_length,
          unsigned int cplen,
          const std::vector<std::vector<gr_complex> > &preamble)
  : gr_block ("ofdm_frame_acquisition",
        gr_make_io_signature2 (2, 2,
          sizeof(gr_complex)*fft_length,
          sizeof(char)),
        gr_make_io_signature2 (2, 2,
          sizeof(gr_complex)*preamble[0].size(),
          sizeof(char))),
    d_occupied_carriers(preamble[0].size()),
    d_fft_length(fft_length),
    d_cplen(cplen),
    d_preamble(preamble),
    d_known_norm(0),
    d_coarse_freq(0),
    d_cur_symbol(0),
    d_min_symbols(0),
    d_num_frames(0),
    d_cur_frame(0),
    d_signal_out(false)
{
  d_known_diff.resize(d_occupied_carriers-2);
  d_symbol_diff.resize(d_fft_length-2);
  d_hestimate.resize(d_occupied_carriers);

  const std::vector<gr_complex> &first = d_preamble[0];

  for(unsigned i = (pad() & 1); i < d_occupied_carriers-2; i+= 2) {
    // NOTE: only even frequencies matter
    // check pad to figure out which are odd
    d_known_diff[i] = first[i] * conj(first[i+2]);
    d_known_norm += norm(d_known_diff[i]);
  }
}

raw_ofdm_frame_acquisition::~raw_ofdm_frame_acquisition(void)
{
}

static const int LOOKAHEAD = 3;
void
raw_ofdm_frame_acquisition::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
  unsigned ninputs = ninput_items_required.size ();
  for (unsigned i = 0; i < ninputs; i++)
//    ninput_items_required[i] = 1;
    ninput_items_required[i] = LOOKAHEAD; // NOTE: so that our frame train method works
}

inline gr_complex
raw_ofdm_frame_acquisition::compensate() const {
  double carrier = (M_TWOPI * d_coarse_freq * d_cur_symbol * d_cplen) / d_fft_length;
  return gr_expj(-carrier);
}

inline int
raw_ofdm_frame_acquisition::pad() const {
  // amount of FFT padding on the left
  return (d_fft_length - d_occupied_carriers + 1)/2 + d_coarse_freq;
}

float
raw_ofdm_frame_acquisition::correlate(const gr_complex *symbol, int &coarse_freq)
{
  // Find a frequency offset that maximizes correlation of the input symbol
  // with the known symbol.
  // Due to possible phase offset we cannot take direct correlation.
  // If we assume that the phase offset is linear in frequency then we can
  // correlate the differential of the phase.
  // v = known symbol after FFT
  // x = input symbol after FFT
  // x[k] = v[k] exp(j (ak + b))
  // dv[k] = v[k] v[k+1]*
  // dx[k] = x[k] x[k+1]*
  // corr[k,d] = dv[k] dx[k+d]*
  // d_opt = argmax_d |sum_k corr[k,d]|
  // coarse_freq_offset = d_opt
  // time_offset = a = -mean(arg(corr[k,d]))
  //
  // Here v[2*k+1] = 0, so we change
  // dv[k] = v[k] v[k+2]*
  // dx[k] = x[k] x[k+2]*
  //

  for(unsigned i = 0; i < d_fft_length-2; i++) {
    d_symbol_diff[i] = conj(symbol[i]) * symbol[i+2];
  }

  unsigned int pad = (d_fft_length - d_occupied_carriers + 1)/2;

  // sweep through all possible/allowed frequency offsets and select the best
  double best_sum = 0.0;
  unsigned int best_d = 0;

  for(unsigned d = 0; d < 2*pad-1; ++d) {
    gr_complex sum = 0.0;
    for(unsigned j = pad & 1; j < d_occupied_carriers-2; j+= 2) {
      sum += (d_known_diff[j] * d_symbol_diff[d+j]);
    }
    //std::cerr << "d = " << d << "\tM = " << abs(sum) << std::endl;
    double asum = abs(sum);
    if(asum > best_sum) {
      best_sum = asum;
      best_d = d;
    }
  }
  coarse_freq = best_d - pad;
#if VERBOSE
  std::cerr << "coarse_freq = " << coarse_freq << std::endl;
#endif

  double norm_sum = 0.0;
  for(unsigned j = pad & 1; j < d_occupied_carriers-2; j+= 2) {
    norm_sum += norm(d_symbol_diff[best_d+j]);
  }
  float correlation = best_sum / sqrt(d_known_norm * norm_sum);

#if 0
  // nice little trick gives us timing offset (but we don't need it)
  double phase_sum = 0.0;
  for(unsigned j = 0; j < d_occupied_carriers-2; j+= 2) {
    // arg is sort of expensive, but it's the correct thing to do here
    float phase = std::arg(d_known_diff[j] * d_symbol_diff[best_d+j]);
    // FIXME: make sure it doesn't wrap (arg(mean) is the alternative)
    phase_sum += phase;
  }
  //std::cerr << "phase_sum " << phase_sum << std::endl;
  float time_offset = phase_sum / (d_occupied_carriers-2);
  std::cerr << "time_offset = " << time_offset
            << " = " << time_offset * d_fft_length / M_TWOPI
            << " samples" << std::endl;
#endif
  return correlation;
}

void
raw_ofdm_frame_acquisition::init_estimate(const gr_complex *symbol)
{
#if 0
  int pad = (d_fft_length - d_occupied_carriers)/2 + d_coarse_freq;

  // set every even tap based on known symbol
  for(int i = 0; i < d_occupied_carriers; i+=2) {
    gr_complex tx = d_known_symbol[i];
    gr_complex rx = symbol[i+pad];
    d_hestimate[i] = tx / rx;
  }

  // linearly interpolate between set carriers to set zero-filled carriers
  for(int i = 1; i < d_occupied_carriers - 1; i+= 2) {
    d_hestimate[i] = (d_hestimate[i-1] + d_hestimate[i+1]) / 2.0f;
  }

  // with even number of carriers; last equalizer tap is wrong
  if(!(d_occupied_carriers & 1)) {
    d_hestimate[d_occupied_carriers-1] = d_hestimate[d_occupied_carriers-2];
  }
#else // do not use the correlation symbol for channel estimation
  for(unsigned int i = 0; i < d_occupied_carriers; ++i) {
    d_hestimate[i] = 0;
  }
#endif
}

void
raw_ofdm_frame_acquisition::update_estimate(const gr_complex *symbol)
{
  int p = pad();
  // take coarse frequency offset into account
  gr_complex comp = compensate();

  const std::vector<gr_complex> &known_symbol = d_preamble[d_cur_symbol];

  // set every even tap based on known symbol
  for(unsigned int i = 0; i < d_occupied_carriers; ++i) {
    gr_complex tx = known_symbol[i];
    gr_complex rx = symbol[i+p];
    d_hestimate[i] += tx / (rx * comp);
  }
}

void
raw_ofdm_frame_acquisition::finish_estimate()
{
  // just normalize
  unsigned int num_symbols = d_preamble.size() - 1; // the first one does not count
  for(unsigned int i = 0; i < d_occupied_carriers; ++i) {
    d_hestimate[i] /= num_symbols;;
  }
#if VERBOSE
  float h_abs = 0.0f;
  float h_abs2 = 0.0f;
  float h_arg = 0.0f;
  float h_max = 0.0f;
  for(unsigned int i = 0; i < d_occupied_carriers; ++i) {
    if (i == (d_occupied_carriers+1)/2) {
      // skip the DC
      continue;
    }
    float aa = std::abs(d_hestimate[i]);
    h_abs += aa;
    h_abs2 += aa*aa;
    h_arg += std::arg(d_hestimate[i]); // FIXME: unwrap
    h_max = std::max(h_max, aa);
  }
  h_abs /= d_occupied_carriers-1;
  h_abs2 /= d_occupied_carriers-1;
  h_arg /= d_occupied_carriers-1;
  std::cerr << "H: phase = " << h_arg
            << "\tavg = " << h_abs
            << "\tmax = " << h_max
            << "\tSD = " << h_abs2 - h_abs*h_abs
            << std::endl;
#endif
}


int
raw_ofdm_frame_acquisition::general_work(
    int noutput_items,
    gr_vector_int &ninput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
  const gr_complex *symbol = (const gr_complex *)input_items[0];
  const char *signal_in = (const char *)input_items[1];

  gr_complex *out = (gr_complex *) output_items[0];
  char *signal_out = (char *) output_items[1];

  int ninput = ninput_items[0];
  int nproduced = 0;
  int nconsumed = 0;

  while((nconsumed < ninput) && (nproduced < noutput_items)) {
    // first, try to determine if new frame
    bool newframe = false;
    int coarse_freq;
    float corr;

    if (d_num_frames == 0) {
      // regular mode
      if(*signal_in) {
        corr = correlate(symbol, coarse_freq);
#if VERBOSE
        std::cerr << "correlation = " << corr << std::endl;
        std::cerr << "num symbols " << (d_cur_symbol+1) << std::endl;
#endif
        // TODO: choose proper correlation threshold
        newframe = (corr > 0.7f);
      }
    } else {
      // burst mode
      // for now we just want to get a solid burst of frames
      // we use high threshold (very low false positives)
      // or low threshold (very low false negatives)
      // we consider two indicators:
      //  is this frame part of burst?
      //  is this frame too short?

      if (d_cur_frame <= 5) {
        // looking for burst start
        if (*signal_in) {
          corr = correlate(symbol, coarse_freq);
          newframe = (corr > 0.7f);
#if 1
          if (newframe)
            std::cerr << "START corr = " << corr << std::endl;
#endif
        }
      } else {
        // accept packets while within burst
        if (d_cur_frame <= d_num_frames) {
          if (*signal_in) {
            corr = correlate(symbol, coarse_freq);
            newframe = (corr > 0.5f);
          } else {
            if (d_cur_symbol + 1 >= d_min_symbols + d_preamble.size()) {
              corr = correlate(symbol, coarse_freq);
              newframe = (corr > 0.6f);
            }
          }
        } else {
          // we're done -- don't accept new frames
        }
      }
    }

    if (newframe) {
      d_coarse_freq = coarse_freq;
      d_cur_symbol = 0;
      d_signal_out = true;
      init_estimate(symbol);
      ++d_cur_frame;

      ++nconsumed;
      ++signal_in;
      symbol+= d_fft_length;
      continue;
    } // else // ignore this blip

    ++d_cur_symbol;
    if (d_cur_symbol < d_preamble.size()) {
      // use for equalization
      update_estimate(symbol);

      ++nconsumed;
      ++signal_in;
      symbol+= d_fft_length;
      continue;
    }

    // time to produce

    if (d_cur_symbol == d_preamble.size()) {
      finish_estimate();
    }

    *signal_out = 0;
    if (d_signal_out) {
      *signal_out = 1;
      d_signal_out = false;
    }

    int p = pad();
    gr_complex comp = compensate();
    for(unsigned int i = 0; i < d_occupied_carriers; i++) {
      out[i] = d_hestimate[i] * comp * symbol[i+p];
    }

    out+= d_occupied_carriers;
    ++signal_out;
    ++nproduced;

    ++nconsumed;
    ++signal_in;
    symbol+= d_fft_length;
  }

  consume_each(nconsumed);
  return nproduced;
}

