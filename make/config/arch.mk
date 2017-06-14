# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')

ifeq ($(ARCH), IRIX64)
MACHTYPE=$(shell uname -p)
endif

ifeq ($(ARCH), Darwin)
MACHTYPE=$(shell uname -p)
ifeq ($(MACHTYPE), i386)
ifeq ($(strip $(shell /usr/sbin/sysctl -n hw.cpu64bit_capable)),1)
MACHTYPE=x86_64
endif
endif
endif

ifeq ($(ARCH), AIX)
MACHTYPE=$(shell uname -p)
endif

ifndef	MACHTYPE
MACHTYPE=$(shell uname -m)
endif

endif
endif


ifeq ($(ARCH), WIN_NT)
ARCH=WIN32
endif
ifeq ($(ARCH), WIN_98)
ARCH=WIN32
endif
ifeq ($(ARCH),WIN32)
WIN32 = 1
endif


ifeq ($(ARCH), CYGWIN_NT5.0)
ARCH=WIN32
endif

ifeq ($(ARCH), CYGWIN_NT5.1)
ARCH=WIN32
endif

# On suse versions > 10.2 'uname-m' appends extra crud
#
ifeq (x86_64, $(findstring x86_64, $(MACHTYPE)))
MACHTYPE = x86_64
endif

ifeq (i686, $(findstring i686, $(MACHTYPE)))
MACHTYPE = i386
endif

ifeq (i586, $(findstring i586, $(MACHTYPE)))
MACHTYPE = i386
endif

ifeq (i486, $(findstring i486, $(MACHTYPE)))
MACHTYPE = i386
endif

ifeq (i386, $(findstring i386, $(MACHTYPE)))
MACHTYPE = i386
endif

include $(TOP)/options.mk

#
# Handle forced word size
#
ifeq ($(strip $(FORCE_WORD_SIZE)),32)
ifeq ($(MACHTYPE),x86_64)
MACHTYPE = i386
endif
endif

ifeq ($(strip $(FORCE_WORD_SIZE)),64)
ifeq ($(MACHTYPE),i386)
MACHTYPE = x86_64
endif
endif


ECHO := echo
