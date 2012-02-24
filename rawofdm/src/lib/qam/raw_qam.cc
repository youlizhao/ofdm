/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <raw_qam.h>
#include <gr_io_signature.h>
#include <limits.h>

// return +1 or -1
static inline int sign(int v) {
  return +1 | (v >> (sizeof(int) * CHAR_BIT - 1));
}

// saturate between 0 and 255
static inline int clamp(int i) {
  return i < 0 ? 0 : (i > 255 ? 255 : i);
}

/**
 * fast QAM (uses at most 4x imull to decode)
 * Tested on 7600 bogomips yields 600-1200Mbps encoding and 300Mbps decoding
 * Compile with -O3
 */
template<int NumBits>
class QAM {
  int d_gain;
  float d_scale_e;
  float d_scale_d;
public:
  /**
   * power -- desired symbol power
   * gain  -- gain on the decoded confidence (power of 2)
   */
  QAM (float power, int gain = 0) {
    d_gain = gain + CHAR_BIT - NumBits;
    const int nn = (1<<(NumBits-1));
    // sum((2k+1)^2,k=0..n-1)
#if 0
    int sum2 = 0;
    for (int i = 0; i < nn; ++i) {
      sum2+= (2*i + 1)*(2*i + 1);
    }
#else
    int sum2 = (4*nn*nn*nn-nn)/3;
#endif
    float sf = sqrt(power * float(nn) / float(sum2));
    d_scale_e = sf;
    d_scale_d = (1 << d_gain) / sf;
  }

  /**
   * We encode recursively to match the decoding process.
   *
   * This could have been implemented by a gray + multiply.
   * gray(i) = (i>>1)^i
   * see: http://www.dspguru.com/dsp/tricks/gray-code-conversion
   */
  inline void encode (const char* bits, float *sym) {
    int pt = 0; // constellation point
    int flip = 1; // +1 or -1 -- for gray coding
    // unrolled with -O3
    for (int i = 0; i < NumBits; ++i) {
      int bit = *bits * 2 - 1; // +1 or -1
      pt = bit * flip + pt * 2;
      flip *= -bit;
      ++bits;
    }
    *sym = pt * d_scale_e;
  }

  /**
   * We decode recursively because we want meaningful confidences.
   * The alternative would be to simply divide and round, which only yields
   * the smallest per-bit confidence.
   *
   * output bit confidence is between 0 and 255
   */
  inline void decode (float sym, unsigned char *bits) {
    int pt = sym * d_scale_d;
    int flip = 1; // +1 or -1 -- for gray coding
    int amp = (1 << (NumBits-1)) << d_gain;
    // unrolled with -O3
    for (int i = 0; i < NumBits; ++i) {
      *bits = clamp(flip * pt + 128);
      int bit = sign(pt);
      pt -= bit * amp;
      flip = -bit;
      amp /= 2;
      ++bits;
    }
  }
};

raw_qam_enc_sptr
 raw_make_qam_enc (int nbits) {
   return raw_qam_enc_sptr (new raw_qam_enc(nbits));
}

raw_qam_enc::raw_qam_enc (int nbits)
  : gr_sync_decimator ("qam_enc",
                       gr_make_io_signature (1, 1, sizeof(char)),
                       gr_make_io_signature (1, 1, sizeof(gr_complex)),
                       nbits),
    d_nbits(nbits)
{
}

int raw_qam_enc::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const char *in = (const char *)input_items[0];
  float *out = (float *)output_items[0];

  switch(d_nbits) {
  case 1: { // BPSK
    QAM<1> q(1.0f);
    for (int i = 0; i < noutput_items; ++i) {
      q.encode(in, out); in+= 1; out+= 2;
    }
  } break;
  case 2: { // QPSK
    QAM<1> q(0.5f);
    for (int i = 0; i < noutput_items; ++i) {
      q.encode(in, out); in+= 1; ++out;
      q.encode(in, out); in+= 1; ++out;
    }
  } break;
  case 4: { // QAM16
    QAM<2> q(0.5f);
    for (int i = 0; i < noutput_items; ++i) {
      q.encode(in, out); in+= 2; ++out;
      q.encode(in, out); in+= 2; ++out;
    }
  } break;
  case 6: { // QAM64
    QAM<3> q(0.5f);
    for (int i = 0; i < noutput_items; ++i) {
      q.encode(in, out); in+= 3; ++out;
      q.encode(in, out); in+= 3; ++out;
    }
  } break;
  }
  return noutput_items;
}

raw_qam_dec_sptr
 raw_make_qam_dec (int nbits) {
   return raw_qam_dec_sptr (new raw_qam_dec(nbits));
}

raw_qam_dec::raw_qam_dec (int nbits)
  : gr_sync_interpolator ("qam_dec",
                       gr_make_io_signature (1, 1, sizeof(gr_complex)),
                       gr_make_io_signature (1, 1, sizeof(unsigned char)),
                       nbits),
    d_nbits(nbits)
{
}

int raw_qam_dec::work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
{
  const float *in = (const float *)input_items[0];
  unsigned char *out = (unsigned char *)output_items[0];

  switch(d_nbits) {
  case 1: { // BPSK
    QAM<1> q(1.0f);
    for (int i = 0; i < noutput_items; ++i) {
      q.decode(*in, out); in+= 2; out+= 1;
    }
  } break;
  case 2: { // QPSK
    QAM<1> q(0.5f);
    for (int i = 0; i < noutput_items/2; ++i) {
      q.decode(*in, out); in+= 1; out+= 1;
      q.decode(*in, out); in+= 1; out+= 1;
    }
  } break;
  case 4: { // QAM16
    QAM<2> q(0.5f);
    for (int i = 0; i < noutput_items/4; ++i) {
      q.decode(*in, out); in+= 1; out+= 2;
      q.decode(*in, out); in+= 1; out+= 2;
    }
  } break;
  case 6: { // QAM64
    QAM<3> q(0.5f);
    for (int i = 0; i < noutput_items/6; ++i) {
      q.decode(*in, out); in+= 1; out+= 3;
      q.decode(*in, out); in+= 1; out+= 3;
    }
  } break;
  }
  return noutput_items;
}


