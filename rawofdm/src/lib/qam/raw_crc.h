/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_CRC_H
#define INCLUDED_RAW_CRC_H

#include <gr_sync_block.h>
#include <gr_crc32.h>
#include <vector>

class raw_crc_enc;
typedef boost::shared_ptr<raw_crc_enc> raw_crc_enc_sptr;

raw_crc_enc_sptr
 raw_make_crc_enc (unsigned length);

/*!
 * \brief Appends CRC32
 *
 * input[0]: packets (bytes)
 * output[0]: packets with crc (bytes)
 */
class raw_crc_enc : public gr_sync_block
{
  friend raw_crc_enc_sptr raw_make_crc_enc (unsigned length);

  raw_crc_enc (unsigned length);

  unsigned d_length;

public:
  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};



class raw_crc_dec ;
typedef boost::shared_ptr<raw_crc_dec> raw_crc_dec_sptr;

raw_crc_dec_sptr
 raw_make_crc_dec (unsigned length);

/*!
 * \brief Verifies CRC32 and erases packets that fail
 *
 * input[0]: packets with crc (bytes)
 * input[1]: verified packets (bytes)
 */
class raw_crc_dec : public gr_block
{
  friend raw_crc_dec_sptr raw_make_crc_dec (unsigned length);

  raw_crc_dec (unsigned length);

  unsigned d_length;

public:
  int general_work(int noutput_items,
                   gr_vector_int &ninput_items,
                   gr_vector_const_void_star &input_items,
                   gr_vector_void_star &output_items);
};

#endif // INCLUDED_RAW_CRC_H
