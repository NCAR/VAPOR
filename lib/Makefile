TOP = ../

include $(TOP)/make/config/prebase.mk


SUBDIRS = common wasp

# if BUILD_GUI or BUILD_VDC. No logical 'or' in gmake :-(
#
do_vdc = 
ifeq ($(BUILD_GUI), 1)
do_vdc = 1
endif

ifeq ($(BUILD_VDC), 1)
do_vdc = 1
endif

ifeq ($(do_vdc), 1)
SUBDIRS += vdc
endif

ifeq ($(BUILD_GUI), 1)
SUBDIRS += params
ifeq ($(BUILD_FLOW),1)
SUBDIRS += flow
endif
SUBDIRS += render
endif

include ${TOP}/make/config/base.mk

