#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

from gnuradio.wxgui import stdgui2, scopesink2
import wx

class _app(stdgui2.stdapp):
  def __init__(self, top_block, data_block, title, isComplex):
    def top_block_maker(frame, panel, vbox, arg):
      # TODO: compute correct rate, change to _c on demand?
      if isComplex:
        sink = scopesink2.scope_sink_c(panel, sample_rate=1e3)
      else:
        sink = scopesink2.scope_sink_f(panel, sample_rate=1e3)

      top_block.connect(data_block, sink)
      vbox.Add(sink.win, 10, wx.EXPAND)
      return top_block

    stdgui2.stdapp.__init__(self, top_block_maker, title=title, nstatus=2)


def scope(top_block, data_block, title, isComplex=False):
  # returns a new function that creates app
  def _make():
    _app(top_block, data_block, title, isComplex).MainLoop()

  return _make




