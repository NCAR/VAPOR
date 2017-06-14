TOP = ../

include $(TOP)/make/config/prebase.mk


SUBDIRS = vaporversion raw2wasp wasp2raw waspcreate ncdf2wasp wasp2ncdf

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
SUBDIRS += cf2vdc cfvdccreate raw2vdc vdc2raw vdccreate wrf2vdc wrfvdccreate vdcdump
endif

ifeq ($(BUILD_GUI), 1)
SUBDIRS += vaporgui tiff2geotiff
endif

include ${TOP}/make/config/base.mk

