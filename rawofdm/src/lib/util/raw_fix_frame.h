/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_FIX_FRAME_H
#define INCLUDED_RAW_FIX_FRAME_H

#include <gr_block.h>

class raw_fix_frame;
typedef boost::shared_ptr<raw_fix_frame> raw_fix_frame_sptr;

raw_fix_frame_sptr raw_make_fix_frame (size_t itemsize, size_t numitems);

/*!
 * \brief Aligns received frames to fixed size.
 *   inputs:
 *      data (itemsize-wide)
 *      framing (+1 to start new message)
 *   outputs:
 *      aligned data (itemsize-wide)
 *   parameters:
 *      itemsize
 *      numitmes
 *
 */
class raw_fix_frame : public gr_block
{
 private:
  void forecast(int noutput_items, gr_vector_int &ninput_items_required);
  // params
  size_t    d_itemsize;
  size_t    d_numitems;
  // dynamic state
  size_t    d_curitem;

  friend raw_fix_frame_sptr
  raw_make_fix_frame(size_t itemsize, size_t numitems);

 protected:
  raw_fix_frame (size_t itemsize, size_t numitems);

 public:
  ~raw_fix_frame ();

  int general_work(int noutput_items,
                  gr_vector_int &ninput_items,
                  gr_vector_const_void_star &input_items,
                  gr_vector_void_star &output_items);
};

#endif /* INCLUDED_RAW_FIX_FRAME_H */
