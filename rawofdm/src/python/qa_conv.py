#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr, gr_unittest
import __init__ as raw
import random
import itertools

def wait_for_gdb():
    import os
    print 'Blocked waiting for GDB attach (pid = %d)' % (os.getpid(),)
    raw_input ('Press Enter to continue: ')

def bitCount(int_type):
    count = 0
    while(int_type):
        int_type &= int_type - 1
        count += 1
    return(count)

class qa_convenc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_enc (self):
        length = 2048 # DANGER: this fails for > 2048 for some reason
        data = [ random.randint(0,255) for i in range(length) ]
        # last K-1 bits are padding, meaning last byte is lost
        data[-1] = 0
        data = tuple(data)
        src = gr.vector_source_b(data)
        enc = raw.conv_enc()
        tofloat = gr.uchar_to_float()
        offset = gr.multiply_const_ff(255.0)
        touchar = gr.float_to_uchar()
        dec = raw.conv_dec(length*8)
        dst = gr.vector_sink_b()
        self.tb.connect(src,
                        enc,
                        tofloat,
                        offset,
                        touchar,
                        dec,
                        dst)

        self.tb.run()
        #self.assertEqual (data, dst.data())
        nerrors = 0
        i = 0
        for (a,b) in itertools.izip(data, dst.data()):
          nerr = bitCount(a ^ b)
          if nerr:
            print "%g " % (i/2048.), nerr
          nerrors += nerr
          i+= 1
        print "Number or Errors %d BER %g" % (nerrors, (nerrors * 1.0 / (length * 8)) )
        self.assertEqual(nerrors, 0)

    def punctest (self, nc, np):
        length = 2046 # should be divisible by nc
        data = [ random.randint(0,255) for i in range(length) ]
        # last K-1 bits are padding, meaning last byte is lost
        data[-1] = 0
        data = tuple(data)
        src = gr.vector_source_b(data)

        enc = raw.conv_enc()
        dec = raw.conv_dec(length*8)

        punc = raw.conv_punc(nc, np)
        depunc = raw.conv_punc(np, nc, 128)

        tofloat = gr.uchar_to_float()
        offset = gr.multiply_const_ff(255.0)
        touchar = gr.float_to_uchar()

        dst = gr.vector_sink_b()
        rx = gr.vector_sink_b()
        self.tb.connect(src,
                        enc,
                        punc,
                        tofloat,
                        offset,
                        touchar,
                        depunc,
                        dec,
                        dst)
        self.tb.connect(punc, rx)
        self.tb.run()
        rxlen = len(rx.data())
        self.assertEqual (rxlen, (length * 8 * 2 * np)/nc)
        self.assertEqual (data, dst.data())

    def test_002_encpunc (self):
        self.punctest(4, 4) # 1/2 rate

    def test_003_encpunc (self):
        self.punctest(6, 4) # 3/4 rate

    def test_004_encpunc (self):
        self.punctest(4, 3) # 2/3 rate

if __name__ == '__main__':
    gr_unittest.main ()
