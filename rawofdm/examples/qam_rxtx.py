#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr
import raw

import gnuradio.gr.gr_threading as _threading
import array, struct, time, sys

"""
  fixed size RX/TX blocks mapped from int8 (signed char)
  single packet source/sink for benchmarking

  ofdm_rxtx + qam + conv + punc
"""

import ofdm_rxtx # sender_thread, queue_watcher?

class RX(gr.hier_block2):
  """ wraps ofdm_rxtx.RX + raw.qam_rx """
  def __init__(self, options):

    ofdm = ofdm_rxtx.RX(options)
    qam = raw.qam_rx(options.bitrate, ofdm.params.data_tones, ofdm.size, log=options.log)

    framebytes = qam.framebytes
    if options.crc:
      framebytes = 188

    gr.hier_block2.__init__(self, "QAM_RX",
      gr.io_signature(1,1, gr.sizeof_gr_complex),
      gr.io_signature(1,1, gr.sizeof_char * framebytes))

    self.connect(self,
                 ofdm,
                 gr.vector_to_stream(gr.sizeof_gr_complex, ofdm.params.data_tones),
                 qam)

    if options.crc:
      self.connect(qam,
                   gr.stream_to_vector(gr.sizeof_char, framebytes + 4),
                   raw.crc_dec(framebytes),
                   self)
    else:
      self.connect(qam,
                   gr.stream_to_vector(gr.sizeof_char, qam.framebytes),
                   self)

    self.ofdm = ofdm
    self.qam = qam
    self.framebytes = framebytes

  def add_options(normal, expert):
    normal.add_option("", "--bitrate", type="int", default=1,
                      help="set bitrate index [default=%default]")
    normal.add_option("", "--crc", action="store_true", default=False)
    ofdm_rxtx.RX.add_options(normal, expert)
  add_options = staticmethod(add_options)


class TX(gr.hier_block2):
  """ wraps ofdm_rxtx.TX + adds QAM+conv """
  def __init__(self, options):

    ofdm = ofdm_rxtx.TX(options)
    qam = raw.qam_tx(options.bitrate, ofdm.params.data_tones, ofdm.size, log=options.log)

    framebytes = qam.framebytes
    if options.crc:
      framebytes = 188

    gr.hier_block2.__init__(self, "TX",
      gr.io_signature(1,1, gr.sizeof_char * framebytes),
      gr.io_signature(1,1, gr.sizeof_gr_complex))

    qam_in = gr.vector_to_stream(gr.sizeof_char, qam.framebytes)

    if options.crc:
      self.connect(self,
                   raw.crc_enc(framebytes),
                   gr.vector_to_stream(gr.sizeof_char, framebytes + 4),
                   qam)
    else:
      self.connect(self,
                   gr.vector_to_stream(gr.sizeof_char, qam.framebytes),
                   qam)

    self.connect(qam,
                 gr.stream_to_vector(gr.sizeof_gr_complex, ofdm.params.data_tones),
                 ofdm,
                 self)

    self.ofdm = ofdm
    self.qam = qam
    self.framebytes = framebytes

  def add_options(normal, expert):
    normal.add_option("", "--bitrate", type="int", default=1,
                      help="set bitrate index [default=%default]")
    normal.add_option("", "--crc", action="store_true", default=False)
    ofdm_rxtx.TX.add_options(normal, expert)
  add_options = staticmethod(add_options)


def make_data(count):
  import random, cmath, math, numpy
  # generate random data
  # return a new source block
  random.seed(78532)
  data = [random.randint(0, 255) for i in range(count)]
  src = gr.vector_source_b(data, repeat=True, vlen=count)
  return src

class BER(gr.hier_block2):
  # ber measuring block
  # ports:
  #     input 0 == rx
  #     input 1 == tx
  #     output 0 == ber (linear scale)
  # arguments:
  #     framebytes = payload size (unpadded)
  #     numframes = number of frames to average over
  #     mode
  # Three available modes:
  #  0 -- ber per packet
  #  1 -- ber per byte in packet
  def __init__(self, framebytes, numframes, mode=0):
    gr.hier_block2.__init__(self, "BER",
      gr.io_signature2(2,2, framebytes, framebytes),
      gr.io_signature(1,1, gr.sizeof_float))

    xor = gr.xor_bb()
    self.connect((self,0), gr.vector_to_stream(gr.sizeof_char, framebytes), (xor,0))
    self.connect((self,1), gr.vector_to_stream(gr.sizeof_char, framebytes), (xor,1))

    if mode == 0:
      # one ber per packet
      ber = raw.ber_avg(1, framebytes)
      self.connect(xor,
                   ber)
    elif mode == 1:
      # one ber per byte in packet
      ber = raw.ber_avg(framebytes, numframes)
      self.connect(xor,
                   gr.stream_to_vector(gr.sizeof_char, framebytes),
                   ber)

    self.connect(ber, self)

