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

#ifndef INCLUDED_FTW_PNC_OFDM_INSERT_PREAMBLE_H
#define INCLUDED_fTW_PNC_OFDM_INSERT_PREAMBLE_H

#include <gr_sync_block.h>
#include <vector>

class ftw_pnc_ofdm_preamble;
typedef boost::shared_ptr<ftw_pnc_ofdm_preamble> ftw_pnc_ofdm_preamble_sptr;

ftw_pnc_ofdm_preamble_sptr
ftw_make_pnc_ofdm_preamble(int symbol_length, int N_symbols,
			     const std::vector<std::vector<gr_complex> > &preamble, int user);



class ftw_pnc_ofdm_preamble : public gr_block
{
  friend ftw_pnc_ofdm_preamble_sptr
  ftw_make_pnc_ofdm_preamble(int symbol_length, int N_symbols,
			       const std::vector<std::vector<gr_complex> > &preamble, int user);

protected:
  ftw_pnc_ofdm_preamble(int symbol_length, int N_symbols,
			  const std::vector<std::vector<gr_complex> > &preamble, int user);

private:
  enum state_t {
    ST_IDLE,
    ST_FIRST_PAYLOAD_AND_PREAMBLE,
  };

  int						d_symbol_length;
  int						d_N_symbols;
  const std::vector<std::vector<gr_complex> > 	d_preamble;
  state_t					d_state;
  int						d_nsymbols_output;
  int						d_pending_flag;
  int                                           d_user;   // role of user


  void enter_idle();
  void enter_first_payload_and_preamble();
  

public:
  ~ftw_pnc_ofdm_preamble();

  int general_work (int noutput_items,
			gr_vector_int &ninput_items_v,
			gr_vector_const_void_star &input_items,
		        gr_vector_void_star &output_items);
};

#endif /* INCLUDED_ftw_ofdm_OFDM_INSERT_PREAMBLE_H */
