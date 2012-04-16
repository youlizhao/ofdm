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

import struct, numpy, math, sys
from gnuradio import gru,gr,ftw

def get_info(payload, regime, symboltime):

        # MAC header for a UNICAST DATA FRAME 
	mac_framectrl = chr(0x08) + chr(0x00)                                                  # this is a data frame
	mac_duration  = chr(0x00) + chr(0x00)                                                  # dummy, value will be set by script
	mac_address1  = chr(0x00) + chr(0x60) + chr(0x08) + chr(0xcd) + chr(0x37) + chr(0xa6)  # some destination mac-address
	mac_address2  = chr(0x00) + chr(0x20) + chr(0xd6) + chr(0x01) + chr(0x3c) + chr(0xf1)  # some source mac-address
	mac_address3  = chr(0x00) + chr(0x60) + chr(0x08) + chr(0xad) + chr(0x3b) + chr(0xaf)  # some BSSID mac-address
	mac_seqctrl   = chr(0x00) + chr(0x00)                                                  # some sequence and fragment number
	
        # pre-assemble the MPDU (duration is filled with dummy and CRC32 is missing at this point)
	packet = mac_framectrl + mac_duration + mac_address1 + mac_address2 + mac_address3 + mac_seqctrl + payload

        # switch coding rate and constellation according to regime profile
	if (regime == "1"):
		modulation = "bpsk"
		rate = 1/float(2)
		N_cbps = 48
		N_bpsc = 1
	elif(regime == "2"):
		modulation = "bpsk"
		rate = 3/float(4)
		N_cbps = 48
		N_bpsc = 1
	elif(regime == "3"):
		modulation = "qpsk"
		rate = 1/float(2)
		N_cbps = 96
		N_bpsc = 2
	elif(regime == "4"):
		modulation = "qpsk"
		rate = 3/float(4)
		N_cbps = 96
		N_bpsc = 2
	elif(regime == "5"):
		modulation = "qam16"
		rate = 1/float(2)
		N_cbps = 192
		N_bpsc = 4
	elif(regime == "6"):
		modulation = "qam16"
		rate = 3/float(4)
		N_cbps = 192
		N_bpsc = 4
	elif(regime == "7"):
		modulation = "qam64"
		rate = 2/float(3)
		N_cbps = 288
		N_bpsc = 6
	elif(regime == "8"):
		modulation = "qam64"
		rate = 3/float(4)
		N_cbps = 288
		N_bpsc = 6
	
	# uncoded bits per OFDM symbol
        N_dbps = int(N_cbps * rate)	

	# number of DATA symbols the frame needs to have
	# 16 service bits, 32 crc bits and 6 tail bits need to be included
	N_sym = int(math.ceil((16 + 32 + 6 + 8 * len(packet))/float(N_dbps)))

	# airtime of frame in microseconds
	# the additional 2 at the end of this formula is not in the 
	# standard encoding rules but in the annex G reference frame it is there!
	txtime = int(5*symboltime + symboltime * N_sym) + 2   
	mac_duration = chr((txtime >> 8) & 0xff) + chr(txtime & 0xff)
	
	# number of uncoded bits that could be sent using N_sym DATA OFDM symbols 
	N_data = int(N_sym * N_dbps)

	# number of (uncoded) padding bits that need to be added at the end
	N_pad = N_data - (16 + 32 + 6 + 8 * len(packet))

        # assemble the MPDU (now duration is correct)
	packet = mac_framectrl + mac_duration + mac_address1 + mac_address2 + mac_address3 + mac_seqctrl + payload

	dic = {"packet":packet, "packet_len":(len(packet)+ 4), "txtime":txtime, "mac_duration":mac_duration, "modulation":modulation,\
	       "rate":rate, "N_sym":N_sym, "N_data":N_data,"N_pad":N_pad, "N_cbps":N_cbps, "N_bpsc":N_bpsc, "N_dbps":N_dbps}	

	return dic


