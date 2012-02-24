/* -*- c++ -*- */

%include "gnuradio.i"                   // the common stuff

%{
#include "raw_qam.h"
#include "raw_conv.h"
#include "raw_intrlv.h"
#include "raw_ber.h"
#include "raw_rs.h"
#include "raw_scrambler_bb.h"
#include "raw_crc.h"
%}

GR_SWIG_BLOCK_MAGIC(raw,qam_enc)
raw_qam_enc_sptr raw_make_qam_enc (int nbits);
class raw_qam_enc : public gr_sync_decimator {};

GR_SWIG_BLOCK_MAGIC(raw,qam_dec)
raw_qam_dec_sptr raw_make_qam_dec (int nbits);
class raw_qam_dec : public gr_sync_interpolator {};

GR_SWIG_BLOCK_MAGIC(raw,conv_enc)
raw_conv_enc_sptr raw_make_conv_enc ();
class raw_conv_enc : public gr_sync_interpolator {};

GR_SWIG_BLOCK_MAGIC(raw,conv_dec)
raw_conv_dec_sptr raw_make_conv_dec (int length);
class raw_conv_dec : public gr_sync_decimator
{
 public:
  void set_length (int length);
};

GR_SWIG_BLOCK_MAGIC(raw,conv_punc)
raw_conv_punc_sptr raw_make_conv_punc (int nc, int np, int zero = 0);
class raw_conv_punc : public gr_block {};

GR_SWIG_BLOCK_MAGIC(raw,intrlv_bit)
raw_intrlv_bit_sptr raw_make_intrlv_bit (int ncarriers, int nbits, bool inverse);
class raw_intrlv_bit : public gr_sync_block {};

GR_SWIG_BLOCK_MAGIC(raw,intrlv_byte)
raw_intrlv_byte_sptr raw_make_intrlv_byte (int nrows, int slope, bool inverse);
class raw_intrlv_byte : public gr_sync_block {};

GR_SWIG_BLOCK_MAGIC(raw,rs_enc)
raw_rs_enc_sptr raw_make_rs_enc ();
class raw_rs_enc : public gr_sync_block {};

GR_SWIG_BLOCK_MAGIC(raw,rs_dec)
raw_rs_dec_sptr raw_make_rs_dec ();
class raw_rs_dec : public gr_sync_block {};

GR_SWIG_BLOCK_MAGIC(raw,scrambler_bb)
raw_scrambler_bb_sptr raw_make_scrambler_bb (unsigned length=1, unsigned seed=0);
class raw_scrambler_bb : public gr_block {};

GR_SWIG_BLOCK_MAGIC(raw,crc_enc)
raw_crc_enc_sptr raw_make_crc_enc (unsigned length);
class raw_crc_enc : public gr_sync_block {};

GR_SWIG_BLOCK_MAGIC(raw,crc_dec)
raw_crc_dec_sptr raw_make_crc_dec (unsigned length);
class raw_crc_dec : public gr_block {};

