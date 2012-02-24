###################################
# Copyright 2010 Szymon Jakubczak
#
# Viterbi decoder from the Spiral Project

include $(top_srcdir)/Makefile.common

AM_CPPFLAGS += -msse3 -std=gnu99

noinst_HEADERS = \
        viterbi.h \
        parity.h

noinst_LTLIBRARIES = libspiral.la

libspiral_la_SOURCES =  \
        parity.c \
        viterbi_common.c \
        viterbi_spiral.c \
        spiral.c

libspiral_la_LDFLAGS = \
        -lm
