#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

import math
from numpy import fft
from gnuradio import gr

import raw_swig as raw

class ofdm_sync(gr.hier_block2):
  """
  OFDM synchronization using pseudo-noise correlation.
  T. M. Schmidl and D. C. Cox, "Robust Frequency and Timing
  Synchonization for OFDM," IEEE Trans. Communications, vol. 45,
  no. 12, 1997.

  Assumes that the signal preamble is two identical copies of pseudo-noise
  half-symbols. Equivalently in frequency domain, the odd freqencies are 0.

  The two half-symbols are expected to be offset by pi T deltaf.
  This method can detect the timing and the frequency offset but only within
  +- 1/T.

  Let L = T/2
  P(d) = sum_{i=1..L} x[d+i] x[d+i+L]*
  R(d) = sum_{i=1..L} |x[d+i+L]|^2
  M(d) = |P(d)|^2/R(d)^2
  d_opt = argmax_d M(d)
  timing_start := d_opt
  fine_freq := angle(P(d_opt)) / (2 pi L)

  Observe: angle(P(d_opt)) is in [-pi, pi]
    Therefore max CFO that can be detected is 1/(2L) * fft_len

  outputs:
    Output 0: delayed signal
    Output 1: fine frequency correction value (to drive NCO)
    Output 2: timing signal (indicates first sample of preamble)

  NOTE: This is equivalent to ofdm_sync_pn.py in trunk.
  Edited for readability only.
  """
  def __init__(self, fft_length, cp_length, half_sync, logging=False):
    gr.hier_block2.__init__(self, "ofdm_sync_pn",
      gr.io_signature(1, 1, gr.sizeof_gr_complex), # Input signature
      gr.io_signature3(3, 3,    # Output signature
        gr.sizeof_gr_complex,   # delayed input
        gr.sizeof_float,        # fine frequency offset
        gr.sizeof_char          # timing indicator
      ))

    if half_sync:
      period = fft_length/2
      window = fft_length/2
    else: # full symbol
      period = fft_length + cp_length
      window = fft_length       # makes the plateau cp_length long

    # Calculate the frequency offset from the correlation of the preamble
    x_corr = gr.multiply_cc()
    self.connect(self, gr.conjugate_cc(), (x_corr, 0))
    self.connect(self, gr.delay(gr.sizeof_gr_complex, period), (x_corr, 1))
    P_d = gr.moving_average_cc(window, 1.0)
    self.connect(x_corr, P_d)

    P_d_angle = gr.complex_to_arg()
    self.connect(P_d, P_d_angle)

    # Get the power of the input signal to normalize the output of the correlation
    R_d = gr.moving_average_ff(window, 1.0)
    self.connect(self, gr.complex_to_mag_squared(), R_d)
    R_d_squared = gr.multiply_ff() # this is retarded
    self.connect(R_d, (R_d_squared, 0))
    self.connect(R_d, (R_d_squared, 1))
    M_d = gr.divide_ff()
    self.connect(P_d, gr.complex_to_mag_squared(), (M_d, 0))
    self.connect(R_d_squared, (M_d, 1))

    # Now we need to detect peak of M_d

    # NOTE: replaced fir_filter with moving_average for clarity
    # the peak is up to cp_length long, but noisy, so average it out
    #matched_filter_taps = [1.0/cp_length for i in range(cp_length)]
    #matched_filter = gr.fir_filter_fff(1, matched_filter_taps)
    matched_filter = gr.moving_average_ff(cp_length, 1.0/cp_length)

    # NOTE: the look_ahead parameter doesn't do anything
    # these parameters are kind of magic, increase 1 and 2 (==) to be more tolerant
    #peak_detect = raw.peak_detector_fb(0.55, 0.55, 30, 0.001)
    peak_detect = raw.peak_detector_fb(0.25, 0.25, 30, 0.001)
    # NOTE: gr.peak_detector_fb is broken!
    #peak_detect = gr.peak_detector_fb(0.55, 0.55, 30, 0.001)
    #peak_detect = gr.peak_detector_fb(0.45, 0.45, 30, 0.001)
    #peak_detect = gr.peak_detector_fb(0.30, 0.30, 30, 0.001)

    # offset by -1
    self.connect(M_d, matched_filter, gr.add_const_ff(-1), peak_detect)

    # peak_detect indicates the time M_d is highest, which is the end of the symbol.
    # We should try to sample in the middle of the plateau!!
    # FIXME until we figure out how to do this, just offset by cp_length/2
    offset = 6 #cp_length/2

    # nco(t) = P_d_angle(t-offset) sampled at peak_detect(t)
    # modulate input(t - fft_length) by nco(t)
    # signal to sample input(t) at t-offset
    #
    # We can't delay by < 0 so instead:
    # input is delayed by fft_length
    # P_d_angle is delayed by offset
    # signal to sample is delayed by fft_length - offset
    #
    phi = gr.sample_and_hold_ff()
    self.connect(peak_detect, (phi,1))
    self.connect(P_d_angle, gr.delay(gr.sizeof_float, offset), (phi,0))
    #self.connect(P_d_angle, matched_filter2, (phi,0)) # why isn't this better?!?

    # FIXME: we add fft_length delay so that the preamble is nco corrected too
    # BUT is this buffering worth it? consider implementing sync as a proper block

    # delay the input signal to follow the frequency offset signal
    self.connect(self, gr.delay(gr.sizeof_gr_complex, (fft_length+offset)), (self,0))
    self.connect(phi, (self,1))
    self.connect(peak_detect, (self,2))

    if logging:
      self.connect(matched_filter, gr.file_sink(gr.sizeof_float, "sync-mf.dat"))
      self.connect(M_d, gr.file_sink(gr.sizeof_float, "sync-M.dat"))
      self.connect(P_d_angle, gr.file_sink(gr.sizeof_float, "sync-angle.dat"))
      self.connect(peak_detect, gr.file_sink(gr.sizeof_char, "sync-peaks.datb"))
      self.connect(phi, gr.file_sink(gr.sizeof_float, "sync-phi.dat"))


