#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr, gr_unittest

import __init__ as raw

import math
import cmath
import random
import numpy

class Options:
  fft_length = 64
  occupied_tones = 52
  cp_length = 16
  num_preambles = 2
  half_sync = True
  size = 4
  verbose = False
  log = False

class qa_ofdm (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def ofdmtest (self):
        options = Options()

        enc = raw.ofdm_mod(options)
        dec = raw.ofdm_demod(options, noutputs=2)

        NN = 4
        qpsk = lambda : cmath.exp(1j * (math.pi/NN * (2*random.randint(0, NN-1)+1)))

        n = enc.params.data_tones * options.size * 1000
        data = tuple([qpsk() for i in range(n)])

        signal = [0] * options.size
        signal[0] = 1
        signal = tuple(signal)

        msg = gr.vector_source_b(signal, True, 1)
        src = gr.vector_source_c(data, False, enc.params.data_tones)
        dst = gr.vector_sink_c(enc.params.data_tones)

        self.tb.connect(src,
                        enc,
                        dec,
                        dst)
        self.tb.connect(msg, (enc,1))
        self.tb.run()
        rxdata = numpy.array(dst.data())
        txdata = numpy.array(data[:len(rxdata)])
        power = numpy.average(numpy.square(numpy.abs(txdata)))
        print len(txdata), len(rxdata), power
        mse = numpy.average(numpy.square(numpy.abs(numpy.subtract(txdata,rxdata))))
        self.assertAlmostEquals(1.0, power, 7)
        snr = 10 * math.log10(power / mse)
        self.assertTrue(snr > 40) # that's a pretty low estimate for noiseless

    def test_001_ofdm (self):
        self.ofdmtest()


if __name__ == '__main__':
    gr_unittest.main ()