def ftw_make(payload, regime, symboltime):

	info = get_info(payload, regime, symboltime)
	packet        = info["packet"]
	packet_len    = info["packet_len"]
	N_sym         = info["N_sym"]
	N_data        = info["N_data"]
	N_pad         = info["N_pad"]
	txtime        = info["txtime"]

	# check length of MPDU
	MAXLEN = 3168
	if packet_len > MAXLEN:
		raise ValueError, "MPDU-length must be in [0, %d]" % (MAXLEN,)
	
	print"Txtime in microseconds:",txtime
	print"Length of MPDU in bytes = ", packet_len
	print"Number of DATA OFDM symbols = ", N_sym	
	print"Number of padding bits = ", N_pad

	# generate rate bits in SIGNAL OFDM symbol
	if (regime == "1"):
		rate_bits = 0x0d
	elif (regime == "2"):
		rate_bits = 0x0f
	elif (regime == "3"):
		rate_bits = 0x05
	elif (regime == "4"):
		rate_bits = 0x07
	elif (regime == "5"):
		rate_bits = 0x09
	elif (regime == "6"):
		rate_bits = 0x0b
	elif (regime == "7"):
		rate_bits = 0x01
	elif (regime == "8"):
		rate_bits = 0x03

	# generate length bits in SIGNAL OFDM symbol
	app = 0
	for i in range (0,12):
		app = app | (((packet_len >> i) & 1) << (11 - i))
	Length = app

	# generate parity check bit in SIGNAL OFDM symbol
	parA = parB = 0	
	for k in range (0,12):
		parA += ((Length >> k) & (0x01))
		parB += ((rate_bits   >> k) & (0x01))
	parity_bit =  (parA + parB) % 2
	
	# generate tail and padding bits
	if ((N_pad + 6) % 8 == 0):
		app = ''
		for i in range (0, (N_pad + 6)/8):
			app += chr(0x00)
		TAIL_and_PAD = app
	else:
		app = ''
		for i in range (0, (N_pad + 6)/8):
			app += chr(0x00)
		# add one more byte if N_pad + 6 is not a multiple of 8
		# we remove those bits again after the convolutional encoder
		TAIL_and_PAD = app + chr(0x00)      
		
	# generate all bits for SIGNAL OFDM symbol
	Signal_tail = 0x0
	Signal_field = 0x000
	Signal_field = (rate_bits << 20) | (Length << 7) | Signal_tail | (parity_bit << 6)
	chr1 = chr((Signal_field >> 16) & 0xff)	
	chr2 = chr((Signal_field >> 8) & 0xff)
	chr3 = chr(Signal_field  & 0xff)
	SIGNAL_FIELD = chr1 + chr2 + chr3

	# generate 16 all-zero SERVICE bits
	SERVICE = chr(0x00) + chr(0x00)

	PLCP_HEADER = SIGNAL_FIELD + SERVICE
	
	MPDU = make_MPDU (packet)
        #print string_to_hex_list(MPDU)
	
	MPDU_with_crc32 = gen_and_append_crc32(MPDU , packet) 
        #print string_to_hex_list(MPDU_with_crc32)

	Length = len(MPDU_with_crc32)
        '''
        print conv_packed_binary_string_to_1_0_string(PLCP_HEADER), len(PLCP_HEADER)
        print string_to_hex_list(PLCP_HEADER)
        print
        print conv_packed_binary_string_to_1_0_string(MPDU_with_crc32), len(MPDU_with_crc32)
        print string_to_hex_list(MPDU_with_crc32)
        print
        print conv_packed_binary_string_to_1_0_string(TAIL_and_PAD), len(TAIL_and_PAD)
	print string_to_hex_list(TAIL_and_PAD)
        '''
        #print "len(PLCP) = ", len(PLCP_HEADER), "len(MPDU+CRC32) = ", len(MPDU_with_crc32), "len(TAIL_PAD) = ", len(TAIL_and_PAD)
	return PLCP_HEADER + MPDU_with_crc32 + TAIL_and_PAD , Length


def make_MPDU(payload):
	app = conv_packed_binary_string_to_1_0_string(payload)
	app = list(app)
	mpdu = ['0'] * len(app)
	j = 0
	while (j < len(app)):
		for i in range (0, 8):
			mpdu[i+j] = app[7-i+j]     # Change into transmit order
		j += 8
	mpdu = "".join(mpdu)
	return conv_1_0_string_to_packed_binary_string(mpdu)



def interleaver (payload, regime, N_cbps, N_bpsc):

	length_payload = len(payload)
	length_payload_bit = 8 * length_payload
	half_interleaved = first_permutation(payload, N_cbps, length_payload_bit)
	interleaved = second_permutation(half_interleaved, N_cbps, N_bpsc, length_payload_bit)

	return interleaved

