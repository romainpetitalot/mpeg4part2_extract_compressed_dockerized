#!/bin/bash
# Test script for the manual compilation of the extract_mvs module
INCLUDE_DIR="/usr/local/include"

rm -r bin

swig -python mpeg42compressed/numpy/extract_mvs.i

mkdir bin

gcc -fpic -c mpeg42compressed/common/extract_mvs.c -o bin/extract_mvs.o -I/usr/include/python3.11 -I $INCLUDE_DIR/ffmpeg -lavcodec -lavformat -lavutil
gcc -fpic -c mpeg42compressed/numpy/extract_mvs_wrap.c -o bin/extract_mvs_wrap.o -I/usr/include/python3.11  #TODO: Faire une commande bash qui recup tout seul la bonne version du /usr/include/python avec la commande python3-config --cflags

ld -shared bin/extract_mvs.o bin/extract_mvs_wrap.o -o mpeg42compressed/numpy/_extract_mvs.so -lavutil -lavformat -lavcodec