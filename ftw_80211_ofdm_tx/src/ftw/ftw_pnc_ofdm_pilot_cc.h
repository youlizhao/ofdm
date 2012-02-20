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
#ifndef INCLUDED_FTW_PNC_PILOT_CC_H
#define INCLUDED_FTW_PNC_PILOT_CC_H

#include <gr_block.h>

class ftw_pnc_ofdm_pilot_cc;

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
typedef boost::shared_ptr<ftw_pnc_ofdm_pilot_cc> ftw_pnc_ofdm_pilot_cc_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of ftw_pnc_ofdm_pilot_cc.
 *
 * To avoid accidental use of raw pointers, ftw_pnc_ofdm_pilot_cc's
 * constructor is private.  ftw_make_ofdm_pilot_cc is the public
 * interface for creating new instances.
 */
ftw_pnc_ofdm_pilot_cc_sptr ftw_make_pnc_ofdm_pilot_cc (int tones, int user);          //////////////////////modified by taotao



class ftw_pnc_ofdm_pilot_cc : public gr_block
{
private:
  int d_tones;
  int d_user;                                                                 //////////////////////modified by taotao
  int i;
  int offset;
  // The friend declaration allows ftw_make_ofdm_pilot_cc to
  // access the private constructor.

  friend ftw_pnc_ofdm_pilot_cc_sptr ftw_make_pnc_ofdm_pilot_cc (int tones, int user); //////////////////////modified by taotao  

  ftw_pnc_ofdm_pilot_cc (int tones, int user);  	// private constructor        //////////////////////modified by taotao

public:
  ~ftw_pnc_ofdm_pilot_cc ();	// public destructor

  // Where all the action really happens

  int general_work (int noutput_items,
		    gr_vector_int &ninput_items,
		    gr_vector_const_void_star &input_items,
		    gr_vector_void_star &output_items);
};

#endif /* INCLUDED_FTW_PNC_PILOT_CC_H */
