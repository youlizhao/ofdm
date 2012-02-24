###################################
# Copyright 2010 Szymon Jakubczak
#
# utilities for RawOFDM

include $(top_srcdir)/Makefile.common

modinclude_HEADERS = \
        raw_fix_frame.h \
        raw_symbol_avg.h \
        raw_peak_detector_fb.h \
        raw_ber.h

noinst_LTLIBRARIES = libraw-util.la

libraw_util_la_SOURCES =  \
        raw_fix_frame.cc \
        raw_symbol_avg.cc \
        raw_peak_detector_fb.cc \
        raw_ber.cc

libraw_util_la_LIBADD = \
        $(GNURADIO_CORE_LA)
