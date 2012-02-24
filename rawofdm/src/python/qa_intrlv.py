#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr, gr_unittest
import __init__ as raw
import random

class qa_intrlv_bit (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def dotest (self, ncarriers, nbits):
        length = 10 * ncarriers * nbits
        data = [ random.randint(0,1) for i in range(length) ]
        data = tuple(data)
        src = gr.vector_source_b(data)
        enc = raw.intrlv_bit(ncarriers, nbits, False)
        dec = raw.intrlv_bit(ncarriers, nbits, True)
        dst = gr.vector_sink_b()
        self.tb.connect(src,
                        enc,dec,
                        dst)
        self.tb.run()
        self.assertEqual(data, dst.data())

    def test_001_cbp_1 (self):
        self.dotest(48, 1)
    def test_002_cbp_2 (self):
        self.dotest(48, 2)
    def test_003_cbp_4 (self):
        self.dotest(48, 4)
    def test_004_cbp_6 (self):
        self.dotest(48, 6)


class qa_intrlv_byte (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def dotest (self, nrows, slope):
        pktlen = nrows * slope;
        delay = (nrows - 1) * pktlen;
        length = pktlen * (delay + 30);
        data = [ random.randint(0,1) for i in range(length) ]
        data = tuple(data)
        src = gr.vector_source_b(data, False, pktlen)
        enc = raw.intrlv_byte(nrows, slope, False)
        dec = raw.intrlv_byte(nrows, slope, True)
        dst = gr.vector_sink_b(pktlen)
        self.tb.connect(src,
                        enc,dec,
                        dst)
        self.tb.run()
        # discard delay
        srcdata = data[:-delay]
        dstdata = dst.data()[delay:]
        self.assertEqual(srcdata, dstdata)

    def test_001_mpeg (self):
        self.dotest(12, 17)



if __name__ == '__main__':
    gr_unittest.main ()
