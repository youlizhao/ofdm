#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr
from gnuradio import eng_notation
from gnuradio.eng_option import eng_option
from optparse import OptionParser

import usrp2
import ofdm_rxtx

from uhd_interface import uhd_receiver

class my_top_block(gr.top_block):
  """
    Three modes of operation:
      usrp -> outfile
      infile -> rxdata
      usrp -> rxdata
  """
  def __init__(self, options):
    gr.top_block.__init__(self)

    if options.rx_freq is not None:
      u = uhd_receiver(options.args,
                       options.bandwidth,
                       options.rx_freq, options.rx_gain,
                       options.spec, options.antenna,
                       options.verbose)
    elif options.infile is not None:
      u = gr.file_source(gr.sizeof_gr_complex, options.infile)
    else:
      import sys
      sys.stderr.write("--freq or --infile must be specified\n")
      raise SystemExit

    self.scope = None

    if options.outfile is not None:
      rx = gr.file_sink(gr.sizeof_gr_complex, options.outfile)
    else:
      rx = ofdm_rxtx.RX(options)
      data_tones = rx.params.data_tones
      if options.rxdata is not None:
        if options.rxdata == '.':
          import scope
          # scope it out
          rxs = gr.vector_to_stream(gr.sizeof_gr_complex, data_tones)
          self.connect(rx, rxs)
          self.scope = scope.scope(self, rxs, 'Frame SNR', isComplex=True)
        else:
          if options.char > 0:
            # rail and scale
            self.connect(rx,
                         gr.vector_to_stream(gr.sizeof_float, data_tones * 2),
                         gr.multiply_const_ff(128.0 * (2**0.5)/ options.char),
                         gr.rail_ff(-128.0, 127.0),
                         gr.float_to_char(),
                         gr.file_sink(gr.sizeof_char, options.rxdata))
          else:
            self.connect(rx, gr.file_sink(data_tones * gr.sizeof_gr_complex, options.rxdata))

      if options.snrdata is not None:
        # select one of the snr modes
        snr = ofdm_rxtx.SNR(rx.params.data_tones, options.size, mode=options.snrmode)
        if options.char > 0:
          # NOTE: we use repeat, assuming the file is long enough or properly aligned
          data = gr.stream_to_vector(gr.sizeof_float, data_tones * 2)
          self.connect(gr.file_source(gr.sizeof_char, options.txdata, repeat=True),
                      gr.char_to_float(),
                      gr.multiply_const_ff(options.char * (2**-0.5) / 128.0),
                      data)
        else:
          data = ofdm_rxtx.make_data(rx.params.data_tones, options.size, options.txdata)
        self.connect(rx, (snr,0))
        self.connect(data, (snr,1))
        if options.snrdata == '-':
          # print it out
          msgq = gr.msg_queue(16)
          self.connect(snr, gr.message_sink(gr.sizeof_float, msgq, True))
          self.watcher = ofdm_rxtx.queue_watcher(msgq)
        elif options.snrdata == '.':
          import scope
          # scope it out
          self.scope = scope.scope(self, snr, 'Frame SNR')
        else:
          self.connect(snr, gr.file_sink(gr.sizeof_float, options.snrdata))
      else:
        pass
        #self.connect(rx, gr.null_sink(symbol_size)) # XXX do we still need this?

    self.connect(u, rx)


  def add_options(normal, expert):
    normal.add_option("", "--infile", type="string",
                      help="select input file")
    normal.add_option("", "--outfile", type="string",
                      help="select output file (raw)")
    normal.add_option("", "--txdata", type="string",
                      help="source data file")
    normal.add_option("", "--rxdata", type="string",
                      help="data file (demodulated)")
    normal.add_option("", "--char", type="eng_float", default=0, metavar="CAMPL",
                      help="output is char data that should be scaled by CAMPL/128: [default=%default]")
    normal.add_option("", "--snrdata", type="string",
                      help="per packet snr data file (- print out, . scope out)")
    normal.add_option("", "--snrmode", type="int", default=0,
                      help="0 - per symbol, 1 - per packet, 2 - per bin [default=%default]")
    normal.add_option("-v", "--verbose", action="store_true", default=False)
    normal.add_option("-W", "--bandwidth", type="eng_float",
                          default=500e3,
                          help="set symbol bandwidth [default=%default]")
    expert.add_option("", "--log", action="store_true", default=False,
                      help="Log all parts of flow graph to files (CAUTION: lots of data)")
    uhd_receiver.add_options(normal)
    #usrp2.add_options(normal)
    ofdm_rxtx.RX.add_options(normal, expert)
  # Make a static method to call before instantiation
  add_options = staticmethod(add_options)


# /////////////////////////////////////////////////////////////////////////////
#                                   main
# /////////////////////////////////////////////////////////////////////////////

def main():
  import signal
  def quit_gracefully(signum, frame):
    raise KeyboardInterrupt, "Signal handler"
  signal.signal(signal.SIGINT, quit_gracefully)

  parser = OptionParser(option_class=eng_option, conflict_handler="resolve")
  expert_grp = parser.add_option_group("Expert")

  my_top_block.add_options(parser, expert_grp)

  (options, args) = parser.parse_args ()

  # build the graph
  tb = my_top_block(options)

  r = gr.enable_realtime_scheduling()
  if r != gr.RT_OK:
    print "Warning: failed to enable realtime scheduling"

  if tb.scope is not None:
    tb.scope()
  else:
    try:
      tb.start()                      # start flow graph
    except KeyboardInterrupt:
      tb.stop()
    tb.wait()                       # wait for it to finish

if __name__ == '__main__':
  try:
    main()
  except KeyboardInterrupt:
    pass

