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

import copy, math, sys
from gnuradio import gr, gru, ftw, eng_notation
import gnuradio.gr.gr_threading as _threading
from gnuradio.digital import psk, qam
import ftw_packet_utils as ofdm_packet_utils

# sets up the transmit path
class ftw_transmit_path(gr.hier_block2): 
    def __init__(self, options, payload=''):
	gr.hier_block2.__init__(self, "transmit_path",
				gr.io_signature(0, 0, 0), # Input signature
				gr.io_signature(1, 1, gr.sizeof_gr_complex)) # Output signature

	# the modulator itself
	self.ofdm_tx       = ofdm_mod(options, payload, msgq_limit=2, pad_for_usrp=False)

        # setup basic connections
        self.connect(self.ofdm_tx, self)
        
    def send_pkt(self, payload='', eof=False):
        return self.ofdm_tx.send_pkt(payload, eof)

# main modulator class
class ofdm_mod(gr.hier_block2):
    """
    Modulates an OFDM stream. Based on the options fft_length, occupied_tones, and
    cp_length, this block creates OFDM symbols using a specified modulation option.
    
    Send packets by calling send_pkt
    """
    def __init__(self, options, payload='', msgq_limit=2, pad_for_usrp=False):
        """
	Hierarchical block for sending packets

        Packets to be sent are enqueued by calling send_pkt.
        The output is the complex modulated signal at baseband.

        @param options: pass modulation options from higher layers (fft length, occupied tones, etc.)
        @param msgq_limit: maximum number of messages in message queue
        @type msgq_limit: int
        @param pad_for_usrp: If true, packets are padded such that they end up a multiple of 128 samples
        """

	gr.hier_block2.__init__(self, "ofdm_mod",
				gr.io_signature(0, 0, 0),       # Input signature
				gr.io_signature(1, 1, gr.sizeof_gr_complex)) # Output signature
	
    
        self._fft_length          = 64
        self._total_sub_carriers  = 53
	self._data_subcarriers    = 48
	self._cp_length           = 16
 	self._regime              = options.regime
	self._symbol_length       = self._fft_length + self._cp_length
        self._role                = options.role
	
	# assuming we have 100Ms/s going to the USRP2 and 80 samples per symbol
	# we can calculate the OFDM symboltime (in microseconds) 
	# depending on the interpolation factor 
	self._symbol_time	  = options.interp*(self._fft_length+self._cp_length)/100
	
        win = []

	if(self._regime == "1" or self._regime == "2"):
	    rotated_const = ofdm_packet_utils.bpsk(self)
        
        elif (self._regime == "3" or self._regime == "4"):	
	    rotated_const = ofdm_packet_utils.qpsk(self)

        elif(self._regime == "5" or self._regime == "6"):
            rotated_const = ofdm_packet_utils.qam16(self) 

        elif(self._regime == "7" or self._regime == "8"):
            rotated_const = ofdm_packet_utils.qam64(self)
	
        # map groups of bits to complex symbols
        self._pkt_input = ftw.ofdm_mapper(rotated_const, msgq_limit, self._data_subcarriers, self._fft_length)
        
        # insert pilot symbols (use pnc block * by lzyou)
        if self._role == 'A':
            print " >>> [FPNC]: *A* Insert Pilot"
            self.pilot = ftw.pnc_ofdm_pilot_cc(self._data_subcarriers, 1)
        elif self._role == 'B':
            print " >>> [FPNC]: *B* Insert Pilot"
            self.pilot = ftw.pnc_ofdm_pilot_cc(self._data_subcarriers, 2)
        else:
            print " >>> [FTW ]: Insert Pilot"
            self.pilot = ftw.ofdm_pilot_cc(self._data_subcarriers)
        # just for test
        #self.pilot = ftw.pnc_ofdm_pilot_cc(self._data_subcarriers, 1)
        #self.pilot = ftw.pnc_ofdm_pilot_cc(self._data_subcarriers, 2)
	
        # move subcarriers to their designated place and insert DC  
        self.cmap  = ftw.ofdm_cmap_cc(self._fft_length, self._total_sub_carriers)        

	# inverse fast fourier transform
        self.ifft = gr.fft_vcc(self._fft_length, False, win, False)

        # add cyclic prefix
	from gnuradio import digital
        self.cp_adder = digital.ofdm_cyclic_prefixer(self._fft_length, self._symbol_length)
        self.connect(gr.null_source(gr.sizeof_char), (self.cp_adder, 1))  # Note: dirty modification to accomdate the API change
        
        # scale accordingly
        self.scale = gr.multiply_const_cc(1.0 / math.sqrt(self._fft_length))
        
	# we need to know the number of OFDM data symbols for preamble and zerogap
	info = ofdm_packet_utils.get_info(payload, options.regime, self._symbol_time)	
	N_sym             = info["N_sym"]
	
	# add training sequence (modify by lzyou)
        if self._role == 'A':
            print " >>> [FPNC]: *A* Insert Preamble"
            self.preamble= ofdm_packet_utils.insert_preamble(self._symbol_length, N_sym, 'A')
        elif self._role == 'B':
            print " >>> [FPNC]: *B* Insert Preamble"
            self.preamble= ofdm_packet_utils.insert_preamble(self._symbol_length, N_sym, 'B')
        else:
            print " >>> [FTW ]: Insert Preamble"
            self.preamble= ofdm_packet_utils.insert_preamble(self._symbol_length, N_sym)        

        # append zero samples at the end (receiver needs that to decode)
        if self._role == None:
            print " >>> [FTW ]: Insert Zerogap"
	    self.zerogap    = ofdm_packet_utils.insert_zerogap(self._symbol_length, N_sym)
        else:
            print " >>> [FPNC]: Insert Zerogap"
            self.zerogap    = ofdm_packet_utils.insert_zerogap(self._symbol_length, N_sym, 'FPNC')

	self.s2v = gr.stream_to_vector(gr.sizeof_gr_complex , self._symbol_length)
	self.v2s = gr.vector_to_stream(gr.sizeof_gr_complex , self._symbol_length)
	
	# swap real and immaginary component before sending (GNURadio/USRP2 bug!)
	if options.swapIQ == True:
		self.gr_complex_to_imag_0 = gr.complex_to_imag(1)
		self.gr_complex_to_real_0 = gr.complex_to_real(1)
		self.gr_float_to_complex_0 = gr.float_to_complex(1)
		self.connect((self.v2s, 0), (self.gr_complex_to_imag_0, 0))
		self.connect((self.v2s, 0), (self.gr_complex_to_real_0, 0))
		self.connect((self.gr_complex_to_real_0, 0), (self.gr_float_to_complex_0, 1))
		self.connect((self.gr_complex_to_imag_0, 0), (self.gr_float_to_complex_0, 0))
		self.connect((self.gr_float_to_complex_0, 0), (self))
	elif options.swapIQ == False:
		self.gr_complex_to_imag_0 = gr.complex_to_imag(1)
		self.gr_complex_to_real_0 = gr.complex_to_real(1)
		self.gr_float_to_complex_0 = gr.float_to_complex(1)
		self.connect((self.v2s, 0), (self.gr_complex_to_imag_0, 0))
		self.connect((self.v2s, 0), (self.gr_complex_to_real_0, 0))
		self.connect((self.gr_complex_to_imag_0, 0), (self.gr_float_to_complex_0, 1))
		self.connect((self.gr_complex_to_real_0, 0), (self.gr_float_to_complex_0, 0))
		self.connect((self.gr_float_to_complex_0, 0), (self))
		
        # connect the blocks
	self.connect((self._pkt_input, 0), (self.pilot, 0))
	self.connect((self._pkt_input,1), (self.preamble, 1))
	self.connect((self.preamble,1), (self.zerogap, 1))
	
	self.connect(self.pilot, self.cmap, self.ifft, self.cp_adder, self.scale, self.s2v, self.preamble, self.zerogap, self.v2s)

        if options.log:
            self.connect((self._pkt_input), gr.file_sink(gr.sizeof_gr_complex * self._data_subcarriers, "ofdm_mapper.dat"))
	    self.connect(self.pilot, gr.file_sink(gr.sizeof_gr_complex * (5 + self._data_subcarriers), "ofdm_pilot.dat"))
	    self.connect(self.cmap, gr.file_sink(gr.sizeof_gr_complex * self._fft_length, "ofdm_cmap.dat"))	
            self.connect(self.ifft, gr.file_sink(gr.sizeof_gr_complex * self._fft_length, "ofdm_ifft.dat"))
            self.connect(self.cp_adder, gr.file_sink(gr.sizeof_gr_complex, "ofdm_cp_adder.dat"))	   
	    self.connect(self.scale, gr.file_sink(gr.sizeof_gr_complex, "ofdm_scale.dat"))
            self.connect(self.preamble, gr.file_sink(gr.sizeof_gr_complex * self._symbol_length, "ofdm_preamble.dat"))
            self.connect(self.zerogap, gr.file_sink(gr.sizeof_gr_complex * self._symbol_length, "ofdm_zerogap.dat"))

    def send_pkt(self, payload='', eof=False):
        """
        Send the payload.

        @param payload: data to send
        @type payload: string
        """
        if eof:
            msg = gr.message(1)              # tell self._pkt_input we're not sending any more packets
        else: 
            info = ofdm_packet_utils.get_info(payload, self._regime, self._symbol_time)
	    N_cbps            = info["N_cbps"]
	    N_bpsc            = info["N_bpsc"]
            N_rate            = info["rate"]
	    N_sym             = info["N_sym"]

	    (pkt,Length) = ofdm_packet_utils.ftw_make(payload,self._regime, self._symbol_time)
	    (pkt_scrambled,Length) = ofdm_packet_utils.scrambler(pkt,Length)	
#            print
#            print conv_packed_binary_string_to_1_0_string(pkt_scrambled)	
	    pkt_coded = ofdm_packet_utils.conv_encoder(pkt_scrambled, Length, self._regime, N_cbps, N_bpsc, N_sym, N_rate)
#            print
#            print conv_packed_binary_string_to_1_0_string(pkt_coded)
	    pkt_interleaved = ofdm_packet_utils.interleaver(pkt_coded , self._regime, N_cbps, N_bpsc)
#            print
#            print conv_packed_binary_string_to_1_0_string(pkt_interleaved)
	    msg = gr.message_from_string(pkt_interleaved) 

        self._pkt_input.msgq().insert_tail(msg)

def conv_packed_binary_string_to_1_0_string(s):
    """
    '\xAF' --> '10101111'
    """
    r = []
    for ch in s:
        x = ord(ch)
        for i in range(7,-1,-1):
            t = (x >> i) & 0x1
            r.append(t)

    return ''.join(map(lambda x: chr(x + ord('0')), r))
