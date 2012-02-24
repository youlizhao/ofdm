/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_fix_frame.h>
#include <gr_io_signature.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


raw_fix_frame_sptr
raw_make_fix_frame (size_t itemsize, size_t numitems)
{
  return raw_fix_frame_sptr(new raw_fix_frame(itemsize, numitems));
}

raw_fix_frame::raw_fix_frame (size_t itemsize, size_t numitems)
  : gr_block("fix_frame",
      gr_make_io_signature2(1, 2, itemsize, sizeof(char)),
      gr_make_io_signature2(1, 2, itemsize, sizeof(char))),
    d_itemsize(itemsize), d_numitems(numitems), d_curitem(d_numitems)
{
}

raw_fix_frame::~raw_fix_frame() {}

void
raw_fix_frame::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
  unsigned ninputs = ninput_items_required.size ();
  for (unsigned i = 0; i < ninputs; i++)
    ninput_items_required[i] = noutput_items; //not entirely accurate
}

int
raw_fix_frame::general_work(
    int noutput_items,
    gr_vector_int &ninput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
  const char *in = (const char *) input_items[0];
  const char *signal_in = (const char *)input_items[1];
  char *out = (char *) output_items[0];
  char *signal_out = (output_items.size() > 1)
                        ? (char *) output_items[1]
                        : NULL;

  if (signal_out)
    signal_out[0] = 0;

  // FIXME: process more than one item at a time
  if (signal_in[0]) {
    if (d_curitem < d_numitems) {
      // pad up current frame
      memset(out, 0, d_itemsize * sizeof(char));
      ++d_curitem;
      // don't consume
      return 1;
    }
    d_curitem = 0;
    if (signal_out)
      signal_out[0] = 1;
  }
  consume_each(1);
  if (d_curitem < d_numitems) {
    memcpy(out, in, d_itemsize * sizeof(char));
    ++d_curitem;
    return 1;
  }

  return 0;
}

