#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

import numpy, sys

data = numpy.fromfile(sys.stdin, dtype=numpy.float32, count=-1, sep='')

print len(data), numpy.mean(data), numpy.std(data), numpy.min(data)

# note: we don't expect more than 1e5 bits per packet, so...
(count, val) = numpy.histogram(data, bins=[0, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1])

total = 0
for i in range(len(count)):
  print "%d\t%g\t%d" % (count[i], val[i], total)
  total+= count[i]
print 0, val[-1], total



