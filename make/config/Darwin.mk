G++-INCLUDE-DIR = /usr/include/g++
CXX = c++ -fno-common
CC = cc -fno-common

ifeq    ($(HAVE_QT_FRAMEWORK),1)
QT_FRAMEWORK = 1
endif

CXXFLAGS          += -std=c++0x -stdlib=libc++ -DDARWIN -Wall 
CXXFLAGS          += -Wno-format -Wno-sign-compare -Wno-deprecated-register 
CXXFLAGS          += -fPIC 

ifeq ($(BUILD_FOR_MACOSX10_6), 1)
CXXFLAGS          += -mmacosx-version-min=10.6 
endif

CXXFLAGS          += -Wno-overloaded-virtual
CXX_RELEASE_FLAGS += -O3 -DNDEBUG
CXX_DEBUG_FLAGS   += -g

CFLAGS            += -DDARWIN -Wall -Wno-format -fPIC

ifeq ($(BUILD_FOR_MACOSX10_6), 1)
CFLAGS            += -mmacosx-version-min=10.6
endif

C_RELEASE_FLAGS   += -O3 -DNDEBUG
C_DEBUG_FLAGS     += -g

LDFLAGS           += -stdlib=libc++ -headerpad_max_install_names 
LDFLAGS           += -framework CoreFoundation

ifeq ($(BUILD_FOR_MACOSX10_6), 1)
LDFLAGS           += -mmacosx-version-min=10.6
endif

LD_RELEASE_FLAGS  += 
LD_DEBUG_FLAGS    += 

PROFILEFLAGS = -pg -a

ifeq ($(MACHTYPE),x86_64)
SHARED_LDFLAGS += -m64 

ifeq ($(BUILD_FOR_MACOSX10_6), 1)
SHARED_LDFLAGS += -mmacosx-version-min=10.6
endif 

LDFLAGS += -m64
CXXFLAGS += -m64
CFLAGS += -m64
else 
SHARED_LDFLAGS += -m32 

ifeq ($(BUILD_FOR_MACOSX10_6), 1)
SHARED_LDFLAGS += -mmacosx-version-min=10.6
endif

LDFLAGS += -m32
CXXFLAGS += -m32
CFLAGS += -m32
endif


CAT = cat
AS = as
LEX = flex -t
LEXLIB = -ll
YACC = bison -y -d
LD = $(CXX)
AR = ar
ARCREATEFLAGS = cr
RANLIB = ranlib
LN = ln -s
MKDIR = mkdir -p
RM = rm -f
CP = cp
MAKE = make
NOWEB = noweb
LATEX = latex
BIBTEX = bibtex
DVIPS = dvips -t letter
GHOSTSCRIPT = gs
LIBPREFIX = lib
ifndef	DLLSUFFIX
DLLSUFFIX = .dylib
endif
LIBSUFFIX = .a
OBJSUFFIX = .o
MV = mv
SHARED_LDFLAGS += -dynamiclib -current_version $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_RELEASE) -compatibility_version $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_RELEASE) -undefined dynamic_lookup
PERL = perl
PYTHON = python

INSTALL_EXEC = /usr/bin/install -m 0755
INSTALL_NONEXEC = /usr/bin/install -m 0644

CLD_EXCLUDE_LIBS =  ^/usr/lib ^/usr/X11 ^/System
