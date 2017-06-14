TOP = ../..

include ${TOP}/make/config/prebase.mk

MOC_DIR = moc
UI_DIR = ui
SHARED = 0
QT = 1
PROGRAM = test_CE
FILES = test_CE test_vizwin
MAKEFILE_INCLUDE_DIRS += -I../../lib/params -I../../lib/render

LIBRARIES = vdc wasp common params render $(QT_LIB) GLEW jpeg tiff 

ifeq ($(ARCH),Darwin)

MAKEFILE_LDFLAGS += -framework CoreFoundation -framework CoreServices
MAKEFILE_LDFLAGS += -framework Cocoa -framework AppKit -framework
MAKEFILE_LDFLAGS += ApplicationServices -framework OpenGL -framework AGL

ifeq ($(QT_FRAMEWORK), 1)
MAKEFILE_LDFLAGS += -F$(QTDIR)/lib -framework QtCore -framework QtGui
MAKEFILE_LDFLAGS += -framework QtOpenGL
endif
MAKEFILE_CXXFLAGS += -Wno-unused-variable -Wno-non-virtual-dtor
endif


QT_HEADERS := test_vizwin

include ${TOP}/make/config/base.mk

