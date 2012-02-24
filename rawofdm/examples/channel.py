#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio import gr
from gnuradio import eng_notation
from gnuradio.eng_option import eng_option
from optparse import OptionParser

import time, sys, random, math

class my_top_block(gr.top_block):
  def __init__(self, options):
    gr.top_block.__init__(self)

    SNR = 10.0**(options.snr/10.0)
    noise_power_in_channel = 1.0/SNR
    noise_voltage = math.sqrt(noise_power_in_channel/2.0)
    print "Noise voltage: ", noise_voltage

    frequency_offset = options.frequency_offset / 64
    print "Frequency offset: ", frequency_offset

    if options.multipath_on:
      taps = [1.0, .2, 0.0, .1, .08, -.4, .12, -.2, 0, 0, 0, .3]
    else:
      taps = [1.0, 0.0]

    self.connect(gr.file_source(gr.sizeof_gr_complex, options.infile),
                 gr.channel_model(noise_voltage, frequency_offset,
                                  options.clockrate_ratio, taps),
                 gr.file_sink(gr.sizeof_gr_complex, options.outfile))

  def add_options(normal, expert):
    """
    Adds usrp-specific options to the Options Parser
    """
    normal.add_option("", "--infile", type="string",
                      help="select input file")
    normal.add_option("", "--outfile", type="string",
                      help="select output file")
    normal.add_option("", "--snr", type="eng_float", default=30,
                      help="set the SNR of the channel in dB [default=%default]")
    normal.add_option("", "--frequency-offset", type="eng_float", default=0,
                      help="set frequency offset introduced by channel [default=%default]")
    normal.add_option("", "--clockrate-ratio", type="eng_float", default=1.0,
                      help="set clock rate ratio (sample rate difference) between two systems [default=%default]")
    normal.add_option("","--multipath-on", action="store_true", default=False,
                      help="enable multipath")
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

  tb.start()                      # start flow graph
  tb.wait()                       # wait for it to finish

if __name__ == '__main__':
  try:
    main()
  except KeyboardInterrupt:
    pass

