/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_PEAK_DETECTOR_FB_H
#define INCLUDED_RAW_PEAK_DETECTOR_FB_H

#include <gr_sync_block.h>

class raw_peak_detector_fb;
typedef boost::shared_ptr<raw_peak_detector_fb> raw_peak_detector_fb_sptr;

raw_peak_detector_fb_sptr
 raw_make_peak_detector_fb (float threshold_factor_rise = 0.25,
                            float threshold_factor_fall = 0.40,
                            int look_ahead = 10,
                            float alpha = 0.001);


class raw_peak_detector_fb : public gr_sync_block
{
  friend raw_peak_detector_fb_sptr
    raw_make_peak_detector_fb (float threshold_factor_rise,
                               float threshold_factor_fall,
                               int look_ahead, float alpha);

  raw_peak_detector_fb (float threshold_factor_rise,
                        float threshold_factor_fall,
                        int look_ahead, float alpha);

 private:
  float d_threshold_factor_rise;
  float d_threshold_factor_fall;
  int d_look_ahead;
  float d_avg_alpha;
  float d_avg;

 public:

  int work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items);
};

#endif
