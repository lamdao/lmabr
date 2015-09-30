HDRS=fx.h qr.h typedefs.h dfile.h
SRCS=main.cc lmabr.cc
GUIO=gui.o

PROGNAME=lma
CFLAGS=-I. -march=native -mtune=native -funroll-loops -flto
LIBS=-lstdc++

ifeq "$(USE_DOUBLE)" "1"
OPTS=-DUSE_DOUBLE
endif

ifeq "$(USE_BR)" "1"
OPTS+=-DUSE_LMA_BR
PROGNAME=lmabr
endif

ifeq "$(USE_M32)" "1"
OPTS+=-m32
else
OPTS+=-m64
endif

ifeq "$(USE_OMP)" "1"
OPTS+=-fopenmp
LIBS+=-lpthread
endif

RESOURCES=$(SRCS) $(HDRS)
ifeq "$(USE_GUI)" "1"
OPTS+=-DUSE_GUI $(shell pkg-config --cflags opencv)
LIBS+=$(GUIO) $(shell pkg-config --libs opencv)
RESOURCES+=$(GUIO)
endif

ifeq "$(DEBUG)" "1"
CFLAGS+=-O0 -g3
else
CFLAGS+=-O3 -s
ifeq "$(USE_FASTMATH)" "1"
CFLAGS+=-ffast-math
PROGEXT=-fm
endif
endif

CFLAGS+=$(OPTS)
SFLAGS=-DSHARED_LIB -fPIC -shared
TARGET=$(PROGNAME)$(PROGEXT)

all: $(TARGET)

gui.o: gui.cc gui.h
	g++ $(OPTS) -c $< 

$(TARGET): $(RESOURCES)
	g++ $(CFLAGS) -o $@ $(SRCS) $(LIBS)

lmadbg: $(RESOURCES)
	g++ $(CFLAGS) -o $@ $(SRCS) $(LIBS)

lib: $(SRCS) $(HDRS)
	g++ $(CFLAGS) $(SFLAGS) -o $(TARGET).so $(SRCS) $(LIBS)

clean:
	rm -f lma lma.so
	rm -f lmabr lmabr.so
	rm -f lma-fm lma-fm.so
	rm -f lmabr-fm lmabr-fm.so

