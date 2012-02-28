#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

import math
from gnuradio import gr
from gnuradio import digital

# local:
import raw_swig as raw
from raw_ofdm_params import ofdm_params # need that because raw has the cc stuff only
from raw_ofdm_rx import ofdm_receiver

# /////////////////////////////////////////////////////////////////////////////
#                   mod/demod with packets as i/o
# /////////////////////////////////////////////////////////////////////////////

class ofdm_mod(gr.hier_block2):
  """
  Modulates an OFDM stream. Creates OFDM symbols from RAW samples.

  input[0]: data samples
  input[1]: start-of-frame indicator (one per symbol)

  output[0]: baseband-modulated signal
  """
  def __init__(self, options):
    """
    @param options: parsed raw.ofdm_params
    """

    self.params = ofdm_params(options)
    params = self.params

    gr.hier_block2.__init__(self, "ofdm_mod",
      gr.io_signature2(2, 2, gr.sizeof_gr_complex*params.data_tones, gr.sizeof_char), # Input signature
      gr.io_signature(1, 1, gr.sizeof_gr_complex)) # Output signature

    win = [] #[1 for i in range(self._fft_length)]
    # see gr_fft_vcc_fftw that it works differently if win = [1 1 1 ...]

    self.mapper = raw.ofdm_mapper(params.padded_carriers)
    self.preambles = digital.ofdm_insert_preamble(params.fft_length, params.padded_preambles)
    self.ifft = gr.fft_vcc(params.fft_length, False, win, True)
    self.cp_adder = digital.ofdm_cyclic_prefixer(params.fft_length, params.fft_length + params.cp_length)
    self.scale = gr.multiply_const_cc(1.0 / math.sqrt(params.fft_length))

    self.connect((self,0), self.mapper, (self.preambles,0))
    self.connect((self,1), (self.preambles,1))
    self.connect(self.preambles, self.ifft, self.cp_adder, self.scale, self)

    if options.log:
        self.connect(self.mapper,
                     gr.file_sink(gr.sizeof_gr_complex*params.fft_length,
                                  "tx-map.dat"))
        self.connect(self.preambles,
                     gr.file_sink(gr.sizeof_gr_complex*params.fft_length,
                                  "tx-pre.dat"))
        self.connect(self.ifft,
                     gr.file_sink(gr.sizeof_gr_complex*params.fft_length,
                                  "tx-ifft.dat"))
        self.connect(self.cp_adder,
                     gr.file_sink(gr.sizeof_gr_complex,
                                  "tx-cp.dat"))

  def add_options(normal, expert):
    ofdm_params.add_options(normal, expert)

  add_options = staticmethod(add_options)


class ofdm_demod(gr.hier_block2):
  """
  Demodulates a received OFDM stream.
  This block performs synchronization, FFT, and demodulation of incoming OFDM
  symbols and passes packets up the a higher layer.

  input[0]: complex base band

  output[0]: data samples
  output[1]: start-of-frame indicator (one per symbol)
  output[2]: average symbol noise
  output[3]: average symbol power
  """

  def __init__(self, options, noutputs = 2):
    """
    @param options: parsed raw.ofdm_params
    """
    self.params = ofdm_params(options)
    params = self.params

    if noutputs == 2:
      output_signature = gr.io_signature2(2, 2,
        gr.sizeof_gr_complex*params.data_tones,
        gr.sizeof_char
      )
    elif noutputs == 3:
      output_signature = gr.io_signature3(3, 3,
        gr.sizeof_gr_complex*params.data_tones,
        gr.sizeof_char,
        gr.sizeof_float
      )
    elif noutputs == 4:
      output_signature = gr.io_signature4(4, 4,
        gr.sizeof_gr_complex*params.data_tones,
        gr.sizeof_char,
        gr.sizeof_float,
        gr.sizeof_float
      )
    else:
      # error
      raise Exception("unsupported number of outputs")

    gr.hier_block2.__init__(self, "ofdm_demod",
      gr.io_signature(1, 1, gr.sizeof_gr_complex),
      output_signature
    )

    self.ofdm_recv = ofdm_receiver(params, options.log)

    # FIXME: magic parameters
    phgain = 0.4
    frgain = phgain*phgain / 4.0
    eqgain = 0.05
    self.ofdm_demod = raw.ofdm_demapper(params.carriers,
                                        phgain, frgain, eqgain)

    # the studios can't handle the whole ofdm in one thread
    #ofdm_recv = raw.wrap_sts(self.ofdm_recv)
    ofdm_recv = self.ofdm_recv

    self.connect(self, ofdm_recv)
    self.connect((ofdm_recv,0), (self.ofdm_demod,0))
    self.connect((ofdm_recv,1), (self.ofdm_demod,1))

    self.connect(self.ofdm_demod, (self,0))
    self.connect((ofdm_recv,1), (self,1))

    if noutputs > 2:
      # average noise power per (pilot) subcarrier
      self.connect((self.ofdm_demod,1), (self,2))

    if noutputs > 3:
      # average signal power per subcarrier
      self.connect((ofdm_recv,0),
        gr.vector_to_stream(gr.sizeof_float, params.occupied_tones),
        gr.integrate_ff(params.occupied_tones),
        gr.multiply_ff(1.0/params.occupied_tones), (self,3))

    if options.log:
      self.connect((self.ofdm_demod, 2),
                   gr.file_sink(gr.sizeof_gr_complex*params.occupied_tones,
                                'rx-eq.dat'))
      self.connect((self.ofdm_demod, 1),
                   gr.file_sink(gr.sizeof_float,
                                'rx-noise.dat'))
      self.connect((self.ofdm_demod, 0),
                   gr.file_sink(gr.sizeof_gr_complex*params.data_tones,
                                'rx-demap.dat'))

  def add_options(normal, expert):
    ofdm_params.add_options(normal, expert)

  add_options = staticmethod(add_options)

