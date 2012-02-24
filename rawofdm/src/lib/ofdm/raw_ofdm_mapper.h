/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_raw_ofdm_mapper_H
#define INCLUDED_raw_ofdm_mapper_H

#include <gr_sync_block.h>
#include <gr_message.h>
#include <gr_msg_queue.h>

class raw_ofdm_mapper;
typedef boost::shared_ptr<raw_ofdm_mapper> raw_ofdm_mapper_sptr;

raw_ofdm_mapper_sptr
raw_make_ofdm_mapper (std::vector<int> carrier_map);

/*!
 * \brief take a stream of samples in and map to a vector of complex
 * constellation points suitable for IFFT input to be used in an ofdm
 * modulator.
 * This module only inserts pilots and empty samples for mute subcarriers.
 *   inputs:
 *      complex data
 *      enable flag [optional]
 *   outputs:
 *      data to be sent to IFFT
 *   parameters:
 *      carrier_map
 *      -- length implies fft_length
 *      -- value == 0, carrier empty
 *      -- value == 1, data carrier
 *      -- value == 2, pilot carrier ((-1,0) or (1,0))
 * \ingroup modulation_blk
 * \ingroup ofdm_blk
 */

class raw_ofdm_mapper : public gr_sync_block
{
  friend raw_ofdm_mapper_sptr
    raw_make_ofdm_mapper(std::vector<int> carrier_map);
 protected:
  raw_ofdm_mapper (std::vector<int> carrier_map);

 private:
  // fixed params
  unsigned int          d_fft_length;
  std::vector<int>      d_data_carriers;
  std::vector<int>      d_pilot_carriers;

 public:

  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

#endif
