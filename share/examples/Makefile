TOP = ../..

include $(TOP)/make/config/prebase.mk

SUBDIRS += idl
SUBDIRS += quickstart
SUBDIRS += WRFCaseStudy
SUBDIRS += VDC
SUBDIRS += NCL

include ${TOP}/make/config/base.mk

INSTALL_EX_DIR = $(INSTALL_SHAREDIR)/examples
install::
	@$(ECHO) "Installing default preferences file in $(INSTALL_EX_DIR)."
	@if test ! -d $(INSTALL_EX_DIR); then $(MKDIR) $(INSTALL_EX_DIR); fi
	$(INSTALL_NONEXEC) vapor_prefs $(INSTALL_EX_DIR)/.vapor_prefs




