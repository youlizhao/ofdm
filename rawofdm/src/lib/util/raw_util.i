/* -*- c++ -*- */

%include "gnuradio.i"                   // the common stuff

%{
#include "raw_fix_frame.h"
#include "raw_symbol_avg.h"
#include "raw_peak_detector_fb.h"
#include "raw_ber.h"
%}

GR_SWIG_BLOCK_MAGIC(raw,fix_frame)
raw_fix_frame_sptr raw_make_fix_frame (size_t itemsize, size_t numitems);
class raw_fix_frame : public gr_block {};

GR_SWIG_BLOCK_MAGIC(raw,symbol_avg)
raw_symbol_avg_sptr raw_make_symbol_avg (size_t vlen, size_t numv);
class raw_symbol_avg : public gr_sync_decimator {};

GR_SWIG_BLOCK_MAGIC(raw,peak_detector_fb)
raw_peak_detector_fb_sptr raw_make_peak_detector_fb (float threshold_factor_rise = 0.25,
                                 float threshold_factor_fall = 0.40,
                                 int look_ahead = 10,
                                 float alpha=0.001);
class raw_peak_detector_fb : public gr_sync_block {};

GR_SWIG_BLOCK_MAGIC(raw,ber_avg)
raw_ber_avg_sptr raw_make_ber_avg (size_t vlen, size_t numv);
class raw_ber_avg : public gr_sync_decimator {};

