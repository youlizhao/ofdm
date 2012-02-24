/* -*- c++ -*- */

%include "gnuradio.i"                   // the common stuff

%{
#include "raw_ofdm_mapper.h"
#include "raw_ofdm_demapper.h"
#include "raw_ofdm_frame_acquisition.h"
#include "raw_ofdm_sampler.h"
%}

%include "std_vector.i"

namespace std {
  %template(IntVector) vector<int>;
}

GR_SWIG_BLOCK_MAGIC(raw,ofdm_mapper);
raw_ofdm_mapper_sptr raw_make_ofdm_mapper (std::vector<int> carrier_map);
class raw_ofdm_mapper : public gr_sync_block {};

GR_SWIG_BLOCK_MAGIC(raw,ofdm_demapper);
raw_ofdm_demapper_sptr
raw_make_ofdm_demapper (std::vector<int> carrier_map,
                        float phase_gain=0.25,
                        float freq_gain=0.25*0.25/4.0,
                        float eq_gain=0.05);
class raw_ofdm_demapper : public gr_sync_block {};

GR_SWIG_BLOCK_MAGIC(raw,ofdm_frame_acquisition);
raw_ofdm_frame_acquisition_sptr
raw_make_ofdm_frame_acquisition(unsigned int fft_length,
                                unsigned int cplen,
                                const std::vector<std::vector<gr_complex> > &preamble);
class raw_ofdm_frame_acquisition : public gr_block
{
 public:
  void set_min_symbols(int val);
  void set_num_frames(int val);
};

GR_SWIG_BLOCK_MAGIC(raw,ofdm_sampler)
raw_ofdm_sampler_sptr raw_make_ofdm_sampler (unsigned int fft_length,
               unsigned int symbol_length,
               unsigned int timeout=1000);
class raw_ofdm_sampler : public gr_sync_block {};

