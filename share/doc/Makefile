TOP = ../..

include $(TOP)/make/config/prebase.mk

SUBDIRS = help
ifeq ($(BUILD_DOXYGEN),1)
SUBDIRS += Doxygen 
endif

INSTALL_DOC_DIR = $(INSTALL_SHAREDIR)/doc


include $(TOP)/make/config/base.mk

install::
	@if test ! -d $(INSTALL_DOC_DIR); then $(MKDIR) $(INSTALL_DOC_DIR); fi
	@for i in $(DOCS); do $(INSTALL_NONEXEC) $$i $(INSTALL_DOC_DIR); done
