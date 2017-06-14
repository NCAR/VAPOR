TOP = ../..

include $(TOP)/make/config/prebase.mk


INSTALL_UDUNITS_DIR = $(INSTALL_SHAREDIR)/udunits

XMLFILES = $(wildcard *.xml)

include $(TOP)/make/config/base.mk

install:: 
	@$(ECHO) "Installing udunits2 xml files $(XMLFILES) in $(INSTALL_UDUNITS_DIR)."
	@if test ! -d $(INSTALL_UDUNITS_DIR); then $(MKDIR) $(INSTALL_UDUNITS_DIR); fi
	@for i in $(XMLFILES); do $(INSTALL_NONEXEC) $$i $(INSTALL_UDUNITS_DIR); done
