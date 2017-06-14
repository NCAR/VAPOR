TOP = ../..

include $(TOP)/make/config/prebase.mk

FFILES = $(wildcard *.ttf)

INSTALL_FONTS_DIR = $(INSTALL_SHAREDIR)/fonts

include $(TOP)/make/config/base.mk

install:: 
	@$(ECHO) "Installing font files $(FFILES) in $(INSTALL_FONTS_DIR)."
	@if test ! -d $(INSTALL_FONTS_DIR); then $(MKDIR) $(INSTALL_FONTS_DIR); fi
	@for i in $(FFILES); do $(INSTALL_NONEXEC) $$i $(INSTALL_FONTS_DIR); done