def first_permutation(payload, N_cbps, length_payload_bit):

	app = conv_packed_binary_string_to_1_0_string(payload)
	app=list(app)
	new = ['0'] * len(app)
	if(N_cbps == 48): # Modulation "bpsk"
		j=0
		while(j < length_payload_bit/N_cbps):
			
			for k in range (0, N_cbps):
				i = (N_cbps/16) * (k%16) + int(math.floor(k/16))	
				new[i + (j*N_cbps)] = app[k + (j*N_cbps)]
			j +=1

	else:   # other modulation ---> first 48 bits alone (signal field)

		for k in range (0, 48):
			i = (48/16) * (k%16) + int(math.floor(k/float(16)))	
			new[i] = app[k]
		j = 0
		while(j < (length_payload_bit - 48) / N_cbps):
			for k in range (0, N_cbps):
				i = (N_cbps/16) * (k%16) + int(math.floor(k/float(16)))
				new[i + 48 + (j*N_cbps)] = app[k + 48 + (j*N_cbps)] 
			j +=1		
	
	new = "".join(new)
	return conv_1_0_string_to_packed_binary_string(new)


def second_permutation(half_interleaved, N_cbps, N_bpsc, length_payload_bit):

	app = conv_packed_binary_string_to_1_0_string(half_interleaved)
	app=list(app)
	new_temp = ['0'] * (len(app) - 48)
	new = app[0:48] + new_temp
	s = max(N_bpsc/2 , 1)
	k=0
	
	while (k < (length_payload_bit - 48)/N_cbps):
		for i in range (0 , N_cbps):
			j = (s * int(math.floor(i/s))) + (i + N_cbps - (int(math.floor(16*i/float(N_cbps))))) % s
			new[j + 48 + (k*N_cbps)] = app[i + 48 + (k*N_cbps)]
		k +=1
	
	new = "".join(new)
	return conv_1_0_string_to_packed_binary_string(new)


def conv_encoder(pkt, Length, regime, N_cbps, N_bpsc, N_sym, N_rate):

	Real_num_of_bits = N_sym * N_cbps    # We could have more then correct number we need 
					     # See  Tail and pad in ftw_make function	 
	app = conv_packed_binary_string_to_1_0_string(pkt)
	app = list(app)
	encoded = ['0'] * (2 * len(app)) 
	g0 = 0x5b   # Generator polynomial (133 base 8)
	g1 = 0x79   # Generator polynomial (171 base 8)
	outA = 0
	outB = 0
 	register = 0
	for i in range (0, len(app)):
		if(app[i] == '1'):
			register = register >> 1     # Shift the status in the conv encoder  
			register = register | 0x40   # push 1 in the first element in the register after the shift
		else:
			register = register >> 1
		
		modA = modB = 0	
		for k in range (0,8):
			modA += (((register & g0) >> k) & (0x01))     # Parity check (count the number of 1s)
			modB += (((register & g1) >> k) & (0x01))     # Parity check
			
		outA     =  modA % 2  # Modulo 2 sum
		outB     =  modB % 2
		encoded[2 * i]   = str(outA)
		encoded[2 * i + 1] = str(outB)

	# Puncturing operations-----------------------------------------------
	if (regime == "1" or regime== "3" or regime == "5"):
		encoded = encoded[0:48 + Real_num_of_bits]
		encoded = "".join(encoded)
		#print conv_1_0_string_to_packed_binary_string(encoded)
		return conv_1_0_string_to_packed_binary_string(encoded)

	elif (regime == "2" or regime == "4" or regime == "6" or regime == "8"):
		signal_coded = encoded[0:48]
		data_coded = encoded[48:]
		dynamic_offset = 0	
		new = data_coded[0:3]
		while dynamic_offset + 9 < len(data_coded)-1 :
			new = new + data_coded[5 + dynamic_offset : 9 + dynamic_offset]
			dynamic_offset += 6
		new.append(data_coded[5 + dynamic_offset])
		new = new[0:Real_num_of_bits]
		new = signal_coded + new
		encoded = "".join(new)
		return conv_1_0_string_to_packed_binary_string(encoded)

	elif (regime == "7"): 
		dinamic_offset = 0
		signal_coded = encoded[0:48]
		data_coded = encoded[48:]
		new = data_coded[0:3]
		while dinamic_offset + 4 < len(data_coded) -1 :
			new = new + data_coded[4 + dinamic_offset : 7 + dinamic_offset]
			dinamic_offset += 4
		new = new[0:Real_num_of_bits]		
		new = signal_coded + new
		encoded = "".join(new)
		return conv_1_0_string_to_packed_binary_string(encoded)	
			

