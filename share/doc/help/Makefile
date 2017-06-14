TOP = ../../..

include $(TOP)/make/config/prebase.mk


DOCS = FieldLineAdvectionHelp.html  UnsteadyHelp.html

INSTALL_DOC_DIR = $(INSTALL_SHAREDIR)/doc

include ${TOP}/make/config/base.mk

install::
	@$(ECHO) "Installing documentation files $(DOCS) in $(INSTALL_DOC_DIR)"
	@if test ! -d $(INSTALL_DOC_DIR); then $(MKDIR) $(INSTALL_DOC_DIR); fi
	@for i in $(DOCS); do $(INSTALL_NONEXEC) $$i $(INSTALL_DOC_DIR); done
