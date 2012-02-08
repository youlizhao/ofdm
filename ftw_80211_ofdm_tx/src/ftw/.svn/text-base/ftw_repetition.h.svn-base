/* -*- c++ -*- */
/*
 * Copyright 2007 Free Software Foundation, Inc.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef INCLUDED_FTW_OFDM_REPETITION_H
#define INCLUDED_FTW_OFDM_REPETITION_H

#include <gr_block.h>
#include <vector>

class ftw_repetition;
typedef boost::shared_ptr<ftw_repetition> ftw_repetition_sptr;

ftw_repetition_sptr
ftw_make_repetition(int symbol_length, int repetition, int N_symbols);



class ftw_repetition : public gr_block
{
  friend ftw_repetition_sptr
  ftw_make_repetition(int symbol_length, int repetition, int N_symbols);

protected:
  ftw_repetition(int symbol_length, int repetition, int N_symbols);

private:
  int d_symbol_length;
  int d_repetition;
  int d_N_symbols;

public:
  ~ftw_repetition();
  int general_work (int noutput_items,
		    gr_vector_int &ninput_items,
		    gr_vector_const_void_star &input_items,
		    gr_vector_void_star &output_items);
};

#endif /* INCLUDED_FTW_OFDM_REPETITION_H */
