#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr, gr_unittest
import __init__ as raw

class qa_qam (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def qamtest (self, nbits):
        data = tuple(range(2**nbits))

        src = gr.vector_source_b(data)
        unpack = gr.unpack_k_bits_bb(nbits)
        enc = raw.qam_enc(nbits)
        dec = raw.qam_dec(nbits)
        tofloat = gr.uchar_to_float()
        slicer = gr.binary_slicer_fb()

        unpacked = gr.vector_sink_b()
        encoded = gr.vector_sink_c()
        decoded = gr.vector_sink_b()
        dst = gr.vector_sink_b()
        self.tb.connect(src,
                        unpack,
                        enc,
                        dec,
                        tofloat,
                        gr.add_const_ff(-128.0),
                        slicer,
                        dst)
        self.tb.connect(unpack, unpacked)
        self.tb.connect(enc, encoded)
        self.tb.connect(dec, decoded)
        self.tb.run()
        #print "data = ", data
        #print "unpacked = ", unpacked.data()
        #print "encoded = ", encoded.data()
        #print "decoded = ", decoded.data()
        #print "dst = ", dst.data()
        self.assertEqual(unpacked.data(), dst.data())
        pwr = sum([abs(x)**2 for x in encoded.data()]) / len(encoded.data())
        #print pwr
        self.assertAlmostEqual(1.0, pwr, 6)
        #print

    def test_001_qam_1 (self):
        self.qamtest(1)

    def test_002_qam_2 (self):
        self.qamtest(2)

    def test_003_qam_4 (self):
        self.qamtest(4)

    def test_004_qam_6 (self):
        self.qamtest(6)

if __name__ == '__main__':
    gr_unittest.main ()
