/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_ofdm_mapper.h>
#include <gr_io_signature.h>
#include <stdexcept>
#include <iostream>
#include <string.h>

raw_ofdm_mapper_sptr
raw_make_ofdm_mapper(std::vector<int> carrier_map)
{
  return raw_ofdm_mapper_sptr (new raw_ofdm_mapper (carrier_map));
}

raw_ofdm_mapper::raw_ofdm_mapper (std::vector<int> carrier_map)
  : gr_sync_block ("ofdm_mapper",
      gr_make_io_signature2 (1, 2,
        sizeof(gr_complex)*std::count(carrier_map.begin(), carrier_map.end(), 1),
        sizeof(char)),
      gr_make_io_signature (1, 1,
        sizeof(gr_complex)*carrier_map.size())
    ),
    d_fft_length(carrier_map.size())
{
  for (unsigned int i = 0; i < d_fft_length; ++i) {
    switch(carrier_map[i]) {
      case 0: break;
      case 1: d_data_carriers.push_back(i); break;
      case 2: d_pilot_carriers.push_back(i); break;
      default:
        throw std::invalid_argument("raw_ofdm_mapper: carrier_map must include only 0,1,2");
    }
  }
}


int
raw_ofdm_mapper::work(int noutput_items,
                      gr_vector_const_void_star &input_items,
                      gr_vector_void_star &output_items)
{
  gr_complex *in = (gr_complex *)input_items[0];
  char *enable = (input_items.size() > 1) ? (char *)input_items[1] : NULL;
  gr_complex *out = (gr_complex *)output_items[0];

  for (int j = 0; j < noutput_items; ++j) {
    if ((enable == NULL) || enable[j]) { // is it even neessary?
      // Build a single symbol:
      // Initialize all bins to 0 to set unused carriers
      memset(out, 0, d_fft_length*sizeof(gr_complex));

      // TODO: FIXME: 802.11a-1999 p.23 (25) defines p_{0..126v} which is cyclic
      // NOTE: but there are at most 20 symbols in our frames, so at most 80 in the seq are used
      // Fill in pilots
      int cur_pilot = 1; // TODO: allow custom pilots?
      for (unsigned int i = 0; i < d_pilot_carriers.size(); ++i) {
        out[d_pilot_carriers[i]] = gr_complex(cur_pilot, 0.0);
        cur_pilot = -cur_pilot;
      }

      // Fill in data
      for (unsigned int i = 0; i < d_data_carriers.size(); ++i) {
        out[d_data_carriers[i]] = in[i];
      }
    }
//    for (int i = 0; i < d_fft_length; ++i)
//      std::cout << "out " << i << " = " << out[i] << std::endl;

    in+= d_data_carriers.size();
    out+= d_fft_length;
  }
  return noutput_items;
}

