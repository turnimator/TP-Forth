# Thanks, Adrian Blake!
# Fixed, changed the make file. Thus:
ARCH ?= 64
CC := clang
CFLAGS = -O3 -m$(ARCH)
srcfiles := $(shell find . -name ".c")
incfiles := $(shell find . -name ".h")

tpforth: ( s r c f i l e s ) (incfiles)
( C C ) (CFLAGS) ( s r c f i l e s ) − o @ -lm

run: tpforth
./tpforth

clean:
rm -f ./tpforth
