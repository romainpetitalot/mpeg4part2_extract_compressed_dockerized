#!/bin/bash
# Script to compile and generate the motion vector extraction method

swig -python extract_mvs.i

gcc -fpic -c extract_mvs.c extract_mvs_wrap.c -I/usr/local/include/python3.6m

ld -shared extract_mvs.o extract_mvs_wrap.o -o _extract_mvs.so -lavutil -lavformat -lavcodec