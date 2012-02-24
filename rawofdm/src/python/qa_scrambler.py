#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr, gr_unittest
import __init__ as raw
import random

class qa_scrambler (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def scrambletest (self):
        length = 128
        count = 1024
        data = tuple([random.randint(0, 255) for i in range(count*length)])
        seed = tuple([random.randint(1, 127) for i in range(count)])

        src = gr.vector_source_b(data)
        seedsrc = gr.vector_source_i(seed)
        enc = raw.scrambler_bb(length, 0)
        dec = raw.scrambler_bb(length, 0)
        dst = gr.vector_sink_b()
        encoded = gr.vector_sink_b()
        self.tb.connect(src, enc, dec, dst)
        self.tb.connect(seedsrc, (enc,1))
        self.tb.connect(seedsrc, (dec,1))
        self.tb.connect(enc, encoded)
        self.tb.run()
        #print "data = ", data
        #print "unpacked = ", unpacked.data()
        #print "encoded = ", encoded.data()
        #print "decoded = ", decoded.data()
        #print "dst = ", dst.data()
        self.assertNotEqual(data, encoded.data())
        self.assertEqual(data, dst.data())
        self.assertEqual(data, dst.data())

    def test_001_scrambler (self):
        self.scrambletest()

    #def test_002_scrambler (self):
        #src = gr.file_source(gr.sizeof_char, "/tmp/tx-rsintrlv.datb")
        #seedsrc = gr.file_source(gr.sizeof_int, "/tmp/tx-id.dati")
        #enc = raw.scrambler_bb(16, 0)
        #dst = gr.file_sink(gr.sizeof_char, "/tmp/tx-scramble-check.datb")
        #self.tb.connect(src, enc, dst)
        #self.tb.connect(seedsrc, (enc,1))
        #self.tb.run()

    #def test_003_scrambler (self):
        #src = gr.file_source(gr.sizeof_char, "/tmp/rx-scramble.datb")
        #seedsrc = gr.file_source(gr.sizeof_int, "/tmp/rx-id.dati")
        #dec = raw.scrambler_bb(16, 0)
        #dst = gr.file_sink(gr.sizeof_char, "/tmp/rx-rsintrlv-check.datb")
        #self.tb.connect(src, dec, dst)
        #self.tb.connect(seedsrc, (dec,1))
        #self.tb.run()

if __name__ == '__main__':
    gr_unittest.main ()
