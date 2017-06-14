TOP = ../..

include ${TOP}/make/config/prebase.mk

LIBRARY = common
FILES = MyBase OptionParser EasyThreads CFuncs Version PVTime \
	GetAppPath utils 


ifneq ($(ARCH),WIN32)
MAKEFILE_CXXFLAGS += -DBINDIR=\"$(BINDIR)\"
MAKEFILE_CXXFLAGS += -DDSO_DIR=\"$(DSO_DIR)\"
MAKEFILE_CXXFLAGS += -DABS_TOP=\"$(ABS_TOP)\"
MAKEFILE_CXXFLAGS += -DQTDIR=\"$(QTDIR)\"
endif

ifneq ($(ARCH),WIN32)
MAKEFILE_CXXFLAGS += -DENABLE_THREADS
endif

ifneq ($(ARCH),Darwin)
LIBRARIES += GL GLU
endif

#
# Force version to be updated before other processing
#
all:: VERSION


include ${TOP}/make/config/base.mk


VERSION: $(INCDIR)/$(PROJECT)/Version.h

DATE = $(shell date)

$(INCDIR)/$(PROJECT)/Version.h:: Version.h.sed  $(TOP)/Version 
	$(ECHO) "Rebuilding $(INCDIR)/$(PROJECT)/Version.h"
	sed -e 's/MAJOR/$(VERSION_MAJOR)/' -e 's/MINOR/$(VERSION_MINOR)/' -e 's/MICRO/$(VERSION_MICRO)/' -e 's/VERSION_RC/\"$(VERSION_RC)\"/' -e 's/DATE/\"$(DATE)\"/' < Version.h.sed > $(INCDIR)/$(PROJECT)/Version.h
