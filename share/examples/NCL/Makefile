TOP = ../../..

include ${TOP}/make/config/prebase.mk

NCL_EXAMPLES = $(wildcard *ncl) WrfTestScripts.Notes

INSTALL_NCL_EX_DIR = $(INSTALL_SHAREDIR)/examples/NCL

include ${TOP}/make/config/base.mk

install:: 
	@$(ECHO) "Installing NCL example files $(NCL_EXAMPLES) in $(INSTALL_NCL_EX_DIR)."
	@if test ! -d $(INSTALL_NCL_EX_DIR); then $(MKDIR) $(INSTALL_NCL_EX_DIR); fi
	@for i in $(NCL_EXAMPLES); do $(INSTALL_NONEXEC) $$i $(INSTALL_NCL_EX_DIR); done
