/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_CONV_H
#define INCLUDED_RAW_CONV_H

#include <gr_sync_interpolator.h>
#include <gr_sync_decimator.h>
#include <gr_block.h>
#include <vector>

class raw_conv_enc;
typedef boost::shared_ptr<raw_conv_enc> raw_conv_enc_sptr;

raw_conv_enc_sptr
 raw_make_conv_enc ();

/*!
 * \brief Takes bytes and encodes them to bits
 *
 * For proper operation, the input must be padded with at least K-1 bits
 *
 * input[0]: bytes
 * output[0]: encoded bits
 */
class raw_conv_enc : public gr_sync_interpolator
{
  friend raw_conv_enc_sptr raw_make_conv_enc ();

  raw_conv_enc ();

  int d_sr; // stored shift-register

 public:
  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

class raw_conv_dec;
typedef boost::shared_ptr<raw_conv_dec> raw_conv_dec_sptr;

raw_conv_dec_sptr
 raw_make_conv_dec (int length);

struct v;
/*!
 * \brief Takes confidence level (0..255) and decodes bytes
 *
 * \param framelen == decoded frame length (in bits, _including_ padding)
 *  -- can be changed
 *
 * input[0]: bit confidence (0..255)
 * output[0]: decoded bytes
 */
class raw_conv_dec : public gr_sync_decimator
{
  friend raw_conv_dec_sptr raw_make_conv_dec (int length);

  raw_conv_dec (int length);

  int d_length; // source bits
  v* d_vp;

  int d_frameno;

 public:
  void set_length(int length);

  int work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items);
};


class raw_conv_punc;
typedef boost::shared_ptr<raw_conv_punc> raw_conv_punc_sptr;

raw_conv_punc_sptr
 raw_make_conv_punc (int nc, int np, unsigned char zero);

/*!
 * \brief Punctures/depunctures encoded bits
 *
 * \param nc number of bits consumed
 * \param np number of bits produced
 *
 * examples: raw_conv_punc(6, 4) converts 1/2-rate to 3/4-rate code
 *
 * input[0]: bits/confidences
 * output[0]: bits/confidences
 */
class raw_conv_punc : public gr_block
{
  friend raw_conv_punc_sptr raw_make_conv_punc (int nc, int np, unsigned char zero);

  raw_conv_punc (int nc, int np, unsigned char zero);

  int d_nc;
  int d_np;
  unsigned char d_zero;

 public:
  int fixed_rate_ninput_to_noutput(int ninput);
  int fixed_rate_noutput_to_ninput(int noutput);
  void forecast (int noutput_items, gr_vector_int &ninput_items_required);
  int general_work(int noutput_items,
                  gr_vector_int &ninput_items,
                  gr_vector_const_void_star &input_items,
                  gr_vector_void_star &output_items);
};

#endif // INCLUDED_RAW_CONV_H
