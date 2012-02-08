/* -*- c++ -*- */
/*
 * Copyright 2005 Free Software Foundation, Inc.
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

%include "gnuradio.i"			// the common stuff

%{
#include "ftw_crc32.h"
#include "ftw_ofdm_cmap_cc.h"
#include "ftw_ofdm_mapper.h"
#include "ftw_ofdm_pilot_cc.h"
#include "ftw_ofdm_preamble.h"
#include "ftw_repetition.h"
#include "ftw_zerogap.h"
%}

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(ftw,ofdm_cmap_cc);

ftw_ofdm_cmap_cc_sptr ftw_make_ofdm_cmap_cc (int fft_size, int tones);

class ftw_ofdm_cmap_cc : public gr_block
{
private:
  ftw_ofdm_cmap_cc (int fft_size, int tones);
};

//-----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(ftw,ofdm_pilot_cc);

ftw_ofdm_pilot_cc_sptr ftw_make_ofdm_pilot_cc (int tones);

class ftw_ofdm_pilot_cc : public gr_block
{
private:
  ftw_ofdm_pilot_cc (int tones);
};

//-----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(ftw,ofdm_preamble);

ftw_ofdm_preamble_sptr
ftw_make_ofdm_preamble(int fft_length, int N_symbols,
			     const std::vector<std::vector<gr_complex> > &preamble);


class ftw_ofdm_preamble : public gr_block
{
 protected:
  ftw_ofdm_preamble(int fft_length, int N_symbols,
			  const std::vector<std::vector<gr_complex> > &preamble);
};

//----------------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(ftw,zerogap);

ftw_zerogap_sptr
ftw_make_zerogap(int symbol_length, int N_symbols,
			     const std::vector<std::vector<gr_complex> > &zerogap);


class ftw_zerogap : public gr_block
{
 protected:
  ftw_zerogap(int fft_length, N_symbols,
			  const std::vector<std::vector<gr_complex> > &zerogap);
};

//----------------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(ftw,repetition);

ftw_repetition_sptr
ftw_make_repetition(int fft_length, int repetition , int N_symbols);


class ftw_repetition : public gr_block
{
 protected:
  ftw_repetition(int fft_length, int repetition, int N_symbols);
};

//----------------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(ftw,ofdm_mapper);

ftw_ofdm_mapper_sptr 
ftw_make_ofdm_mapper (const std::vector<gr_complex> &constellation,
			 unsigned int msgq_limit,
			 unsigned int bits_per_symbol, 
			 unsigned int fft_length);


class ftw_ofdm_mapper : public gr_sync_block
{
 protected:
  ftw_ofdm_mapper (const std::vector<gr_complex> &constellation,
		      unsigned int msgq_limit,
		      unsigned int bits_per_symbol,
		      unsigned int fft_length);
  
 public:
  gr_msg_queue_sptr msgq();
  
  int work(int noutput_items,
	   gr_vector_const_void_star &input_items,
	   gr_vector_void_star &output_items);
};

//-----------------------------------------------------------------

unsigned int ftw_update_crc32(unsigned int crc, const unsigned char *data, size_t len);
unsigned int ftw_update_crc32(unsigned int crc, const std::string s);
unsigned int ftw_crc32(const unsigned char *buf, size_t len);
unsigned int ftw_crc32(const std::string s);
