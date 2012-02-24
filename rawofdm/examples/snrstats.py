#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Szymon Jakubczak
#

import numpy, sys

data = numpy.fromfile(sys.stdin, dtype=numpy.float32, count=-1, sep='')

print len(data), numpy.mean(data), numpy.std(data), numpy.min(data)
(count, val) = numpy.histogram(data, bins=[-100, -5, 0, 5, 10, 15, 20, 25, 30])

total = 0
for i in range(len(count)):
  print "%d\t%.2f\t%d" % (count[i], val[i], total)
  total+= count[i]
print 0, val[-1], total



