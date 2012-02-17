#!/usr/bin/env python
#
# Copyright 2006,2007,2011 Free Software Foundation, Inc.
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
# 

#
# modify by youlizhao.nju@gmail.com
# setup a Tx path and Rx path at the same time
#

from gnuradio import gr, blks2
from gnuradio import eng_notation
from gnuradio.eng_option import eng_option
from optparse import OptionParser

from gnuradio import digital

# from current dir
from transmit_path import transmit_path
from receive_path import receive_path
from uhd_interface import uhd_transmitter
from uhd_interface import uhd_receiver

import struct, sys, time

class my_top_block(gr.top_block):
    def __init__(self, callback, options):
        gr.top_block.__init__(self)

	### Rx Side ###

        if(options.rx_freq is not None):
            self.source = uhd_receiver(options.args_rx,
                                       options.bandwidth,
                                       options.rx_freq, options.rx_gain,
                                       options.spec, options.antenna,
                                       options.verbose)
        elif(options.from_file is not None):
            self.source = gr.file_source(gr.sizeof_gr_complex, options.from_file)
        else:
            self.source = gr.null_source(gr.sizeof_gr_complex)


        # Set up receive path
        # do this after for any adjustments to the options that may
        # occur in the sinks (specifically the UHD sink)
        self.rxpath = receive_path(callback, options)

        
	## Tx Side ###
	if(options.tx_freq is not None):
            self.sink = uhd_transmitter(options.args_tx,
                                        options.bandwidth,
                                        options.tx_freq, options.tx_gain,
                                        options.spec, options.antenna,
                                        options.verbose)
        elif(options.to_file is not None):
            self.sink = gr.file_sink(gr.sizeof_gr_complex, options.to_file)
        else:
            self.sink = gr.null_sink(gr.sizeof_gr_complex)


        # do this after for any adjustments to the options that may
        # occur in the sinks (specifically the UHD sink)
        self.txpath = transmit_path(options)

	# if you use two USRPs and want to synchonized
	# need to change uhd_interface.py
#	self.source.config_mimo()
#	time.sleep(1)	# to make sync stable

	if options.debug:
	    self.connect(self.source, gr.file_sink(gr.sizeof_gr_complex, 'rx.dat'))	# Save reception signal 
	else:
	    self.connect(self.source, self.rxpath)

        self.connect(self.txpath, self.sink)

	if(options.verbose):
            self._print_verbage()

    def carrier_sensed_value(self):
        """
        Return True if the receive path thinks there's carrier
        """
        return self.rxpath.carrier_sensed_value()


    def _print_verbage(self):
        """
        Prints information about the UHD transmitter
        """
        print "\nUHD Transmitter:"
        print "UHD Args:    %s"    % (self.sink._args)
        print "Freq:        %sHz"  % (eng_notation.num_to_str(self.sink._freq))
        print "Gain:        %f dB" % (self.sink._gain)
        print "Sample Rate: %ssps" % (eng_notation.num_to_str(self.sink._rate))
        print "Antenna:     %s"    % (self.sink._ant)
        print "Subdev Sec:  %s"    % (self.sink._spec)   

	print "\nUHD Receiver:"
        print "UHD Args:    %s"    % (self.source._args)
        print "Freq:        %sHz"  % (eng_notation.num_to_str(self.source._freq))
        print "Gain:        %f dB" % (self.source._gain)
        print "Sample Rate: %ssps" % (eng_notation.num_to_str(self.source._rate))
        print "Antenna:     %s"    % (self.source._ant)
        print "Subdev Sec:  %s"    % (self.source._spec) 

# /////////////////////////////////////////////////////////////////////////////
#                                   main
# /////////////////////////////////////////////////////////////////////////////

def main():

    global n_rcvd, n_right
        
    n_rcvd = 0
    n_right = 0

#    def rx_callback(ok, payload):
#        global n_rcvd, n_right
#        n_rcvd += 1
#        (pktno,) = struct.unpack('!H', payload[0:2])
#        if ok:
#            n_right += 1
#        print "ok: %r \t pktno: %d \t n_rcvd: %d \t n_right: %d" % (ok, pktno, n_rcvd, n_right)

#        if 0:
#            printlst = list()
#            for x in payload[2:]:
#                t = hex(ord(x)).replace('0x', '')
#                if(len(t) == 1):
#                    t = '0' + t
#                printlst.append(t)
#            printable = ''.join(printlst)

