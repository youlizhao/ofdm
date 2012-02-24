/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_peak_detector_fb.h>
#include <gr_io_signature.h>
#include <string.h>
#include <stdio.h>

raw_peak_detector_fb_sptr
raw_make_peak_detector_fb (float threshold_factor_rise,
                    float threshold_factor_fall,
                    int look_ahead, float alpha)
{
  return raw_peak_detector_fb_sptr (new raw_peak_detector_fb (threshold_factor_rise,
                                  threshold_factor_fall,
                                  look_ahead, alpha));
}

raw_peak_detector_fb::raw_peak_detector_fb (float threshold_factor_rise,
                float threshold_factor_fall,
                int look_ahead, float alpha)
  : gr_sync_block ("peak_detector_fb",
                  gr_make_io_signature (1, 1, sizeof (float)),
                  gr_make_io_signature (1, 1, sizeof (char))),
    d_threshold_factor_rise(threshold_factor_rise),
    d_threshold_factor_fall(threshold_factor_fall),
    d_look_ahead(look_ahead), d_avg_alpha(alpha), d_avg(0)
{
}

int
raw_peak_detector_fb::work (int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items)
{
  float *iptr = (float *) input_items[0];
  char *optr = (char *) output_items[0];

  memset(optr, 0, noutput_items*sizeof(char));

  float peak_val = -(float)INFINITY; // szym: FIX
  int peak_ind = 0;
  unsigned char state = 0; // szym: FIX
  int i = 0;

  d_avg = -1.0f;

  while(i < noutput_items) {
    if(state == 0) {  // below threshold
      if(iptr[i] > d_avg*d_threshold_factor_rise) {
        state = 1;
      } else {
        i++;
      }
    } else if(state == 1) {  // above threshold, have not found peak
      if(iptr[i] > peak_val) {
        peak_val = iptr[i];
        peak_ind = i;
        i++;
      } else if (iptr[i] > d_avg*d_threshold_factor_fall) {
        i++;
      } else {
        optr[peak_ind] = 1;
        state = 0;
        peak_val = -(float)INFINITY;
      }
    }
  }

  if(state == 0) {
    return noutput_items;
  } else {   // only return up to passing the threshold
    return peak_ind; // NOTE: peak itself must remain unconsumed
  }
}
