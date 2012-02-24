#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

def add_options(parser):
  """
  Adds usrp-specific options to the Options Parser
  """
  parser.add_option("-e", "--ifc", type="string", default="eth1",
                    help="select Ethernet interface, [default=%default]")
  parser.add_option("-a", "--mac", type="string", default="",
                    help="select USRP by MAC address, default is auto-select")
  parser.add_option("-f", "--freq", type="eng_float", default=None,
                    help="set transmit frequency to FREQ [default=%default]", metavar="FREQ")
  parser.add_option("-i", "--interp", type="intx", default=16,
                    help="set fpga decimation/interpolation rate to INTERP [default=%default]")
  parser.add_option("", "--gain", type="eng_float", default=None, metavar="GAIN",
                    help="set gain in dB [default=max(tx),midpoint(rx)]")

def expand(mac):
  mac_prefix = "00:50:c2:85:32:3c"
  mac = mac_prefix[:(len(mac_prefix)-len(mac))] + mac
  return mac

def sink(options):
  from gnuradio import usrp2
  u = usrp2.sink_32fc (ifc=options.ifc, mac=expand(options.mac))
  u.set_interp(options.interp)
  ok = u.set_center_freq(options.freq)
  if not ok:
    raise ValueError("Failed to set Tx frequency to %s" % (eng_notation.num_to_str(options.freq),))
  if options.gain is None:
    options.gain = u.gain_max() # default gain is MAX (actually, it doesn't seem to matter)
  u.set_gain(options.gain) # (self.u.gain_range()[1])
  return u

def source(options):
  from gnuradio import usrp2
  u = usrp2.source_32fc (ifc=options.ifc, mac=expand(options.mac))
  u.set_decim(options.interp)
  ok = u.set_center_freq(options.freq)
  if not ok:
    raise ValueError("Failed to set Rx frequency to %s" % (eng_notation.num_to_str(options.freq)))
  if options.gain is None:
    options.gain = (u.gain_min() + u.gain_max())/2 # default gain is midpoint
  u.set_gain(options.gain)
  return u
