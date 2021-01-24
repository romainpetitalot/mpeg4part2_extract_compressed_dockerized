#!/bin/bash
# Test script for the manual compilation of the extract_mvs module

rm -r bin

swig -python mpeg42compressed/numpy/extract_mvs.i

mkdir bin

gcc -fpic -c mpeg42compressed/common/extract_mvs.c -o bin/extract_mvs.o -I/usr/local/include/python3.6m
gcc -fpic -c mpeg42compressed/numpy/extract_mvs_wrap.c -o bin/extract_mvs_wrap.o -I/usr/local/include/python3.6m

ld -shared bin/extract_mvs.o bin/extract_mvs_wrap.o -o mpeg42compressed/numpy/_extract_mvs.so -lavutil -lavformat -lavcodec