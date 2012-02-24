###################################
# Copyright 2010 Szymon Jakubczak
#
# OFDM for RawOFDM

include $(top_srcdir)/Makefile.common

modinclude_HEADERS = \
        raw_ofdm_mapper.h \
        raw_ofdm_demapper.h \
        raw_ofdm_frame_acquisition.h \
        raw_ofdm_sampler.h

noinst_LTLIBRARIES = libraw-ofdm.la

libraw_ofdm_la_SOURCES =  \
        raw_ofdm_mapper.cc \
        raw_ofdm_demapper.cc \
        raw_ofdm_frame_acquisition.cc \
        raw_ofdm_sampler.cc

libraw_ofdm_la_LIBADD = \
        $(GNURADIO_CORE_LA)

