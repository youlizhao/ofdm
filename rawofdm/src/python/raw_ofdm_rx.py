#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

import math
from gnuradio import gr

# local:
import raw_swig as raw
from raw_ofdm_params import ofdm_params # need that because raw has the cc stuff only
from raw_ofdm_sync import ofdm_sync

# /////////////////////////////////////////////////////////////////////////////
#                   OFDM receiver
# /////////////////////////////////////////////////////////////////////////////

class ofdm_receiver(gr.hier_block2):
  """
  Performs receiver synchronization on OFDM symbols.

  The receiver performs channel filtering as well as symbol, frequency, and phase synchronization.
  """

  def __init__(self, p, logging=False):
    """
    Hierarchical block for receiving OFDM symbols.

    The input is the complex modulated signal at baseband.
    Synchronized packets are sent back to the demodulator.

    @param params: Raw OFDM parameters
    @type  params: ofdm_params
    @param logging: turn file logging on or off
    @type  logging: bool
    """
    from numpy import fft

    gr.hier_block2.__init__(self, "ofdm_receiver",
      gr.io_signature(1, 1, gr.sizeof_gr_complex), # Input signature
      gr.io_signature2(2, 2, gr.sizeof_gr_complex*p.occupied_tones, gr.sizeof_char)) # Output signature

    # low-pass filter the input channel
    bw = (float(p.occupied_tones) / float(p.fft_length)) / 2.0
    tb = bw*0.08
    lpf_coeffs = gr.firdes.low_pass (1.0,                     # gain
                                     1.0,                     # sampling rate
                                     bw+tb,                   # midpoint of trans. band
                                     tb,                      # width of trans. band
                                     gr.firdes.WIN_HAMMING)   # filter type
    lpf = gr.fft_filter_ccc(1, lpf_coeffs)
    #lpf = gr.add_const_cc(0.0)  ## no-op low-pass-filter
    self.connect(self, lpf)

    # to the synchronization algorithm
    #from gnuradio import blks2impl
    #from gnuradio.blks2impl.ofdm_sync_pn import ofdm_sync_pn
    sync = ofdm_sync(p.fft_length, p.cp_length, p.half_sync, logging)
    self.connect(lpf, sync)

    # correct for fine frequency offset computed in sync (up to +-pi/fft_length)
    # NOTE: frame_acquisition can correct coarse freq offset (i.e. kpi/fft_length)
    if p.half_sync:
      nco_sensitivity = 2.0/p.fft_length
    else:
      nco_sensitivity = 1.0/(p.fft_length + p.cp_length)
    #nco_sensitivity = 0

    nco = gr.frequency_modulator_fc(nco_sensitivity)
    sigmix = gr.multiply_cc()

    self.connect((sync,0), (sigmix,0))
    self.connect((sync,1), nco, (sigmix,1))

    # sample at symbol boundaries
    # NOTE: (sync,2) indicates the first sample of the symbol!
    sampler = raw.ofdm_sampler(p.fft_length, p.fft_length+p.cp_length, timeout=100)
    self.connect(sigmix, (sampler,0))
    self.connect((sync,2), (sampler,1))  # timing signal to sample at

    # fft on the symbols
    win = [1 for i in range(p.fft_length)]
    # see gr_fft_vcc_fftw that it works differently if win = []
    fft = gr.fft_vcc(p.fft_length, True, win, True)
    self.connect((sampler,0), fft)

    # use the preamble to correct the coarse frequency offset and initial equalizer
    frame_acq = raw.ofdm_frame_acquisition(p.fft_length, p.cp_length, p.preambles)

    self.frame_acq = frame_acq
    self.connect(fft, (frame_acq,0))
    self.connect((sampler,1), (frame_acq,1))

    self.connect((frame_acq,0), (self,0))  # finished with fine/coarse freq correction
    self.connect((frame_acq,1), (self,1))  # frame and symbol timing

    if logging:
      self.connect(lpf,
                   gr.file_sink(gr.sizeof_gr_complex, "rx-filt.dat"))
      self.connect(fft,
                   gr.file_sink(gr.sizeof_gr_complex*p.fft_length, "rx-fft.dat"))
      self.connect((frame_acq,0),
                   gr.file_sink(gr.sizeof_gr_complex*p.occupied_tones, "rx-acq.dat"))
      self.connect((frame_acq,1),
                   gr.file_sink(1, "rx-detect.datb"))
      self.connect(sampler,
                   gr.file_sink(gr.sizeof_gr_complex*p.fft_length, "rx-sampler.dat"))
      self.connect(sigmix,
                   gr.file_sink(gr.sizeof_gr_complex, "rx-sigmix.dat"))
      self.connect(nco,
                   gr.file_sink(gr.sizeof_gr_complex, "rx-nco.dat"))

