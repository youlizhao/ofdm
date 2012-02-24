/* -*- c++ -*- */
/*
 * Copyright 2010 Szymon Jakubczak
 */

#ifndef INCLUDED_RAW_OFDM_FRAME_ACQUISITION_H
#define INCLUDED_RAW_OFDM_FRAME_ACQUISITION_H

#include <gr_block.h>
#include <vector>

class raw_ofdm_frame_acquisition;
typedef boost::shared_ptr<raw_ofdm_frame_acquisition> raw_ofdm_frame_acquisition_sptr;

raw_ofdm_frame_acquisition_sptr
raw_make_ofdm_frame_acquisition(unsigned int fft_length,
                                unsigned int cplen,
                                const std::vector<std::vector<gr_complex> > &preamble);

/*!
 * \brief Takes output of the FFT compensated for fine frequency offset
 * and performs correlation with the known symbol to equalize timing offset
 * and coarse frequency offset.
 *
 * The PN sequence used for known symbol should be a PSK sequence.
 * The first symbol has no odd frequencies and is used for coarse freq offset.
 * The next num_symbols have all frequencies and are used for channel estimation.
 *
 * \ingroup ofdm_blk
 *
 */

class raw_ofdm_frame_acquisition : public gr_block
{
  /*!
   * \brief blah
   * \param occupied_carriers The number of subcarriers with data in the received symbol
   * \param fft_length        The size of the FFT vector (occupied_carriers + unused carriers)
   * \param cplen             The length of the cycle prefix
   * \param preamble          A vector of vectors of complex numbers representing
   *                          the known symbol at the start of a frame (a PSK PN sequence)
   */
  friend raw_ofdm_frame_acquisition_sptr
  raw_make_ofdm_frame_acquisition(unsigned int fft_length,
                                  unsigned int cplen,
                                  const std::vector<std::vector<gr_complex> > &preamble);

protected:
  raw_ofdm_frame_acquisition (unsigned int fft_length,
                              unsigned int cplen,
                              const std::vector<std::vector<gr_complex> > &preamble);

 private:
  void forecast(int noutput_items, gr_vector_int &ninput_items_required);
  float correlate(const gr_complex *symbol, int &coarse_freq);

  // channel estimate computation
  void init_estimate(const gr_complex *symbol); // one even-freq-only symbol
  void update_estimate(const gr_complex *symbol); // extra symbols
  void finish_estimate(); // normalize

  inline gr_complex compensate() const; // coarse freq compensation factor
  inline int pad() const;

  // params
  unsigned int d_occupied_carriers; // includes DC
  unsigned int d_fft_length;
  unsigned int d_cplen;

  const std::vector<std::vector<gr_complex> >  d_preamble;
  // derivative params
  std::vector<gr_complex> d_known_diff; // of d_preamble[0]
  float d_known_norm;

  // dynamic state
  std::vector<gr_complex> d_symbol_diff; // storage
  int d_coarse_freq;                    // frequency offset in number of bins
  unsigned int d_cur_symbol;                     // how many symbols into the frame we are
  std::vector<gr_complex> d_hestimate;  // channel estimate (includes timing offset)

  unsigned int d_min_symbols; // how short frames to expect (excluding preambles)
  unsigned int d_num_frames; // how many frames to expect in a burst?
  unsigned int d_cur_frame;

  bool d_signal_out; // should indicate signal_out on the next symbol

 public:
  ~raw_ofdm_frame_acquisition(void);
  int general_work(int noutput_items,
                  gr_vector_int &ninput_items,
                  gr_vector_const_void_star &input_items,
                  gr_vector_void_star &output_items);
  void set_min_symbols(int val) { d_min_symbols = val; }
  void set_num_frames(int val) { d_num_frames = val; }
};

#endif //INCLUDED_RAW_OFDM_FRAME_ACQUISITION_H
