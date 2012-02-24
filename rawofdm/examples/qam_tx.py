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
import qam_rxtx
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

    if options.freq is not None:
      u = usrp2.sink(options)
    elif options.outfile is not None:
      u = gr.file_sink(gr.sizeof_gr_complex, options.outfile)
    else:
      raise SystemExit("--freq or --outfile must be specified\n")

    if options.infile is not None:
      tx = gr.file_source(gr.sizeof_gr_complex, options.infile)
    else:
      tx = qam_rxtx.TX(options)
      framebytes = tx.framebytes
      if options.txdata is not None:
        data = gr.file_source(framebytes * gr.sizeof_char, options.txdata, options.repeat)
      else:
        data = qam_rxtx.make_data(framebytes)
        if options.log:
          self.connect(data, gr.file_sink(framebytes * gr.sizeof_char, 'tx-data.datb'))

      self.connect(data, tx)
      self.sender = ofdm_rxtx.sender_thread(tx.ofdm, options)

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
    normal.add_option("", "--amp", type="eng_float", default=1, metavar="AMPL",
                      help="set transmitter digital amplifier: [default=%default]")
    normal.add_option("-v", "--verbose", action="store_true", default=False)
    normal.add_option("", "--repeat", action="store_true", default=False)
    expert.add_option("", "--log", action="store_true", default=False,
                      help="Log all parts of flow graph to files (CAUTION: lots of data)")
    usrp2.add_options(normal)
    qam_rxtx.TX.add_options(normal, expert)
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

