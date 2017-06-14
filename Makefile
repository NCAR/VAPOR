TOP = .

include $(TOP)/make/config/prebase.mk

SUBDIRS = lib apps include scripts share  buildutils

ifeq ($(BUILD_TESTAPPS), 1)
SUBDIRS += test_apps
endif

all::

install-dep:: 
	@$(ECHO) "Removing $(INSTALL_PREFIX_DIR)"
	@if test -d $(INSTALL_PREFIX_DIR); then $(RM) -r $(INSTALL_PREFIX_DIR); fi

include ${TOP}/make/config/base.mk


install-dep:: install
	sed -e s#ARCH#$(ARCH)# -e s#VERSION#$(VERSION)# < vapor-install.csh.sed > $(INSTALL_PREFIX_DIR)/vapor-install.csh
	chmod +x $(INSTALL_PREFIX_DIR)/vapor-install.csh



ifeq ($(BUILD_GUI), 1)

CLD_EXCLUDE_FLAGS = $(addprefix -exclude , $(CLD_EXCLUDE_LIBS))
CLD_INCLUDE_FLAGS = $(addprefix -include , $(CLD_INCLUDE_LIBS))
CLD_INCLUDE_FLAGS += -include ^$(QTDIR)/lib
LDLIBPATHS = -ldlibpath $(QTDIR)/lib

install-dep:: 
	@$(ECHO) "Copying Qt plugins to $(INSTALL_PLUGINSDIR)"
	@$(MAKE_INSTALL_PLUGINSDIR)
	$(CP) -R $(QTDIR)/plugins/* $(INSTALL_PLUGINSDIR)
	$(RM) $(INSTALL_PLUGINSDIR)/*/*debug*
	@$(ECHO) "Copying Qt plugin library dependencies to $(INSTALL_LIBDIR)"
	$(PERL) $(TOP)/buildutils/copylibdeps.pl -arch $(ARCH) $(LDLIBPATHS) $(CLD_EXCLUDE_FLAGS) $(CLD_INCLUDE_FLAGS) $(INSTALL_PLUGINSDIR)/*/* $(INSTALL_LIBDIR)
	@$(ECHO) "Copying Python modules $(INSTALL_LIBDIR)"
	$(CP) -R $(PYTHONDIR)/lib/python$(PYTHONVERSION) $(INSTALL_LIBDIR)
	$(CP) $(PYTHONDIR)/bin/python $(INSTALL_BINDIR)
	$(CP) -R $(PYTHONDIR)/include/python$(PYTHONVERSION) $(INSTALL_PREFIX_DIR)/include
	@$(ECHO) "Copying Python plugin library dependencies to $(INSTALL_LIBDIR)"
	$(PERL) $(TOP)/buildutils/copylibdeps.pl -arch $(ARCH) $(LDLIBPATHS) $(CLD_EXCLUDE_FLAGS) $(CLD_INCLUDE_FLAGS) $(INSTALL_LIBDIR)/python$(PYTHONVERSION)/./lib-dynload/*.so $(INSTALL_LIBDIR)/python$(PYTHONVERSION)/site-packages/numpy/linalg/*.so $(INSTALL_LIBDIR)/python$(PYTHONVERSION)/site-packages/scipy/linalg/*.so $(INSTALL_LIBDIR)
#	@$(MAKE_INSTALL_SHAREDIR)
#	@$(ECHO) "Copying GRIB_API share files to  $(INSTALL_SHAREDIR)"
#	$(CP) -R $(GRIB_API_DIR)/share/grib_api $(INSTALL_SHAREDIR)
	@$(ECHO) "Copying PROJ4 share files to  $(INSTALL_SHAREDIR)"
	@$(MAKE_INSTALL_SHAREDIR)
	$(CP) -R $(PROJ4_DIR)/share/proj $(INSTALL_SHAREDIR)

endif


#ifeq ($(ARCH), Linux)
#shlibs = $(wildcard $(INSTALL_LIBDIR)/lib*.so.*  $(INSTALL_PLUGINSDIR)/lib*.so)
#install-dep:: 
#	@$(ECHO) "Removing rpaths from shared libraries..."
#	@for i in $(shlibs); do echo "	$$i"; /usr/bin/patchelf --set-rpath "" $$i; done
#endif

ifeq ($(ARCH), Darwin)
CLD_INCLUDE_FLAGS = $(addprefix -include  ^, $(INSTALL_LIBDIR))
CLD_INCLUDE_FLAGS += $(addprefix -include  ^, $(LIB_SEARCH_DIRS))
CLD_INCLUDE_FLAGS += $(addprefix -include  ^, $(QTDIR)/lib)
install-dep:: 
	@$(ECHO) "Fixing shared library paths..."
	@$(PERL) $(TOP)/buildutils/install_name.pl $(CLD_INCLUDE_FLAGS) $(INSTALL_LIBDIR) $(INSTALL_PLUGINSDIR) $(INSTALL_BINDIR)

VAPOR_NAME = VAPOR3
endif


MAC_BUNDLE_DIR = /Users/clyne/tmp/vapor-macbundle

macbundle:: install-dep macbundle-scripts

macbundle-scripts:: 
	@if test -d $(MAC_BUNDLE_DIR); then $(RM) -r $(MAC_BUNDLE_DIR); fi
	@if test ! -d $(MAC_BUNDLE_DIR); then $(MKDIR) $(MAC_BUNDLE_DIR)/$(VAPOR_NAME); fi
	$(TOP)/buildutils/macbundle.pl $(INSTALL_PREFIX_DIR) $(TOP)/MacBundle/VAPOR/VAPOR.app $(MAC_BUNDLE_DIR)/$(VAPOR_NAME) $(VERSION) 
	if test ! -d $(MAC_BUNDLE_DIR)/Install_Resources; then $(MKDIR) $(MAC_BUNDLE_DIR)/Install_Resources; fi
	$(CP) $(TOP)/buildutils/postflight $(MAC_BUNDLE_DIR)/Install_Resources
	$(CP) $(TOP)/buildutils/postupgrade $(MAC_BUNDLE_DIR)/Install_Resources
	$(CP) $(TOP)/buildutils/postinstall $(MAC_BUNDLE_DIR)/Install_Resources
	$(CP) $(TOP)/Images/splash.jpg $(MAC_BUNDLE_DIR)/Install_Resources
	$(MKDIR) $(MAC_BUNDLE_DIR)/$(VAPOR_NAME)/VAPOR.app/Contents/MacOS/lib && $(MV) $(MAC_BUNDLE_DIR)/$(VAPOR_NAME)/VAPOR.app/Contents/MacOS/python$(PYTHONVERSION) $(MAC_BUNDLE_DIR)/$(VAPOR_NAME)/VAPOR.app/Contents/MacOS/lib