def scrambler(pkt, Length_data):

	scrambling_seq = [0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1,\
                          0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1,\
                          0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1,\
                          0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1];

	app = conv_packed_binary_string_to_1_0_string(pkt)
	app = list(app)
	zero_forcing_index = Length_data * 8
	scrambled = app[0:24]

	# Start from 24 because SIGNAL symbol mustn't be scrambled
	for k in range (24 , len(app)):              
		scrambled.append(str(int(app[k])^(scrambling_seq[(k-24) % 127])))

	# Force six bit to "0" in return to 0 state at last (this is tail bits, not set by scrambler * by lzyou)
	for i in range (23 + zero_forcing_index +17 , 23 + zero_forcing_index + 22 + 1):
		scrambled[i] = '0'
#        print zero_forcing_index, len(app)

	scrambled = "".join(scrambled)

	return conv_1_0_string_to_packed_binary_string(scrambled) , Length_data


def gen_and_append_crc32(MPDU, packet_for_crc):
	crc = ftw.ftw_crc32(packet_for_crc)
	return MPDU + struct.pack(">I", hexint(crc) & 0xFFFFFFFF)

def insert_preamble(length, N_sym, role=None):
	ftw_preamble= [list(fft_preamble)]
        if role == 'A':
	    preamble = ftw.pnc_ofdm_preamble(length, N_sym, ftw_preamble, 1)
        elif role == 'B':
	    preamble = ftw.pnc_ofdm_preamble(length, N_sym, ftw_preamble, 2)
        else:
	    preamble = ftw.ofdm_preamble(length, N_sym, ftw_preamble)

	return preamble 

def insert_zerogap(length, N_sym, state='FTW'):
	gap = [list(gap_sample)]
        if state == 'FPNC':
            ftw_zerogap = ftw.pnc_zerogap(length, N_sym, gap)
#            ftw_zerogap = ftw.zerogap(length, N_sym, gap)
        else:
	    ftw_zerogap = ftw.zerogap(length, N_sym, gap)
	return ftw_zerogap

def qam16(self):
	return gray_const_qam16
	
def qam64(self):
	return gray_const_qam64

def qpsk(self):
	return gray_const_qpsk

def bpsk(self):
	return gray_const_bpsk

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

def conv_1_0_string_to_packed_binary_string(s):
    """
    '10101111' -> ('\xAF', False)

    Basically the inverse of conv_packed_binary_string_to_1_0_string,
    but also returns a flag indicating if we had to pad with leading zeros
    to get to a multiple of 8.
    """
    if not is_1_0_string(s):
        raise ValueError, "Input must be a string containing only 0's and 1's"
    
    # pad to multiple of 8
    padded = False
    rem = len(s) % 8
    if rem != 0:
        npad = 8 - rem
        s = '0' * npad + s
        padded = True

    assert len(s) % 8 == 0

    r = []
    i = 0
    while i < len(s):
        t = 0
        for j in range(8):
            t = (t << 1) | (ord(s[i + j]) - ord('0'))
        r.append(chr(t))
        i += 8
    return ''.join(r)
        

def is_1_0_string(s):
    if not isinstance(s, str):
        return False
    for ch in s:
        if not ch in ('0', '1'):
            return False
    return True

def string_to_hex_list(s):
    return map(lambda x: hex(ord(x)), s)


def hexint(mask):
  """
  Convert unsigned masks into signed ints.

  This allows us to use hex constants like 0xf0f0f0f2 when talking to
  our hardware and not get screwed by them getting treated as python
  longs.
  """
  if mask >= 2**31:
     return int(mask-2**32)
  return mask


def ascii_to_bin(char):
	ascii = ord(char)
	bin = []

	while (ascii > 0):
		if (ascii & 1) == 1:
		     bin.append("1")
		else:
		     bin.append("0")
		ascii = ascii >> 1             
        bin.reverse()
	binary = "".join(bin)
	zerofix = (8 - len(binary)) * '0'

	return zerofix + binary


#////////////////////////////////////////////////////////////////////////////////
#                                Static tables
#///////////////////////////////////////////////////////////////////////////////

gray_const_bpsk = (-1+0j, +1+0j)

gray_const_qpsk = (-0.7071 - 0.7071j, -0.7071 + 0.7071j, +0.7071 - 0.7071j, +0.7071 + 0.7071j)

gray_const_qam16 = (-0.9487 - 0.9487j,-0.9487 - 0.3162j,-0.9487 + 0.9487j,-0.9487 + 0.3162j,-0.3162 - 0.9487j,-0.3162 - 0.3162j,-0.3162 + 0.9487j,-0.3162 + 0.3162j,0.9487 - 0.9487j, 0.9487 - 0.3162j, 0.9487 + 0.9487j, 0.9487 + 0.3162j, 0.3162 - 0.9487j, 0.3162 - 0.3162j, 0.3162 + 0.9487j, 0.3162 + 0.3162j)