#            print printable
#            print "\n"
    def rx_callback(ok, payload, secs, frac_secs):
        global n_rcvd, n_right
        n_rcvd += 1
        (pktno,) = struct.unpack('!H', payload[0:2])
        if ok:
            n_right += 1
        print "timestamp: %f \t ok: %r \t pktno: %d \t n_rcvd: %d \t n_right: %d" % (secs+frac_secs, ok, pktno, n_rcvd, n_right)

        if 0:
            printlst = list()
            for x in payload[2:]:
                t = hex(ord(x)).replace('0x', '')
                if(len(t) == 1):
                    t = '0' + t
                printlst.append(t)
            printable = ''.join(printlst)

            print printable
            print "\n"


    def send_pkt(payload='', eof=False):
        return tb.txpath.send_pkt(payload, eof)

    parser = OptionParser(option_class=eng_option, conflict_handler="resolve")
    expert_grp = parser.add_option_group("Expert")

    # for tx and rx
    parser.add_option("-s", "--size", type="eng_float", default=400,
                      help="set packet size [default=%default]")
    parser.add_option("-M", "--megabytes", type="eng_float", default=1.0,
                      help="set megabytes to transmit [default=%default]")
    parser.add_option("","--discontinuous", action="store_true", default=False,
                      help="enable discontinuous")
    parser.add_option("-v", "--verbose", action="store_true",
                      default=False)
    parser.add_option("-d", "--debug", action="store_true",
                      default=False)
    parser.add_option("", "--log", action="store_true",
                      default=False,
                      help="Log all parts of flow graph to file (CAUTION: lots of data)")
    parser.add_option("", "--spec", type="string", default=None,
                          help="Subdevice of UHD device where appropriate")
    parser.add_option("-A", "--antenna", type="string", default=None,
                          help="select Rx Antenna where appropriate")
    parser.add_option("-W", "--bandwidth", type="eng_float",
                      default=500e3,
                      help="set symbol bandwidth [default=%default]")

    # for tx
    parser.add_option("","--to-file", default=None,
                      help="Output file for modulated samples")
    parser.add_option("", "--tx-amplitude", type="eng_float",
                      default=0.1, metavar="AMPL",
                      help="set transmitter digital amplitude: 0 <= AMPL < 1.0 [default=%default]")
    parser.add_option("", "--tx-freq", type="eng_float", default=None,
                      help="set transmit frequency to FREQ [default=%default]",
                      metavar="FREQ")
    parser.add_option("", "--tx-gain", type="eng_float", default=15,
                      help="set transmit gain in dB (default is midpoint)")
    parser.add_option("-t", "--args-tx", type="string", default="",
                      help="UHD device address args [default=%default]")

    # for rx
    parser.add_option("","--from-file", default=None,
                      help="input file of samples to demod")
    parser.add_option("", "--rx-freq", type="eng_float", default=None,
                      help="set receive frequency to FREQ [default=%default]",
                      metavar="FREQ")
    parser.add_option("", "--rx-gain", type="eng_float", default=50,
                      help="set receive gain in dB (default is midpoint)")
    parser.add_option("-r", "--args-rx", type="string", default="",
                      help="UHD device address args [default=%default]")


    ###########################################################################
#    transmit_path.add_options(parser, expert_grp)
#    uhd_transmitter.add_options(parser)
    digital.ofdm_mod.add_options(parser, expert_grp)

#    receive_path.add_options(parser, expert_grp)
#    uhd_receiver.add_options(parser)
    digital.ofdm_demod.add_options(parser, expert_grp)

    (options, args) = parser.parse_args ()

    ###########################################################################

    if options.from_file is None or options.to_file is None:
        if options.rx_freq is None or options.tx_freq is None:
            sys.stderr.write("You must specify -f FREQ or --freq FREQ\n")
            parser.print_help(sys.stderr)
            sys.exit(1)

    # build the graph
    tb = my_top_block(rx_callback, options)

    r = gr.enable_realtime_scheduling()
    if r != gr.RT_OK:
        print "Warning: failed to enable realtime scheduling"

    tb.start()                      # start flow graph
    time.sleep(5)		    # setup all blocks

    ###########################################################################

    # generate and send packets
    nbytes = int(1e6 * options.megabytes)
    n = 0
    pktno = 0
    pkt_size = int(options.size)

    while n < nbytes:
        if options.from_file is None:
            data = (pkt_size - 2) * chr(pktno & 0xff) 
        else:
            data = source_file.read(pkt_size - 2)
            if data == '':
                break;

        payload = struct.pack('!H', pktno & 0xffff) + data
        send_pkt(payload)
        n += len(payload)
        sys.stderr.write('.')
        if options.discontinuous and pktno % 5 == 4:
            time.sleep(1)
        pktno += 1
        
    send_pkt(eof=True)
    """
    
    counter = 1
    while counter < 10:
	print "RX: carrier sense value", tb.carrier_sensed_value()
	counter = counter + 1
	time.sleep(1)
	payload = '10'
	send_pkt(payload)
	print "TX: transmit value", payload
    """
    ###########################################################################

    tb.wait()                       # wait for it to finish

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
