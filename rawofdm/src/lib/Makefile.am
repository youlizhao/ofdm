###################################
# Copyright 2010 Szymon Jakubczak
#
# RawOFDM

include $(top_srcdir)/Makefile.common

SUBDIRS = ofdm qam rscode spiral util
DIST_SUBDIRS = $(SUBDIRS)

lib_LTLIBRARIES = libgnuradio-raw.la

libgnuradio_raw_la_SOURCES =

libgnuradio_raw_la_LIBADD = \
        ofdm/libraw-ofdm.la \
        qam/libraw-qam.la \
        util/libraw-util.la \
        spiral/libspiral.la \
        rscode/librscode.la

if PYTHON
###################################
# SWIG Python interface and library

TOP_SWIG_IFILES = \
        raw.i

raw_la_swig_libadd = \
        libgnuradio-raw.la

include $(top_srcdir)/Makefile.swig

# add some of the variables generated inside the Makefile.swig.gen
BUILT_SOURCES = $(swig_built_sources)

# Do not distribute the output of SWIG
no_dist_files = $(swig_built_sources)

# additional SWIG files to be installed
# FIXME: they are installed in swig/ without subdirectories!
raw_swiginclude_headers = \
        ofdm/raw_ofdm.i \
        qam/raw_qam.i \
        util/raw_util.i

endif
