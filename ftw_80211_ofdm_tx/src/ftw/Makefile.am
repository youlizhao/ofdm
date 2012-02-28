#
# Copyright 2004,2005,2006,2008,2009 Free Software Foundation, Inc.
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

include $(top_srcdir)/Makefile.common

# C/C++ headers get installed in ${prefix}/include/gnuradio
grinclude_HEADERS =             \
	ftw_crc32.h             \
	ftw_ofdm_cmap_cc.h      \
	ftw_ofdm_mapper.h       \
	ftw_ofdm_pilot_cc.h     \
        ftw_pnc_ofdm_pilot_cc.h \
	ftw_ofdm_preamble.h     \
        ftw_pnc_ofdm_preamble.h \
	ftw_repetition.h        \
	ftw_zerogap.h           \
        ftw_pnc_zerogap.h       \
        ftw_pnc_frequency_modulator_fc.h

###################################
# SWIG Python interface and library

TOP_SWIG_IFILES = 		\
	ftw.i

# Install so that they end up available as:
#   import gnuradio.ftw
# This ends up at:
#   ${prefix}/lib/python${python_version}/site-packages/gnuradio
ftw_pythondir_category =	\
	gnuradio

# additional arguments to the SWIG command
#ftw_la_swig_args =		\
#	-g

# additional sources for the SWIG-generated library
ftw_la_swig_sources =          \
	ftw_crc32.cc             \
	ftw_ofdm_cmap_cc.cc      \
	ftw_ofdm_mapper.cc       \
	ftw_ofdm_pilot_cc.cc     \
        ftw_pnc_ofdm_pilot_cc.cc \
	ftw_ofdm_preamble.cc     \
        ftw_pnc_ofdm_preamble.cc \
	ftw_repetition.cc        \
	ftw_zerogap.cc           \
        ftw_pnc_zerogap.cc       \
        ftw_pnc_frequency_modulator_fc.cc

# additional arguments to the SWIG command
#ftw_la_swig_args =		\
#	$(SWIG_PYTHON_ARGS)

# additional libraries for linking with the SWIG-generated library
#ftw_la_swig_libadd =		\
#	libm.dylib

# additional LD flags for linking the SWIG-generated library
#ftw_la_swig_ldflags =	\
#	-L/opt/local/bin

# additional Python files to be installed along with the SWIG-generated one
#ftw_python =			\
#	__init__.py

# additional SWIG files to be installed
#ftw_swiginclude_headers =	\
#	ftw_crc32.i

include $(top_srcdir)/Makefile.swig

# add some of the variables generated inside the Makefile.swig.gen
BUILT_SOURCES = $(swig_built_sources)

# Do not distribute the output of SWIG
no_dist_files = $(swig_built_sources)
