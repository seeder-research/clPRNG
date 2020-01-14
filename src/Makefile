#Optionally specify the path to the OpenCL headers
#and uncomment the line below
CXXFLAGS=-fPIC -Wall -m64 -pthread -O3
CXX ?= g++
INSTALL?=install
LDLIBS+=-lOpenCL

PREFIX?=/usr/local

MAJOR_VERSION=0
MINOR_VERSION=0
REV_VERSION=1
ifeq ($(OS),Windows_NT)
    CXXFLAGS+=-I${CUDA_PATH}\\include
    CXXFLAGS+=-D__WIN32 -DCLPRNG_EXPORT
    LDFLAGS+=-L${CUDA_PATH}\\lib\\x64
    DLLFLAGS=-Wl,--out-implib,libclPRNG.a -Wl,--major-image-version,0,--minor-image-version,1
    LIBEXT=.dll
else
    CXXFLAGS+= -I${CUDAROOT}/include
    LDFLAGS+=-L${CUDAROOT}/lib64
    LIBEXT=.so.${MAJOR_VERSION}.${MINOR_VERSION}.${REV_VERSION}
    DLLFLAGS=-Wl,-soname,libclPRNG.so.${MAJOR_VERSION}
endif

INSTALL_INCLUDEDIR=$(PREFIX)/$(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION)/include
INSTALL_LIBDIR=$(PREFIX)/$(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION)/lib

#On Windows specify the path to the OpenCL lib file
#the first commented line is the typical path for NVIDIA GPUs
#the second is for AMD GPUS.
#LDFLAGS= -L"$(CUDA_PATH)\lib\x64" -lOpenCL
#LDFLAGS= -L"$(AMDAPPSDKROOT)lib\x86_64" -lOpenCL

library/shared: clPRNG.cpp clPRNG.hpp
	${CXX} ${CXXFLAGS} -c clPRNG.cpp -o clPRNG.o
	${CXX} ${LDFLAGS} -shared clPRNG.o -o libclPRNG${LIBEXT} ${LDLIBS} ${DLLFLAGS}
	ln -s libclPRNG${LIBEXT} libclPRNG.so.${MAJOR_VERSION}
	ln -s libclPRNG.so.${MAJOR_VERSION} libclPRNG.so

library/static: clPRNG.cpp clPRNG.hpp
	${CXX} ${CXXFLAGS} -c clPRNG.cpp -o clPRNG.o
	${AR} rcs libclPRNG.a *.o

all: library/shared library/static

install: all
	$(INSTALL) -D -m 644 -t $(INSTALL_INCLUDEDIR) ../include/clPRNG.h
	$(INSTALL) -D -m 755 -t $(INSTALL_LIBDIR) *.so.$(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION)
	ln -Lrs $(INSTALL_LIBDIR)/libclPRNG.so.$(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION) $(INSTALL_LIBDIR)/libclPRNG.so.$(MAJOR_VERSION).$(MINOR_VERSION)
	ln -Lrs $(INSTALL_LIBDIR)/libclPRNG.so.$(MAJOR_VERSION).$(MINOR_VERSION) $(INSTALL_LIBDIR)/libclPRNG.so.$(MAJOR_VERSION)
	ln -Lrs $(INSTALL_LIBDIR)/libclPRNG.so.$(MAJOR_VERSION) $(INSTALL_LIBDIR)/libclPRNG.so
	$(INSTALL) -D -m 644 -t $(INSTALL_LIBDIR) *.a

clean:
	rm -f *.o *.lib lib*.so* *.a *.dll
