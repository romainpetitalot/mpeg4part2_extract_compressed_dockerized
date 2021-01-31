rm _extract_mvs.so
rm extract_mvs_wrap.c
rm extract_mvs_wrap.o
rm extract_mvs.o
rm extract_mvs.py

swig -python extract_mvs.i

gcc -fpic -c extract_mvs.c -o extract_mvs.o -I/usr/local/include/python3.6m
gcc -fpic -c extract_mvs_wrap.c -o extract_mvs_wrap.o -I/usr/local/include/python3.6m

ld -shared extract_mvs_wrap.o extract_mvs.o  -o _extract_mvs.so -lavutil -lavformat -lavcodec