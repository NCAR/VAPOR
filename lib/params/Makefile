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
	HelloParams TFInterpolator \
  ImageParams ContourParams

#arrowparams

ifeq ($(BUILD_FLOW),1)
FILES += streamparams
endif

ifeq ($(ARCH),WIN32)
MAKEFILE_CXXFLAGS += /D"PARAMS_EXPORTS"
endif


include ${TOP}/make/config/base.mk

