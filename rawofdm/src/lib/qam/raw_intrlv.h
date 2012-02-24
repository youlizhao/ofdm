/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_INTRLV_H
#define INCLUDED_RAW_INTRLV_H

#include <gr_sync_block.h>
#include <vector>

class raw_intrlv_bit;
typedef boost::shared_ptr<raw_intrlv_bit> raw_intrlv_bit_sptr;

raw_intrlv_bit_sptr
 raw_make_intrlv_bit (int ncarriers, int nbits, bool inverse);

/*!
 * \brief Interleaves (coded) bits over OFDM QAM symbols.
 * See 802.11a/g
 *
 * \param ncarriers  number of data carriers
 * \param nbits      number of (coded) bits per carrier
 * \param inverse    true if deinterleaving
 *
 * input[0]: coded bits
 * input[1]: interleaved bits ready for QAM
 *
 * if(inverse):
 *      input[0]: coded confidences from QAM
 *      input[1]: deinterleaved confidences ready for conv_dec
 */
class raw_intrlv_bit : public gr_sync_block
{
  friend raw_intrlv_bit_sptr raw_make_intrlv_bit (int ncarriers, int nbits, bool inverse);

  raw_intrlv_bit (int ncarriers, int nbits, bool inverse);

  std::vector<unsigned> d_map;

public:
  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};


class raw_intrlv_byte;
typedef boost::shared_ptr<raw_intrlv_byte> raw_intrlv_byte_sptr;

raw_intrlv_byte_sptr
 raw_make_intrlv_byte (int nrows, int slope, bool inverse);

/*!
 * \brief Convolutional interleaver
 * Interleaves bytes in RS-encoded packets.
 * See DVB-T.
 *
 * \param nrows (= 12) number of FIFOs
 * \param slope (= 17) difference in length between subsequent FIFOs
 * \param inverse    true if deinterleaving
 *
 * NOTE: packet_len = nrows * slope
 * NOTE: delay = (nrows-1) * packet_len
 * NOTE: continuous operation only (no flush-out!)
 *
 * input[0]: RS-encoded packets (packet-sized)
 * input[1]: interleaved bytes ready for conv_enc
 *
 * if(inverse):
 *      input[0]: viterbi-decoded bytes
 *      input[1]: deinterleaved bytes ready for RS-decoder (packet-sized)
 */
class raw_intrlv_byte : public gr_sync_block
{
  friend raw_intrlv_byte_sptr raw_make_intrlv_byte (int nrows, int slope, bool inverse);

  raw_intrlv_byte (int nrows, int slope, bool inverse);

  struct FIFO {
    int n; // length
    int p; // current index
    unsigned char buf[256];
    unsigned char push(unsigned char c) {
      if (n) { // horrible for branch prediction
        unsigned char t = buf[p];
        buf[p] = c;
        p = (p+1) % n;
        return t;
      } else return c;
    }
  };

  int d_length;
  int d_head; // set to delay at the beginning, used to flush out the delay
  std::vector<FIFO> d_fifos;

public:
  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

#endif