gray_const_qam64 = (-1.0801 - 1.0801j,  -1.0801 - 0.7715j,  -1.0801 - 0.1543j,  -1.0801 - 0.4629j, -1.0801 + 1.0801j,  -1.0801 + 0.7715j,  -1.0801 + 0.1543j,  -1.0801 + 0.4629j, -0.7715 - 1.0801j,  -0.7715 - 0.7715j,  -0.7715 - 0.1543j,  -0.7715 - 0.4629j, -0.7715 + 1.0801j,  -0.7715 + 0.7715j,    -0.7715 + 0.1543j,  -0.7715 + 0.4629j, -0.1543 - 1.0801j,  -0.1543 - 0.7715j,  -0.1543 - 0.1543j,  -0.1543 - 0.4629j, -0.1543 + 1.0801j,  -0.1543 + 0.7715j,  -0.1543 + 0.1543j,  -0.1543 + 0.4629j, -0.4629 - 1.0801j,  -0.4629 - 0.7715j,  -0.4629 - 0.1543j,  -0.4629 - 0.4629j, -0.4629 + 1.0801j,   -0.4629 + 0.7715j,  -0.4629 + 0.1543j,  -0.4629 + 0.4629j, 1.0801 - 1.0801j,   1.0801 - 0.7715j,   1.0801 - 0.1543j,   1.0801 - 0.4629j, 1.0801 + 1.0801j,   1.0801 + 0.7715j,   1.0801 + 0.1543j,   1.0801 + 0.4629j, 0.7715 - 1.0801j,   0.7715 - 0.7715j,   0.7715 - 0.1543j,   0.7715 - 0.4629j,
0.7715 + 1.0801j,   0.7715 + 0.7715j,   0.7715 + 0.1543j,   0.7715 + 0.4629j, 0.1543 - 1.0801j,   0.1543 - 0.7715j,   0.1543 - 0.1543j, 0.1543 - 0.4629j,
0.1543 + 1.0801j,   0.1543 + 0.7715j,   0.1543 + 0.1543j,   0.1543 + 0.4629j, 0.4629 - 1.0801j,   0.4629 - 0.7715j,   0.4629 - 0.1543j,   0.4629 - 0.4629j,
0.4629 + 1.0801j,   0.4629 + 0.7715j,   0.4629 + 0.1543j,   0.4629 + 0.4629j)

