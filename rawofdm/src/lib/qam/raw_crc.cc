/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_crc.h>
#include <gr_io_signature.h>
#include <string.h>

raw_crc_enc_sptr
 raw_make_crc_enc (unsigned length) {
   return raw_crc_enc_sptr (new raw_crc_enc(length));
}

raw_crc_enc::raw_crc_enc (unsigned length)
  : gr_sync_block ("crc_enc",
                    gr_make_io_signature (1, 1, length * sizeof(char)),
                    gr_make_io_signature (1, 1, length * sizeof(char) + sizeof(uint32_t))),
    d_length(length)
{
}

int raw_crc_enc::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  char unsigned *out = (char unsigned *)output_items[0];

  for (unsigned i = 0; i < noutput_items; ++i) {
    memcpy(out, in, d_length);
    *(uint32_t *)(out+d_length) = digital_crc32(in, d_length);

    in+= d_length;
    out+= d_length + sizeof(uint32_t);
  }
  return noutput_items;
}

raw_crc_dec_sptr
 raw_make_crc_dec (unsigned length) {
   return raw_crc_dec_sptr (new raw_crc_dec(length));
}

raw_crc_dec::raw_crc_dec (unsigned length)
  : gr_block ("crc_dec",
                    gr_make_io_signature (1, 1, length * sizeof(char) + sizeof(uint32_t)),
                    gr_make_io_signature (1, 1, length * sizeof(char))),
    d_length(length)
{
}

int
raw_crc_dec::general_work(int noutput_items,
                   gr_vector_int &ninput_items,
                   gr_vector_const_void_star &input_items,
                   gr_vector_void_star &output_items)
{
  const unsigned char *in = (const unsigned char *)input_items[0];
  char unsigned *out = (char unsigned *)output_items[0];

  int ninput = ninput_items[0];
  int nconsumed = 0;
  int nproduced = 0;
  while ((nconsumed < ninput) && (nproduced < noutput_items)) {
    uint32_t check = *(uint32_t *)(in + d_length);
    if (check == digital_crc32(in, d_length)) {
      memcpy(out, in, d_length);
      out+= d_length;
      ++nconsumed;
    }
    in+= d_length + sizeof(uint32_t);
    ++nproduced;
  }
  consume(0, nconsumed);
  return nproduced;
}


