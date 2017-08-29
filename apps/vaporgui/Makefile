TOP = ../..

include ${TOP}/make/config/prebase.mk
PROGRAM	= vaporgui
export PROGRAM

FILES = \
	EventRouter RenderEventRouter RenderHolder ImageFrame ControlPointEditor \
	BoxSliderFrame DomainWidget GLWidget OpacityWidget Histo \
	ColorbarWidget StartupEventRouter AppSettingsEventRouter \
	VizSelectCombo VizFeatureEventRouter \
	AnimationEventRouter MappingFrame HelloEventRouter \
	regioneventrouter ViewpointEventRouter \
	TwoDDataEventRouter ImageEventRouter \
	RangeController Statistics Plot SeedMe \
	helloSubtabs TFWidget VariablesWidget \
	GeometryWidget RangeCombos BarbEventRouter \
	BannerGUI AnimationParams \
	AppSettingsParams GUIStateParams MouseModeParams StartupParams \
	main MainForm VizWin VizWinMgr \
	TabManager TrackBall GLColorbarWidget ErrorReporter

QT_HEADERS := \
	RenderHolder BoxSliderFrame DomainWidget GLWidget \
	OpacityWidget \
	ControlPointEditor ColorbarWidget ImageFrame \
	VizSelectCombo \
	MappingFrame VizFeatureEventRouter \
	AnimationEventRouter HelloEventRouter \
	StartupEventRouter AppSettingsEventRouter\
	regioneventrouter ViewpointEventRouter \
	TwoDDataEventRouter ImageEventRouter \
	RangeController Statistics Plot SeedMe \
	TFWidget VariablesWidget \
	helloSubtabs TwoDSubtabs GeometryWidget RangeCombos \
	BarbEventRouter BarbSubtabs \
	BannerGUI AnimationParams \
	MainForm TabManager VizWinMgr VizWin  TrackBall GLColorbarWidget ErrorReporter \
	ImageSubtabs 


ifneq ($(ARCH),WIN32)
MAKEFILE_CXXFLAGS += -DPYTHONVERSION=\"$(PYTHONVERSION)\"
MAKEFILE_CXXFLAGS += -DPYTHONDIR=\"$(PYTHONDIR)\"
endif


MOC_DIR = moc
UI_DIR = ui
include $(UI_DIR)/ui.mk
SHARED = 0
QT = 1


LIBRARIES = dl util m render params jpeg vdc wasp common 
LIBRARIES += $(QT_LIB) GLEW tiff geotiff python$(PYTHONVERSION)
LIBRARIES += assimp

ifneq ($(ARCH),Darwin)
LIBRARIES += QtOpenGL $(GLU_LIB) $(OGL_LIB)
MAKEFILE_LDFLAGS += -Xlinker -export-dynamic
endif


ifeq ($(ARCH),Darwin)

MAKEFILE_LDFLAGS += -framework CoreFoundation -framework CoreServices 
MAKEFILE_LDFLAGS += -framework Cocoa -framework AppKit -framework 
MAKEFILE_LDFLAGS += ApplicationServices -framework OpenGL -framework AGL
MAKEFILE_LDFLAGS += -u _PyMac_Error 

ifeq ($(QT_FRAMEWORK), 1)
MAKEFILE_LDFLAGS += -F$(QTDIR)/lib -framework QtCore -framework QtGui 
MAKEFILE_LDFLAGS += -framework QtOpenGL
endif

MAKEFILE_CXXFLAGS += -Wno-unused-variable -Wno-non-virtual-dtor

endif   

include ${TOP}/make/config/base.mk


cat:
	echo "QTDIR=$(QTDIR)"