fft_preamble = (0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j,-0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j, 0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j, -0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j,0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j,-0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j, 0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j, -0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j, 0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j,-0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j, 0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j, -0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j, 0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j,-0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j, 0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j, -0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j, 0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j,-0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j, 0.3680 + 0.3680j, -1.0596 + 0.0187j, -0.1078 - 0.6282j, 1.1421 - 0.1012j, 0.7360, 1.1421 - 0.1012j, -0.1078 - 0.6282j, -1.0596 + 0.0187j, 0.3680 + 0.3680j, 0.0187 - 1.0596j, -0.6282 - 0.1078j, -0.1012 + 1.1421j, 0 + 0.7360j, -0.1012 + 1.1421j, -0.6282 - 0.1078j, 0.0187 - 1.0596j,-1.2500, 0.0983 - 0.7808j, 0.7337 - 0.8470j, -0.7351 - 0.9210j, -0.0224 - 0.4302j, 0.6006 + 0.5923j, -1.0186 + 0.1640j, -0.9751 + 0.1325j, -0.2803 + 1.2071j, -0.4516 + 0.1744j, -0.4825 - 0.6503j, 0.5565 - 0.1130j, 0.6577 - 0.7389j, -1.0501 - 0.5218j, -0.4577 - 0.3144j, 0.2953 - 0.7868j, 0.5000 + 0.5000j, 0.9539 + 0.0328j, -0.1799 - 1.2853j, 0.4694 + 0.1195j, 0.1958 + 0.4683j, -1.0944 + 0.3790j, 0.0079 + 0.9200j,  0.4267 - 0.0326j, 0.7803 + 0.2071j, -0.3065 + 0.8494j, -0.9210 + 0.4414j, 0.4786 + 0.7017j, 0.1689 - 0.2231j, 0.7747 - 0.6624j, 0.3180 + 0.8893j, -0.0410 + 0.9626j, 1.2500, -0.0410 - 0.9626j, 0.3180 - 0.8893j, 0.7747 + 0.6624j, 0.1689 + 0.2231j, 0.4786 - 0.7017j, -0.9210 - 0.4414j, -0.3065 - 0.8494j, 0.7803 - 0.2071j, 0.4267 + 0.0326j, 0.0079 - 0.9200j, -1.0944 - 0.3790j, 0.1958 - 0.4683j, 0.4694 - 0.1195j, -0.1799 + 1.2853j, 0.9539 - 0.0328j, 0.5000 - 0.5000j, 0.2953 + 0.7868j, -0.4577 + 0.3144j, -1.0501 + 0.5218j, 0.6577 + 0.7389j, 0.5565 + 0.1130j, -0.4825 + 0.6503j, -0.4516 - 0.1744j, -0.2803 - 1.2071j, -0.9751 - 0.1325j, -1.0186 - 0.1640j, 0.6006 - 0.5923j, -0.0224 + 0.4302j, -0.7351 + 0.9210j, 0.7337 + 0.8470j, 0.0983 + 0.7808j,-1.2500, 0.0983 - 0.7808j, 0.7337 - 0.8470j, -0.7351 - 0.9210j, -0.0224 - 0.4302j, 0.6006 + 0.5923j, -1.0186 + 0.1640j, -0.9751 + 0.1325j, -0.2803 + 1.2071j, -0.4516 + 0.1744j, -0.4825 - 0.6503j, 0.5565 - 0.1130j, 0.6577 - 0.7389j, -1.0501 - 0.5218j, -0.4577 - 0.3144j, 0.2953 - 0.7868j, 0.5000 + 0.5000j, 0.9539 + 0.0328j, -0.1799 - 1.2853j, 0.4694 + 0.1195j, 0.1958 + 0.4683j, -1.0944 + 0.3790j, 0.0079 + 0.9200j,  0.4267 - 0.0326j, 0.7803 + 0.2071j, -0.3065 + 0.8494j, -0.9210 + 0.4414j, 0.4786 + 0.7017j, 0.1689 - 0.2231j, 0.7747 - 0.6624j, 0.3180 + 0.8893j, -0.0410 + 0.9626j, 1.2500, -0.0410 - 0.9626j, 0.3180 - 0.8893j, 0.7747 + 0.6624j, 0.1689 + 0.2231j, 0.4786 - 0.7017j, -0.9210 - 0.4414j, -0.3065 - 0.8494j, 0.7803 - 0.2071j, 0.4267 + 0.0326j, 0.0079 - 0.9200j, -1.0944 - 0.3790j, 0.1958 - 0.4683j, 0.4694 - 0.1195j, -0.1799 + 1.2853j, 0.9539 - 0.0328j, 0.5000 - 0.5000j, 0.2953 + 0.7868j, -0.4577 + 0.3144j, -1.0501 + 0.5218j, 0.6577 + 0.7389j, 0.5565 + 0.1130j, -0.4825 + 0.6503j, -0.4516 - 0.1744j, -0.2803 - 1.2071j, -0.9751 - 0.1325j, -1.0186 - 0.1640j, 0.6006 - 0.5923j, -0.0224 + 0.4302j, -0.7351 + 0.9210j, 0.7337 + 0.8470j, 0.0983 + 0.7808j,-1.2500, 0.0983 - 0.7808j, 0.7337 - 0.8470j, -0.7351 - 0.9210j, -0.0224 - 0.4302j, 0.6006 + 0.5923j, -1.0186 + 0.1640j, -0.9751 + 0.1325j, -0.2803 + 1.2071j, -0.4516 + 0.1744j, -0.4825 - 0.6503j, 0.5565 - 0.1130j, 0.6577 - 0.7389j, -1.0501 - 0.5218j, -0.4577 - 0.3144j, 0.2953 - 0.7868j, 0.5000 + 0.5000j, 0.9539 + 0.0328j, -0.1799 - 1.2853j, 0.4694 + 0.1195j, 0.1958 + 0.4683j, -1.0944 + 0.3790j, 0.0079 + 0.9200j,  0.4267 - 0.0326j, 0.7803 + 0.2071j, -0.3065 + 0.8494j, -0.9210 + 0.4414j, 0.4786 + 0.7017j, 0.1689 - 0.2231j, 0.7747 - 0.6624j, 0.3180 + 0.8893j, -0.0410 + 0.9626j)

gap_sample = (0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j,0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j,0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j,0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j,0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j,0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j,0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j, 0+0j)

