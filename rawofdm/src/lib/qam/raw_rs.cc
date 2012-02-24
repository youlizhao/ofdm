/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_rs.h>
#include <gr_io_signature.h>
#include <string.h>
#include <ecc.h>

const int ParitySize = NPAR;
const int PlainSize = 188;
const int CodedSize = PlainSize + ParitySize;
const int FullSize = 255;
const int PadSize = FullSize - CodedSize;

raw_rs_enc_sptr
 raw_make_rs_enc () {
   return raw_rs_enc_sptr (new raw_rs_enc());
}

raw_rs_enc::raw_rs_enc ()
  : gr_sync_block ("rs_enc",
                    gr_make_io_signature (1, 1, PlainSize * sizeof(char)),
                    gr_make_io_signature (1, 1, CodedSize * sizeof(char)))
{
  memset(msg, 0, sizeof(msg));
  memset(code, 0, sizeof(code));
}

int raw_rs_enc::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  char unsigned *out = (char unsigned *)output_items[0];

  for (unsigned i = 0; i < noutput_items; ++i) {
    memcpy(msg + PadSize, in, PlainSize);
    d_enc.encode_data(msg, PlainSize + PadSize, code);
    memcpy(out, code + PadSize, CodedSize);

    in+= PlainSize;
    out+= CodedSize;
  }
  return noutput_items;
}

raw_rs_dec_sptr
 raw_make_rs_dec () {
   return raw_rs_dec_sptr (new raw_rs_dec());
}

raw_rs_dec::raw_rs_dec ()
  : gr_sync_block ("rs_dec",
                    gr_make_io_signature (1, 1, CodedSize * sizeof(char)),
                    gr_make_io_signature (1, 1, PlainSize * sizeof(char)))
{
  memset(msg, 0, sizeof(msg));
  memset(code, 0, sizeof(code));
}

int raw_rs_dec::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  char unsigned *out = (char unsigned *)output_items[0];

  for (unsigned i = 0; i < noutput_items; ++i) {
    // read PlainSize into msg + PadSize
    memcpy(code + PadSize, in, CodedSize);
    d_dec.decode_data(code, FullSize);
    if (d_dec.check_syndrome()) {
      // TODO: consider marking erasures somehow?
      d_dec.correct_errors_erasures (code, FullSize, 0, NULL);
    }
    memcpy(out, code + PadSize, PlainSize);

    in+= CodedSize;
    out+= PlainSize;
  }
  return noutput_items;
}

