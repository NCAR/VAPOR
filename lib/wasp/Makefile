TOP = ../..

include ${TOP}/make/config/prebase.mk

LIBRARY = wasp
FILES = \
	WaveFiltBase  WaveFiltBior  WaveFiltDaub  WaveFiltCoif \
	WaveFiltHaar WaveFiltInt MatWaveBase  MatWaveDwt MatWaveWavedec  \
	Compressor NetCDFCpp WASP SignificanceMap
	

LIBRARIES += netcdf


#LIB_DEFS = vdf.def

ifeq ($(ARCH),WIN32)
MAKEFILE_CXXFLAGS += /D"VDF_EXPORTS" 
MAKEFILE_CXXFLAGS += /D"DLL_NETCDF"
endif

include ${TOP}/make/config/base.mk
