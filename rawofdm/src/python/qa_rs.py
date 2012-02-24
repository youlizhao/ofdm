#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr, gr_unittest
import __init__ as raw
import random

class qa_rs (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def rstest (self, nerrors):
        pktlen = 188
        length = pktlen * 2
        data = [ random.randint(0,255) for i in range(length) ]
        data = tuple(data)
        src = gr.vector_source_b(data, False, pktlen)

        enc = raw.rs_enc()
        dec = raw.rs_dec()

        # TODO: add some noise source -- want to corrupt 8 bytes in each frame

        dst = gr.vector_sink_b(pktlen)
        #encdst = gr.vector_sink_b(pktlen+16)
        self.tb.connect(src, enc, dec, dst)
        #self.tb.connect(enc, encdst)
        self.tb.run()
        #print encdst.data()
        self.assertEqual(data, dst.data())

    def test_001_rs (self):
        self.rstest(0)

if __name__ == '__main__':
    gr_unittest.main ()
