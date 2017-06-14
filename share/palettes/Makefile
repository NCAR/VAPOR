TOP = ../..

include $(TOP)/make/config/prebase.mk

TFS = $(wildcard *tf3)

INSTALL_PALETTE_DIR = $(INSTALL_SHAREDIR)/palettes

include $(TOP)/make/config/base.mk

install:: 
	@$(ECHO) "Installing color palette files $(TFS) in $(INSTALL_PALETTE_DIR)."
	@if test ! -d $(INSTALL_PALETTE_DIR); then $(MKDIR) $(INSTALL_PALETTE_DIR); fi
	@for i in $(TFS); do $(INSTALL_NONEXEC) $$i $(INSTALL_PALETTE_DIR); done
