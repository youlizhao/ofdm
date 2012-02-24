/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_SCRAMBLER_BB_H
#define INCLUDED_RAW_SCRAMBLER_BB_H

#include <gr_block.h>

class raw_scrambler_bb;
typedef boost::shared_ptr<raw_scrambler_bb> raw_scrambler_bb_sptr;

raw_scrambler_bb_sptr
 raw_make_scrambler_bb (unsigned length=0, unsigned seed=0);


/**
 * scramble byte by byte
 * \see gri_glfsr
 *
 * x[-7] + x[-4] + 1
 *
 * parameters:
 *  length -- how long does each value on the seed input last
 *
 * inputs:
 *   bytes to be scrambled
 *   seed (when !=0, only bottom 7 bits matter) [int-sized]
 * outputs:
 *   scrambled bytes
 */
class raw_scrambler_bb : public gr_block
{
 private:
  unsigned d_length; // resets d_length each time
  unsigned d_reg;   // current state
  unsigned d_count; // how many to process before new seed should be read

  friend raw_scrambler_bb_sptr
    raw_make_scrambler_bb (unsigned length, unsigned seed);

  raw_scrambler_bb (unsigned length, unsigned seed);

  void forecast(int noutput_items, gr_vector_int &ninput_items_required);

 public:
  int general_work(int noutput_items,
                   gr_vector_int &ninput_items,
                   gr_vector_const_void_star &input_items,
                   gr_vector_void_star &output_items);
};

#endif
