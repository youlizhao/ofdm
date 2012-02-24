/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_SYMBOL_AVG_H
#define INCLUDED_RAW_SYMBOL_AVG_H

#include <gr_sync_decimator.h>

class raw_symbol_avg;
typedef boost::shared_ptr<raw_symbol_avg> raw_symbol_avg_sptr;

raw_symbol_avg_sptr raw_make_symbol_avg (size_t vlen, size_t numv);

/*!
 * \brief Computes average of values within a symbol (vector), fixed number of symbols
 * NOTE: this is like gr_integrate_XX, but integrate does not support vectors.
 *   inputs:
 *      float data (vlen-wide)
 *   outputs:
 *      average
 *   parameters:
 *      vector length
 *      number of vectors to average over
 *
 */
class raw_symbol_avg : public gr_sync_decimator
{
 private:
  // params
  size_t    d_vlen;
  size_t    d_numv;

  friend raw_symbol_avg_sptr
  raw_make_symbol_avg(size_t vlen, size_t numv);

 protected:
  raw_symbol_avg (size_t vlen, size_t numv);

 public:
  ~raw_symbol_avg ();

  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

#endif /* INCLUDED_RAW_SYMBOL_AVG_H */
