/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_BER_AVG_H
#define INCLUDED_RAW_BER_AVG_H

#include <gr_sync_decimator.h>

class raw_ber_avg;
typedef boost::shared_ptr<raw_ber_avg> raw_ber_avg_sptr;

raw_ber_avg_sptr raw_make_ber_avg (size_t vlen, size_t numv);

/*!
 * \brief Computes average BER (vector) over fixed number of observations
 * Just like raw_symbol_avg but computes bits flipped in byte as well
 *   inputs:
 *      byte data (vlen-wide) char
 *   outputs:
 *      average BER (vlen-wide) float
 *   parameters:
 *      vector length
 *      number of vectors to average over (set to 1 for no decimation)
 *
 */
class raw_ber_avg : public gr_sync_decimator
{
 private:
  // params
  size_t    d_vlen;
  size_t    d_numv;

  friend raw_ber_avg_sptr
  raw_make_ber_avg(size_t vlen, size_t numv);

 protected:
  raw_ber_avg (size_t vlen, size_t numv);

 public:

  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

#endif /* INCLUDED_RAW_BER_AVG_H */
