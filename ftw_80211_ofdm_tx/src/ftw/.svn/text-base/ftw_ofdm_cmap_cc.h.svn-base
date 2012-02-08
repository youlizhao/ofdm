/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef INCLUDED_FTW_CMAP_CC_H
#define INCLUDED_FTW_CMAP_CC_H

#include <gr_block.h>

class ftw_ofdm_cmap_cc;

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr_blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
typedef boost::shared_ptr<ftw_ofdm_cmap_cc> ftw_ofdm_cmap_cc_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of ftw_ofdm_cmap_cc.
 *
 * To avoid accidental use of raw pointers, ftw_ofdm_cmap_cc's
 * constructor is private.  ftw_make_ofdm_cmap_cc is the public
 * interface for creating new instances.
 */
ftw_ofdm_cmap_cc_sptr ftw_make_ofdm_cmap_cc (int fft_size, int tones);

/*!
 * \brief square a stream of floats.
 * \ingroup block
 *
 * \sa ftw_square2_ff for a version that subclasses gr_sync_block.
 */
class ftw_ofdm_cmap_cc : public gr_block
{
private:
  int d_fft_size;
  int d_tones;
  // The friend declaration allows ftw_make_ofdm_cmap_cc to
  // access the private constructor.

  friend ftw_ofdm_cmap_cc_sptr ftw_make_ofdm_cmap_cc (int fft_size, int tones);

  ftw_ofdm_cmap_cc (int fft_size, int tones);  	// private constructor

public:
  ~ftw_ofdm_cmap_cc ();	// public destructor

  // Where all the action really happens

  int general_work (int noutput_items,
		    gr_vector_int &ninput_items,
		    gr_vector_const_void_star &input_items,
		    gr_vector_void_star &output_items);
};

#endif /* INCLUDED_FTW_CMAP_CC_H */
