TOP = ../../..

include ${TOP}/make/config/prebase.mk



include ${TOP}/make/config/base.mk

ifeq ($(BUILD_DOXYGEN), 1)

all:: Doxyfile

Doxyfile: FRC
	@$(ECHO) Building documentation
	@sed -e "s#TOP_OF_TREE#$(TOP)#" -e s#DOCDIR#$(DOCDIR)/library# $@.in > $@ 
	@@doxygen $@

FRC:


INSTALL_DOC_DIR = $(INSTALL_SHAREDIR)/doc

install:: all
	@$(ECHO) "Installing Doxygen documentation in $(INSTALL_DOC_DIR)"
	@if test ! -d $(INSTALL_DOC_DIR); then $(MKDIR) $(INSTALL_DOC_DIR); fi
	cd $(DOCDIR); tar cf - library | (cd $(INSTALL_DOC_DIR); tar xf -)

endif

clobber::
	@$(ECHO) Removing doxygen output
	@-rm -fr $(DOCDIR)/*
