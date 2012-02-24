#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

import math, random, cmath

# /////////////////////////////////////////////////////////////////////////////
#                 Shared configuration
# /////////////////////////////////////////////////////////////////////////////

class ofdm_params:
  # General OFDM parameters:
  #   fft_length        size of FFT
  #   occupied_tones    how many tones in the FFT are occupied (excluding DC)
  #                     (the margin is used for shaping and coarse freq. tracking)
  #   cp_length         cyclic prefix
  #
  # FFT diagram:
  #   |< pad >|< occupied/2 >|DC|< occupied/2 >|< pad-1 >|
  #                          |^fft_length/2 == DC (even)

  def pad_symbol(self, symbol):
    padded = self.fft_length*[0,]
    padded[self.pad : self.pad + self.occupied_tones + 1] = symbol
    return padded

  def random_symbol(self):
    # NOTE: pilot bins have power 1 (see mapper/demapper)
    # we make our preamble have (complex) power 1 per bin as well x scaling factor
    sf = 1 # FIXME: allow non-unity scaling factors
    psk = lambda : cmath.exp(1j * 2 * math.pi * random.random())
    symbol = [ sf * psk() for i in range(self.occupied_tones)]
    # fill in the null DC
    half = self.occupied_tones/2
    return symbol[:half] + [0] + symbol[half:]

  def make_even(self, symbol):
    # NOTE: we're using only even frequencies, so give them more power
    # so that the overall symbol power is the same.
    # raw_ofdm_frame_acquisition::correlate() uses diff phase only so it's ok
    sf = math.sqrt(2)
    for i in range(len(symbol)):
      if((self.pad + i) & 1): # zero-out every odd frequency
        symbol[i] = 0
      else: # boost power in every even frequency
        symbol[i] *= sf

  def __init__(self, options):
    # for simplicity make fft_length and occupied_tones even
    self.fft_length = int(math.ceil(options.fft_length / 2.0) * 2)
    self.occupied_tones = int(math.ceil(options.occupied_tones / 2.0) * 2)
    self.cp_length = options.cp_length
    self.num_preambles = options.num_preambles

    pad = (self.fft_length - self.occupied_tones)/2
    self.pad = pad

    # make the preamble
    random.seed(9817)
    preambles = [ self.random_symbol() for i in range(options.num_preambles+1) ]

    self.half_sync = options.half_sync
    if self.half_sync:
      # the first one even for sync
      self.make_even(preambles[0])

    self.preambles = preambles

    if not self.half_sync:
      # repeat the first one for sync (but note frame_acq doesn't know about this)
      preambles = [ preambles[0] ] + preambles

    self.padded_preambles = [ self.pad_symbol(p) for p in preambles ]

    # build carrier map
    half = self.occupied_tones/2
    carriers = half*[1,]

    # set as pilots every 13 subcarriers
    num_pilots = 0
    for i in range(5, half, 13):
      carriers[i] = 2
      num_pilots += 1

    # mirror around the DC in the middle
    carriers.extend([0] + list(reversed(carriers)))
    self.carriers = carriers

    padded_carriers = self.fft_length*[0,]
    padded_carriers[pad : pad + self.occupied_tones + 1] = carriers
    self.padded_carriers = padded_carriers

    self.pilot_tones = num_pilots * 2
    self.data_tones = self.occupied_tones - self.pilot_tones

    # for all I/O sizing purposes, include DC
    self.occupied_tones+= 1

    if options.verbose:
        self._print_verbage()

  def add_options(normal, expert):
    """
    Adds OFDM-specific options to the Options Parser
    """
    expert.add_option("", "--fft-length", type="intx", default=64,
                      help="set the number of FFT bins [default=%default]")
    expert.add_option("", "--occupied-tones", type="intx", default=52,
                      help="set the number of occupied FFT bins [default=%default]")
    expert.add_option("", "--cp-length", type="intx", default=16,
                      help="set the number of bits in the cyclic prefix [default=%default]")
    expert.add_option("", "--num-preambles", type="intx", default=2,
                      help="number of extra preamble symbols [default=%default]")
    expert.add_option("", "--half-sync", action="store_true", default=True,
                      help="use two-halves symbol for sync [default]")
    expert.add_option("", "--full-sync", action="store_false", dest="half_sync", default=True,
                      help="use full symbol for sync")

  add_options = staticmethod(add_options)

  def _print_verbage(self):
    """
    Prints information about the OFDM modulator
    """
    from sys import stderr
    print >> stderr, "\nRaw OFDM parameters:"
    print >> stderr, "FFT length:      %3d"   % (self.fft_length)
    print >> stderr, "Occupied Tones:  %3d (with DC)"   % (self.occupied_tones)
    print >> stderr, "Pilot Tones:     %3d"   % (self.pilot_tones)
    print >> stderr, "Data Tones:      %3d"   % (self.data_tones)
    print >> stderr, "CP length:       %3d"   % (self.cp_length)
    print >> stderr, "Extra Preambles: %3d"   % (self.num_preambles)

