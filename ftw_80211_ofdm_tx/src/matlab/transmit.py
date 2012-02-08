#!/usr/bin/env python
#
# Copyright 2005, 2006 Free Software Foundation, Inc.
# 
# This file is part of GNU Radio
# 
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

from gnuradio import gr
from gnuradio import usrp2
from gnuradio.eng_option import eng_option
from optparse import OptionParser
import sys

class top_block(gr.top_block):

	def __init__(self, options):
                gr.top_block.__init__(self)
		
		self.file_source = gr.file_source(gr.sizeof_gr_complex*1, "matlabframe.dat", options.repeat)
		self.usrp2_sink = usrp2.sink_32fc(options.interface, options.mac_addr)
		self.usrp2_sink.set_interp(options.interp)
		self.usrp2_sink.set_center_freq(options.freq)
		self.usrp2_sink.set_gain(options.gain)
		self.connect((self.file_source, 0), (self.usrp2_sink, 0))

def main():
	parser = OptionParser(option_class=eng_option, conflict_handler="resolve")

	parser.add_option("-e", "--interface", type="string", default="eth0",
                          help="select Ethernet interface, default is eth0")

	parser.add_option("-m", "--mac-addr", type="string", default="",
                          help="select USRP by MAC address, default is auto-select")

	parser.add_option('-f', '--freq', type="eng_float",
                          default = 2.412e9, help="set Tx and/or Rx frequency to FREQ [default=%default]",
                          metavar="FREQ")
	
	parser.add_option("-i", "--interp", type="intx", default=5,
                          help="set fpga interpolation rate to INTERP [default=%default]") 

	parser.add_option("-g", "--gain", type="intx", default=0,
                          help="set Tx gain [default=%default]")

	parser.add_option("-r", "--repeat", action="store_true", default=False, help="loop frame transmission infinitely")

	(options, args) = parser.parse_args ()

	tb = top_block(options)
	tb.run()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
