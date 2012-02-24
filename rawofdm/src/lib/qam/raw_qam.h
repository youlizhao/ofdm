/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_QAM_H
#define INCLUDED_RAW_QAM_H

#include <gr_sync_decimator.h>
#include <gr_sync_interpolator.h>
#include <vector>

class raw_qam_enc;
typedef boost::shared_ptr<raw_qam_enc> raw_qam_enc_sptr;

raw_qam_enc_sptr
 raw_make_qam_enc (int nbits);

/*!
 * \brief Takes bits and converts them to amplitude
 * Uses Gray coding.
 *
 * nbits = number of bits per symbol (1,2,4,6)
 *
 * input[0]: bits
 * output[0]: modulated complex symbols (normalized to unit power)
 */
class raw_qam_enc : public gr_sync_decimator
{
  friend raw_qam_enc_sptr raw_make_qam_enc (int nbits);

  raw_qam_enc (int nbits);

  int d_nbits;

 public:
  int work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items);
};

class raw_qam_dec;
typedef boost::shared_ptr<raw_qam_dec> raw_qam_dec_sptr;

raw_qam_dec_sptr
 raw_make_qam_dec (int nbits);

/*!
 * \brief Takes modulated symbols and outputs soft values.
 * Uses Gray coding.
 *
 * nbits = number of bits per symbol (1,2,4,6)
 *
 * input[0]: modulated complex symbols (normalized to unit power)
 * output[0]: soft values (confidence, 0..255)
 */
class raw_qam_dec : public gr_sync_interpolator
{
  friend raw_qam_dec_sptr raw_make_qam_dec (int nbits);

  raw_qam_dec (int nbits);

  int d_nbits;

 public:
  int work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items);
};

#endif
