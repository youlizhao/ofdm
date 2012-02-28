#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr
from gnuradio import eng_notation
from gnuradio.eng_option import eng_option
from optparse import OptionParser

from uhd_interface import uhd_transmitter
import ofdm_rxtx


class my_top_block(gr.top_block):
  """
    Three modes of operation:
      txdata -> outfile
      infile -> usrp
      txdata -> usrp
  """
  def __init__(self, options):
    gr.top_block.__init__(self)

    if options.tx_freq is not None:
      u = uhd_transmitter(options.args,
                          options.bandwidth,
                          options.tx_freq, options.tx_gain,
                          options.spec, options.antenna,
                          options.external, options.verbose)
    elif options.outfile is not None:
      u = gr.file_sink(gr.sizeof_gr_complex, options.outfile)
    else:
      raise SystemExit("--freq or --outfile must be specified\n")

    if options.infile is not None:
      tx = gr.file_source(gr.sizeof_gr_complex, options.infile, repeat = options.repeat)
    else:
      tx = ofdm_rxtx.TX(options)
      data_tones = tx.params.data_tones
      if options.char > 0:
        # read char from file
        data = gr.stream_to_vector(gr.sizeof_float, data_tones * 2)
        # NOTE: we use repeat, assuming the file is long enough or properly aligned
        self.connect(gr.file_source(gr.sizeof_char, options.txdata, repeat=True),
                     gr.char_to_float(),
                     gr.multiply_const_ff(options.char * (2**-0.5) / 128.0),
                     data)
      else:
        data = ofdm_rxtx.make_data(data_tones, options.size, options.txdata)
        if options.log:
          self.connect(data, gr.file_sink(data_tones * gr.sizeof_gr_complex, 'tx-data.dat'))

      self.connect(data, tx)
      self.sender = ofdm_rxtx.sender_thread(tx, options)

    if options.amp != 1:
      amp = gr.multiply_const_cc(options.amp)
      self.connect(tx, amp, u)
    else:
      self.connect(tx, u)

  def add_options(normal, expert):
    """
    Adds usrp-specific options to the Options Parser
    """
    normal.add_option("", "--infile", type="string",
                      help="select input file to TX from")
    normal.add_option("", "--outfile", type="string",
                      help="select output file to modulate to")
    normal.add_option("", "--txdata", type="string",
                      help="select data file")
    normal.add_option("", "--char", type="eng_float", default=0, metavar="CAMPL",
                      help="input is char file that should be scaled by CAMPL/128: [default=%default]")
    normal.add_option("", "--amp", type="eng_float", default=1, metavar="AMPL",
                      help="set transmitter digital amplifier: [default=%default]")
    normal.add_option("-v", "--verbose", action="store_true", default=False)
    normal.add_option("", "--repeat", action="store_true", default=False)
    normal.add_option("-W", "--bandwidth", type="eng_float",
                          default=500e3,
                          help="set symbol bandwidth [default=%default]")
    expert.add_option("", "--log", action="store_true", default=False,
                      help="Log all parts of flow graph to files (CAUTION: lots of data)")
    uhd_transmitter.add_options(normal)
    ofdm_rxtx.TX.add_options(normal, expert)
    ofdm_rxtx.sender_thread.add_options(normal)
  # Make a static method to call before instantiation
  add_options = staticmethod(add_options)

# /////////////////////////////////////////////////////////////////////////////
#                                   main
# /////////////////////////////////////////////////////////////////////////////

def main():

  parser = OptionParser(option_class=eng_option, conflict_handler="resolve")
  expert_grp = parser.add_option_group("Expert")

  my_top_block.add_options(parser, expert_grp)

  (options, args) = parser.parse_args()

  # build the graph
  tb = my_top_block(options)

  r = gr.enable_realtime_scheduling()
  if r != gr.RT_OK:
    print "Warning: failed to enable realtime scheduling"

  tb.start()                       # start flow graph

  if options.infile is None:
    tb.sender.run()

  tb.wait()                       # wait for it to finish

if __name__ == '__main__':
  try:
    main()
  except KeyboardInterrupt:
    pass

