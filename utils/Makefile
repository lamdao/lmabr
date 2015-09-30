CFLAGS=-O3 -s -I. -I..

ifeq "$(USE_DOUBLE)" "1"
CFLAGS+=-DUSE_DOUBLE
endif

all:
	g++ $(CFLAGS) -o createbx createbx.cc -lstdc++
	./createbx bx.dat
