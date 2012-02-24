#!/bin/sh

for w in config bootstrap Makefile.swig Makefile.swig.gen.t; do
  ln -s $1/$w
done
