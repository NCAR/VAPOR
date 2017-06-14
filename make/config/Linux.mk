# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

G++-INCLUDE-DIR = /usr/include/g++

ifeq	($(HAVE_INTEL_COMPILERS),1)
CXX = icpc
CC = icc
else
CXX = g++
CC = gcc
endif


ifeq	($(HAVE_INTEL_COMPILERS),1)

CXXFLAGS          += -DLINUX -D__USE_LARGEFILE64
CXX_RELEASE_FLAGS += -O -DNDEBUG
CXX_DEBUG_FLAGS   += -g

CFLAGS            += -DLINUX -D__USE_LARGEFILE64
C_RELEASE_FLAGS   += -O
C_DEBUG_FLAGS     += -g

else

CXXFLAGS          += -std=c++0x -DLINUX -Wall -Wno-sign-compare 
CXXFLAGS          += -D__USE_LARGEFILE64 -pthread
CXX_RELEASE_FLAGS += -O3 -DNDEBUG -fno-strict-aliasing
CXX_DEBUG_FLAGS   += -g

#CFLAGS            += -DLINUX -Wall -Werror -Wmissing-prototypes -Wsign-compare
CFLAGS            += -DLINUX -Wall -Wmissing-prototypes -Wno-sign-compare -D__USE_LARGEFILE64
CFLAGS            += -pthread
C_RELEASE_FLAGS   += -O3 -DNDEBUG -fno-strict-aliasing
C_DEBUG_FLAGS     += -g

endif

CXXFLAGS          += -fPIC
CFLAGS          += -fPIC


ifeq ($(MACHTYPE),x86_64)
SHARED_LDFLAGS +=	-m64
LDFLAGS +=			-m64
CXXFLAGS +=			-m64
CFLAGS +=			-m64
else
SHARED_LDFLAGS +=	-m32
LDFLAGS +=			-m32
CXXFLAGS +=			-m32
CFLAGS +=			-m32
endif

RPATHFLAG		= -Wl,-rpath,

LDFLAGS			+= -lrt -pthread

LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    += 

PROFILEFLAGS = -pg -a

CAT = cat
AS = as
LEX = flex -t
LEXLIB = -ll
YACC = bison -y -d
LD = $(CXX)
AR = ar
ARCREATEFLAGS = cr
RANLIB = true
LN = ln -s
MKDIR = mkdir -p
RM = rm -f
CP = cp
#MAKE = gmake -s
MAKE = make 
NOWEB = noweb
LATEX = latex
BIBTEX = bibtex
DVIPS = dvips -t letter
GHOSTSCRIPT = gs
LIBPREFIX = lib
DLLSUFFIX = .so
LIBSUFFIX = .a
OBJSUFFIX = .o
MV = mv
SHARED_LDFLAGS += -shared -Wl,-soname,$(LIB_SONAME) -pthread
PERL = perl
PYTHON = python
JGRAPH = /u/eldridge/bin/IRIX/jgraph
PS2TIFF = pstotiff.pl
PS2TIFFOPTIONS = -alpha -mag 2
PS2PDF = ps2pdf

MPI_CC = mpicc
MPI_CXX = mpiCC
MPI_LDFLAGS =

INSTALL_EXEC = /usr/bin/install -m 0755
INSTALL_NONEXEC = /usr/bin/install -m 0644

CLD_EXCLUDE_LIBS = ^/usr ^/lib 

#
# various system libraries that aren't necessarily installed on all 
# flavors of linux
#
CLD_INCLUDE_LIBS += libpng
CLD_INCLUDE_LIBS += libcurl
CLD_INCLUDE_LIBS += libexpat
CLD_INCLUDE_LIBS += liblapack
CLD_INCLUDE_LIBS += libblas
CLD_INCLUDE_LIBS += libgfortran
CLD_INCLUDE_LIBS += libf77blas
CLD_INCLUDE_LIBS += libcblas
CLD_INCLUDE_LIBS += libatlas
