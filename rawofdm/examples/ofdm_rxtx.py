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
"""

class RX(gr.hier_block2):
  def __init__(self, options):
    self.rx = raw.ofdm_demod(options, (options.nsr is not None) and 4 or 2)
    self.params = self.rx.params
    symbol_size = self.params.data_tones * gr.sizeof_gr_complex

    gr.hier_block2.__init__(self, "RX",
      gr.io_signature(1,1, gr.sizeof_gr_complex),
      gr.io_signature(1,1, symbol_size))

    ff = raw.fix_frame(symbol_size, options.size)
    self.connect(self, self.rx, (ff,0), self)
    self.connect((self.rx,1), (ff,1))

    # noise-to-signal ratio estimate
    # TODO: make it a proper output from the block
    if options.nsr is not None:
      nsr = gr.divide_ff()
      self.connect((self.rx,2), gr.integrate_ff(options.size), (nsr,0))
      self.connect((self.rx,3), gr.integrate_ff(options.size), (nsr,1))
      self.connect(nsr, gr.file_sink(gr.sizeof_float, options.nsr)) # what to do with the output?

    acq = self.rx.ofdm_recv.frame_acq
    acq.set_min_symbols(options.size)
    if options.numpkts is not None:
      acq.set_num_frames(options.numpkts)

    self.size = options.size

    if options.log:
      self.connect((self.rx,0), gr.file_sink(symbol_size, 'rx-ofdmdemod.dat'))
      self.connect((self.rx,1), gr.file_sink(gr.sizeof_char, 'rx-ofdmdemod.datb'))
      self.connect((ff,0), gr.file_sink(symbol_size, 'rx-fixframe.dat'))

  def add_options(normal, expert):
    normal.add_option("-s", "--size", type="int", default=16,
                      help="set number of symbols per packet [default=%default]")
    normal.add_option("-N", "--numpkts", type="int", default=0,
                      help="set number of packets to expect")
    normal.add_option("", "--nsr", type="string",
                      help="per packet estimated noise-to-signal data file")
    raw.ofdm_params.add_options(normal, expert)
  add_options = staticmethod(add_options)


class TX(gr.hier_block2):
  def __init__(self, options):
    self.tx = raw.ofdm_mod(options)
    self.params = self.tx.params
    symbol_size = self.params.data_tones * gr.sizeof_gr_complex

    gr.hier_block2.__init__(self, "TX",
      gr.io_signature(1,1, symbol_size),
      gr.io_signature(1,1, gr.sizeof_gr_complex))

    self.msg = gr.message_source(gr.sizeof_char, 2)
    self.connect(self, (self.tx, 0), self)
    self.connect(self.msg, (self.tx, 1))

    #self.connect(self.msg, gr.file_sink(gr.sizeof_char, 'tx-msg.datb'));

    self.size = options.size

  def send_packet(self, num=1): # NOTE: we keep this to allow discontinuous operation
    """ send a packet of options.size symbols from the datafile """
    msgq = self.msg.msgq()
    payload = [0] * self.size
    payload[0] = 1
    payload = array.array('B', payload) * num
    msgq.insert_tail(gr.message_from_string(payload.tostring()))

  def send_eof(self):
    # signal EOF
    msgq = self.msg.msgq()
    msgq.insert_tail(gr.message(1))

  def add_options(normal, expert):
    normal.add_option("-s", "--size", type="int", default=16,
                      help="set number of symbols per packet [default=%default]")
    raw.ofdm_params.add_options(normal, expert)
  add_options = staticmethod(add_options)


def make_data(data_tones, num_symbols, filename=None):
  import random, cmath, math, numpy
  # load data from options.txdata
  # otherwise generate random data
  # return a new source block
  count = num_symbols*data_tones
  if filename is not None:
    data = numpy.fromfile(filename, dtype=numpy.complex64, count=count, sep='')
    data = [complex(d) for d in data]
  else:
    random.seed(78532)
    #data = [cmath.exp(1j * 2*math.pi * random.random()) for i in range(count)]
    NN = 4
    sf = 1 #math.sqrt(0.5)
    qpsk = lambda : sf * cmath.exp(1j * (math.pi/NN * (2*random.randint(0, NN-1)+1)))
    off = NN/2-0.5
    qam = lambda : sf/off * ((random.randint(0, NN-1) - off) + 1j*(random.randint(0, NN-1) - off))

    data = [qpsk() for i in range(count)]
  src = gr.vector_source_c(data, repeat=True, vlen=data_tones)
  return src

class SNR(gr.hier_block2):
  # snr measuring block
  # ports:
  #     input 0 == rx
  #     input 1 == tx
  #     output 0 == snr (linear scale)
  # arguments:
  #     data_tones = number of separate averages
  #     num_symbols = number of samples to average
  #     mode
  # Three available modes:
  #  0 -- snr per symbol
  #  1 -- snr per packet
  #  2 -- snr per frequency bin
  def __init__(self, data_tones, num_symbols, mode=0):
    symbol_size = data_tones * gr.sizeof_gr_complex
    gr.hier_block2.__init__(self, "SNR",
      gr.io_signature2(2,2, symbol_size, symbol_size),
      gr.io_signature(1,1, gr.sizeof_float))

    sub = gr.sub_cc(data_tones)
    self.connect((self,0), (sub,0))
    self.connect((self,1), (sub,1))

    err = gr.complex_to_mag_squared(data_tones);
    self.connect(sub, err);
    pow = gr.complex_to_mag_squared(data_tones);
    self.connect((self,1), pow);

    if mode == 0:
      # one snr per symbol (num_symbols is ignored)
      snr = gr.divide_ff()
      self.connect(pow, gr.vector_to_stream(gr.sizeof_float, data_tones),
        gr.integrate_ff(data_tones), (snr,0))
      self.connect(err, gr.vector_to_stream(gr.sizeof_float, data_tones),
        gr.integrate_ff(data_tones), (snr,1))
      out = snr
    elif mode == 1:
      # one snr per packet
      snr = gr.divide_ff()
      self.connect(pow, gr.vector_to_stream(gr.sizeof_float, data_tones),
        gr.integrate_ff(data_tones*num_symbols), (snr,0))
      self.connect(err, gr.vector_to_stream(gr.sizeof_float, data_tones),
        gr.integrate_ff(data_tones*num_symbols), (snr,1))
      out = snr
    elif mode == 2:
      # one snr per frequency bin
      snr = gr.divide_ff(data_tones)
      self.connect(pow, raw.symbol_avg(data_tones, num_symbols), (snr,0))
      self.connect(err, raw.symbol_avg(data_tones, num_symbols), (snr,1))
      out = gr.vector_to_stream(gr.sizeof_float, data_tones)
      self.connect(snr, out)

    self.connect(out, gr.nlog10_ff(10), self)

class sender_thread():
  def add_options(parser):
    parser.add_option("-N", "--numpkts", type="eng_float", default=0,
                      help="set number of packets to transmit [default=%default]")
    parser.add_option("","--pause", type="eng_float", default=0,
                      help="inter-burst pause [s]")
    parser.add_option("","--burst", type="intx", default=1,
                      help="burst length")
  add_options = staticmethod(add_options)

  def __init__(self, tx, options):
    self.tx = tx
    self.pause = options.pause
    self.numpkts = options.numpkts
    self.burst = options.burst

  def run(self):
    start = time.time()
    # generate and send framing indicators
    numpkts = abs(int(self.numpkts))
    if numpkts == 0:
      numpkts = -1 # so that we go forever
    pktno = 0
    while pktno != numpkts:
      try:
        self.tx.send_packet(self.burst)
        pktno+= self.burst
        sys.stdout.write('.')
        if (self.pause > 0.0):
          sys.stdout.flush()
          time.sleep(self.pause)
      except:
        break
    self.tx.send_eof()

    elapsed = time.time() - start
    print "Sent %d packets of %d symbols in %.2f seconds" % (pktno, self.tx.size, elapsed)
    print pktno*self.tx.size / elapsed

class queue_watcher(_threading.Thread):
  def __init__(self, q):
    _threading.Thread.__init__(self)
    self.setDaemon(1)
    self.q = q
    #self.callback = callback
    self.keep_running = True
    self.start()

  def done(self):
    self.keep_running = False

  def run(self):
    count = 0
    size = struct.calcsize('f')
    start = time.time()
    while self.keep_running:
      msg = self.q.delete_head()
      #a = array.array('f')
      s = msg.to_string()
      offset = 0
      while offset < len(s):
        #print "%.4f\t%.3f" % (time.time() - start, struct.unpack_from('f', s, offset)[0])
        print "%.3f" % struct.unpack_from('f', s, offset)[0]
        offset+= size
        count+= 1
      #finish = time.time()
      #print len(s), s
      #data = struct.unpack('f', msg.to_string())
      #a.fromstring(s)
      #self.callback(data)
      #print a
    finish = time.time()
    total = finish - start
    print "Queue: %d/%.2fs = %f" % (count, total, count / total)

