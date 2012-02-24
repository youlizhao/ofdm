/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_RS_H
#define INCLUDED_RAW_RS_H

#include <gr_sync_block.h>
#include <vector>

#include <ecc.h>

class raw_rs_enc;
typedef boost::shared_ptr<raw_rs_enc> raw_rs_enc_sptr;

raw_rs_enc_sptr
 raw_make_rs_enc ();

/*!
 * \brief Encodes (shortened) RS 188/204
 *
 * input[0]: packets (bytes)
 * output[0]: coded packets (bytes)
 */
class raw_rs_enc : public gr_sync_block
{
  friend raw_rs_enc_sptr raw_make_rs_enc ();

  raw_rs_enc ();

  RS::Encoder d_enc;

  unsigned char msg[256];
  unsigned char code[256];

public:
  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};



class raw_rs_dec ;
typedef boost::shared_ptr<raw_rs_dec> raw_rs_dec_sptr;

raw_rs_dec_sptr
 raw_make_rs_dec ();

/*!
 * \brief Decodes (shortened) RS 188/204
 *
 * input[0]: coded packets (bytes)
 * output[0]: decoded packets (bytes)
 */
class raw_rs_dec : public gr_sync_block
{
  friend raw_rs_dec_sptr raw_make_rs_dec ();

  raw_rs_dec ();

  RS::Decoder d_dec;

  unsigned char msg[256];
  unsigned char code[256];

public:
  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

#endif // INCLUDED_RAW_RS_H
