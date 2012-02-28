###################################
# Copyright 2010 Szymon Jakubczak
#
# modulation and FEC for RawOFDM

include $(top_srcdir)/Makefile.common

modinclude_HEADERS = \
        raw_qam.h \
        raw_conv.h \
        raw_intrlv.h \
        raw_rs.h \
        raw_scrambler_bb.h \
        raw_crc.h \
        gr_crc32.h

noinst_LTLIBRARIES = libraw-qam.la

libraw_qam_la_SOURCES =  \
        raw_qam.cc \
        raw_conv.cc \
        raw_intrlv.cc \
        raw_rs.cc \
        raw_scrambler_bb.cc \
        raw_crc.cc \
        gr_crc32.cc

libraw_qam_la_LIBADD = \
        $(GNURADIO_CORE_LA) \
        $(GR_DIGITAL_LA)

