TOP = ../..

include ${TOP}/make/config/prebase.mk

PROGRAM = test_amrdata
FILES = test_amrdata flashhdf5


LIBRARIES = common vdf

MAKEFILE_CXXFLAGS += -I/fs/local/include -I$(EXPAT_INC_PATH)
MAKEFILE_LDFLAGS += -L/fs/local/lib -lhdf5 $(EXPATLIB) -L$(NETCDF_LIB_PATH) -lnetcdf -lnetcdf_c++

include ${TOP}/make/config/base.mk
