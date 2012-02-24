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
      usrp -> outfile
      infile -> rxdata
      usrp -> rxdata
  """
  def __init__(self, options):
    gr.top_block.__init__(self)

    if options.freq is not None:
      u = usrp2.source(options)
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
      rx = qam_rxtx.RX(options)
      framebytes = rx.framebytes
      if options.rxdata is not None:
        if options.rxdata == '-':
          self.connect(rx, gr.file_descriptor_sink(gr.sizeof_char * framebytes, 1))
        else:
          self.connect(rx, gr.file_sink(gr.sizeof_char * framebytes, options.rxdata))

      if options.berdata is not None:
        # select one of the ber modes
        ber = qam_rxtx.BER(framebytes, 100, mode=options.bermode)
        data = qam_rxtx.make_data(framebytes)
        self.connect(rx, (ber,0))
        self.connect(data, (ber,1))
        if options.berdata == '-':
          # print it out
          msgq = gr.msg_queue(16)
          self.connect(ber, gr.message_sink(gr.sizeof_float, msgq, True))
          self.watcher = ofdm_rxtx.queue_watcher(msgq)
        elif options.berdata == '.':
          import scope
          # scope it out
          self.scope = scope.scope(self, ber, 'Frame BER')
        else:
          self.connect(ber, gr.file_sink(gr.sizeof_float, options.berdata))
      else:
        pass
        #self.connect(rx, gr.null_sink(symbol_size)) # XXX do we still need this?

    self.connect(u, rx)
    #self.connect(u, gr.file_sink(gr.sizeof_gr_complex, 'RXdiag.dat'))


  def add_options(normal, expert):
    normal.add_option("", "--infile", type="string",
                      help="select input file")
    normal.add_option("", "--outfile", type="string",
                      help="select output file (raw)")
    normal.add_option("", "--txdata", type="string",
                      help="source data file")
    normal.add_option("", "--rxdata", type="string",
                      help="data file (demodulated)")
    normal.add_option("", "--berdata", type="string",
                      help="per packet snr data file (- print out, . scope out)")
    normal.add_option("", "--bermode", type="int", default=0,
                      help="0 - per packet, 1 - per byte [default=%default]")
    normal.add_option("-v", "--verbose", action="store_true", default=False)
    expert.add_option("", "--log", action="store_true", default=False,
                      help="Log all parts of flow graph to files (CAUTION: lots of data)")
    usrp2.add_options(normal)
    qam_rxtx.RX.add_options(normal, expert)
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

  #import os
  #print 'Blocked waiting for GDB attach (pid = %d)' % (os.getpid(),)
  #raw_input ('Press Enter to continue: ')

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
    #tb.watcher.done()

if __name__ == '__main__':
  try:
    main()
  except KeyboardInterrupt:
    pass

