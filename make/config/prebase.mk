ABS_TOP = $(abspath $(TOP))

# abspath doesn't work on older versions of gmake
ifeq ($(ABS_TOP), )
	ABS_TOP = $(shell cd $(TOP) && pwd)
endif

SHARED = 1

include $(TOP)/make/config/arch.mk
include $(TOP)/make/config/$(ARCH).mk
include $(TOP)/options.mk

ifeq ($(BUILD_MODELS),1)
MAKEFILE_CXXFLAGS += -DMODELS
endif

ifeq ($(BUILD_FLOW),1)
MAKEFILE_CXXFLAGS += -DFLOW
endif

