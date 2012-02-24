#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr, gr_unittest
import __init__ as raw
import random
import math
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

bitrates = {
  1 : (1, (4, 4)),
  2 : (1, (6, 4)),
  3 : (2, (4, 4)),
  4 : (2, (6, 4)),
  5 : (4, (4, 4)),
  6 : (4, (6, 4)),
  7 : (6, (4, 3)),
  8 : (6, (6, 4))
  }

class fullstack:
  def __init__(self):
    self.tb = gr.top_block ()
    self.qambits = 1
    self.puncparam = (4,4)
    self.nsymbols = 16
    self.nframes = 100
    self.ncarriers = 48

  def setup(self, bitrate):
    (qambits, puncparam) = bitrates[bitrate]
    self.qambits = qambits
    self.puncparam = puncparam


  def test(self, ebno, data=None):
    random.seed(0)
    (nc, np) = self.puncparam
    # round number of bytes and symbols (FIXME, this is probably not always round)
    # one symbol = self.ncarriers * self.qambits * 0.5 * (np / nc) bits
    symbits = self.ncarriers * self.qambits * nc / (2. * np)
    #print "symbits = ", symbits
    assert int(self.nsymbols * symbits) % 8 == 0
    length = int(self.nsymbols * symbits) / 8 # incl padding

    #print 'src: %d' % (length-1)
    #print 'pad: %d' % length
    #print 'fenc: %d' % (length * 8 * 2)
    #print 'punc: %f' % (length * 8 * 2. * np / nc)
    #print 'intrlv: %f' % ((length * 8 * 2. * np / nc) / (self.ncarriers*self.qambits))
    #print 'total bits: %d' % (self.nframes * length * 8 * 2)

    if data is None:
      data = [ random.randint(0,255) for i in range((length-1)*self.nframes) ]
    data = tuple(data)

    src = gr.vector_source_b(data)
    #src = gr.file_source(gr.sizeof_char, "src1.datb");
    dst = gr.vector_sink_b()

    fenc = raw.conv_enc()
    fdec = raw.conv_dec(length*8)

    punc = raw.conv_punc(nc, np)
    depunc = raw.conv_punc(np, nc, 128)

    pad = raw.conv_punc(length-1, length)
    depad = raw.conv_punc(length, length-1)

    intrlv = raw.intrlv_bit(self.ncarriers, self.qambits, False)
    deintrlv = raw.intrlv_bit(self.ncarriers, self.qambits, True)

    qenc = raw.qam_enc(self.qambits)
    qdec = raw.qam_dec(self.qambits)

    SNR = 10.0**(ebno/10.0)
    noise_power_in_channel = 1.0/SNR
    noise_voltage = math.sqrt(noise_power_in_channel/2.0)
    # AWGN only
    noise = gr.noise_source_c(gr.GR_GAUSSIAN, noise_voltage, 0)
    chan = gr.add_cc()
    self.tb.connect(noise, (chan, 1))
    #chan = gr.channel_model(noise_voltage, 0.0, 1.0, [1.0, 0.0]),

    tofloat = gr.uchar_to_float()
    offset = gr.multiply_const_ff(255.0)
    touchar = gr.float_to_uchar()

    self.tb.connect(src,  # length-1
                    pad,  # length
                    fenc, # 2*length
                    punc, # 2*length*np/nc
                    intrlv, # 2*length*np/nc (% ncarriers*qambits)
                      qenc, # 2*length*np/nc / qambits (% ncarriers)
                      chan,
                      qdec,
                      #tofloat, offset, touchar,
                    deintrlv,
                    depunc,
                    fdec,
                    depad,
                    dst)

    #self.tb.connect(src, gr.file_sink(gr.sizeof_char, "src.datb"))
    #self.tb.connect(pad, gr.file_sink(gr.sizeof_char, "pad.datb"))
    #self.tb.connect(fenc, gr.file_sink(gr.sizeof_char, "fenc.datb"))
    #self.tb.connect(punc, gr.file_sink(gr.sizeof_char, "punc.datb"))
    #self.tb.connect(qenc, gr.file_sink(gr.sizeof_gr_complex, "qenc.dat"))
    #self.tb.connect(qdec, gr.file_sink(gr.sizeof_char, "qdec.datb"))
    #self.tb.connect(touchar, gr.file_sink(gr.sizeof_char, "qdec.datb"))
    #self.tb.connect(depunc, gr.file_sink(gr.sizeof_char, "depunc.datb"))
    #self.tb.connect(fdec, gr.file_sink(gr.sizeof_char, "fdec.datb"))
    #self.tb.connect(depad, gr.file_sink(gr.sizeof_char, "depad.datb"))

    self.tb.run()

    nerrors = 0
    for (a,b) in itertools.izip(data, dst.data()):
      count = bitCount(a ^ b)
      nerrors += count
    ber = nerrors*1.0/len(data)/8
    print "%d x %g @ %g dB\t#errors = %d\tBER = %g" % (self.qambits, nc/(2.0*np), ebno, nerrors, ber)
    return nerrors



class qa_fullstack (gr_unittest.TestCase):

    def setUp (self):
      self.s = fullstack()

    def tearDown (self):
      self.s.tb = None

    def longframe(self, bitrate, ebno):
      self.s.setup(bitrate)
      self.s.nframes = 1
      self.s.nsymbols = 800
      nerr = self.s.test(ebno)
      self.assertEqual (nerr, 0)

    def multiframe(self, bitrate, ebno):
      self.s.setup(bitrate)
      self.s.nframes = 300
      self.s.nsymbols = 14
      nerr = self.s.test(ebno)
      self.assertEqual (nerr, 0)

    def test_rate1 (self): # BPSK, 1/2 rate
      self.longframe(1, 3.0)
    def test_rate2 (self): # BPSK, 3/4 rate
      self.longframe(2, 4.0)
    def test_rate3 (self): # QPSK, 1/2 rate
      self.longframe(3, 4.5)
    def test_rate4 (self): # QPSK, 3/4 rate
      self.longframe(4, 7.0)
    def test_rate5 (self): # QAM16, 1/2 rate
      self.longframe(5, 10.5)
    def test_rate6 (self): # QAM16, 3/4 rate
      self.longframe(6, 14.0)
    def test_rate7 (self): # QAM64, 2/3 rate
      self.longframe(7, 17.5)
    def test_rate8 (self): # QAM64, 3/4 rate
      self.longframe(8, 19.5)

    def test_mrate1 (self): # BPSK, 1/2 rate
      self.multiframe(1, 3.0)
    def test_mrate2 (self): # BPSK, 3/4 rate
      self.multiframe(2, 4.5)
    def test_mrate3 (self): # QPSK, 1/2 rate
      self.multiframe(3, 4.5)
    def test_mrate4 (self): # QPSK, 3/4 rate
      self.multiframe(4, 8.0)
    def test_mrate5 (self): # QAM16, 1/2 rate
      self.multiframe(5, 10.5)
    def test_mrate6 (self): # QAM16, 3/4 rate
      self.multiframe(6, 14.0)
    def test_mrate7 (self): # QAM64, 2/3 rate
      self.multiframe(7, 19.0)
    def test_mrate8 (self): # QAM64, 3/4 rate
      self.multiframe(8, 21.0)

    def xtest_1(self):
      self.s.setup(2)
      self.s.nsymbols = 1
      data = [ 140, 3, 184, 102 ]
      nerr = self.s.test(50, data)

    def xtest_2(self):
      self.s.setup(2)
      self.s.nsymbols = 1
      data = [ 140, 3 ]
      nerr = self.s.test(50, data)

if __name__ == '__main__':
    gr_unittest.main ()
