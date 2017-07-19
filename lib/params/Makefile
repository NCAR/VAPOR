TOP = ../..

include ${TOP}/make/config/prebase.mk

LIBRARY = params
FILES = \
	XmlNode ParamsBase ColorMap OpacityMap MapperFunction \
	TransferFunction Box ColorbarPbase \
	RenderParams TwoDDataParams \
	BarbParams \
	Viewpoint ViewpointParams regionparams \
	ParamsMgr DataStatus VizFeatureParams \
	HelloParams TFInterpolator

#arrowparams

ifeq ($(BUILD_FLOW),1)
FILES += streamparams
endif

ifeq ($(ARCH),WIN32)
MAKEFILE_CXXFLAGS += /D"PARAMS_EXPORTS"
endif


#ifeq ($(ARCH),Darwin)
#MAKEFILE_INCLUDE_DIRS += -I/System/Library/Frameworks/OpenGL.framework/Headers -I/System/Library/Frameworks/AGL.framework/Headers
#MAKEFILE_CXXFLAGS += -Wno-unused-variable -Wno-non-virtual-dtor
#endif


include ${TOP}/make/config/base.mk

