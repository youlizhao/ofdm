/* -*- c++ -*- */
/*
 * Copyright 2004,2010,2011 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ftw_pnc_frequency_modulator_fc.h>
#include <gr_io_signature.h>
#include <gr_sincos.h>
#include <math.h>
#include <boost/math/special_functions/trunc.hpp>
#include <iostream>

static const pmt::pmt_t SYNC_CFO = pmt::pmt_string_to_symbol("sync_cfo");


ftw_pnc_frequency_modulator_fc_sptr ftw_make_pnc_frequency_modulator_fc (double sensitivity)
{
  return gnuradio::get_initial_sptr(new ftw_pnc_frequency_modulator_fc (sensitivity));
}

ftw_pnc_frequency_modulator_fc::ftw_pnc_frequency_modulator_fc (double sensitivity)
  : gr_sync_block ("pnc_frequency_modulator_fc",
		   gr_make_io_signature (1, 1, sizeof (gr_complex)),
		   gr_make_io_signature (1, 1, sizeof (gr_complex))),
    d_sensitivity (sensitivity), d_phase (0), d_value(0)
{
}

int
ftw_pnc_frequency_modulator_fc::work (int noutput_items,
				 gr_vector_const_void_star &input_items,
				 gr_vector_void_star &output_items)
{
  gr_complex *in = (gr_complex *) input_items[0];
  gr_complex *out = (gr_complex *) output_items[0];

      // test by lzyou
      // With a preamble, let's now check for the preamble sync timestamp
      std::vector<gr_tag_t> rx_sync_tags;
      int port = 0;
      int index= 0;
      const uint64_t nread = this->nitems_read(port);
      this->get_tags_in_range(rx_sync_tags, port, nread+index, nread+index+input_items.size(), SYNC_CFO);
      if(rx_sync_tags.size()>0) {
        size_t t = rx_sync_tags.size()-1;
        const pmt::pmt_t value = rx_sync_tags[t].value;
        double sync_cfo = pmt::pmt_to_double(value);
        if(false) {
          std::cout << "---- [FM] Range: ["<<nread+index<<":"<<nread+index+input_items.size()<<") \t index: " <<index<<"\n";
          std::cout << "---- [FM] CFO received, value = "<<sync_cfo<<"\n";
        }
      }

  for (int i = 0; i < noutput_items; i++){
    d_phase = d_phase + d_sensitivity * d_value;
    float oi, oq;
    gr_sincosf (d_phase, &oq, &oi);
    gr_complex t = gr_complex (oi, oq);
    out[i] = in[i] * t;
    //std::cout<<t<<" : "<<d_value<<std::endl;
    //std::cout<<i<<" : "<<noutput_items<<" : "<<d_phase<<" : "<<in[i]<<" : "<<out[i]<<std::endl;
  }

  // Limit the phase accumulator to [-16*pi,16*pi]
  // to avoid loss of precision in the addition above.

  if (fabs (d_phase) > 16 * M_PI){
    double ii = boost::math::trunc (d_phase / (2 * M_PI));
    d_phase = d_phase - (ii * 2 * M_PI);
  }

  return noutput_items;
}
