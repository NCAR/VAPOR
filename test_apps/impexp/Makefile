TOP = ../..

include ${TOP}/make/config/prebase.mk

PROGRAM = testimpexp
FILES = testimpexp


LDFLAGS += $(EXPATLIB)

LIBRARIES = common vdf proj udunits2 netcdf 

CXXFLAGS += -I$(EXPAT_INC_PATH)

include ${TOP}/make/config/base.mk

