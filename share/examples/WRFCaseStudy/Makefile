TOP = ../../..

include ${TOP}/make/config/prebase.mk

WRF_VSS = WRFFigure1.8.vss WRFFigure1.9.vss \
	WRFFigure1.10.vss \
	WRFFigure1.13.vss WRFFigure1.14.vss WRFFigure1.15.vss \
	WRFFigure1.16.vss WRFFigure1.17.vss WRFFigure1.18.vss \
	WRFFigure2.1.vss WRFFigure2.3.vss \
	WRFFigure2.4.vss WRFFigure2.5.vss WRFFigure2.6.vss \
	WRFFigure2.7.vss WRFFigure3.1.vss WRFFigure3.2.vss \
	WRFFigure3.3.vss  \
	WRFFigure4.1.vss WRFFigure4.2.vss WRFFigure4.3.vss \
	WRFFigure4.4.vss WRFFigure4.5.vss WRFFigure4.6.vss \
	WRFFigure4.7.vss WRFFigure4.8.vss WRFFigure4.9.vss \
	WRFFigure4.10.vss WRFFigure4.11.vss WRFFigure4.12.vss \
	WRFFigure4.13.vss \
	WRFFigure5.1.vss WRFFigure5.2.vss WRFFigure5.3.vss \
	WRFFigure5.4.vss WRFFigure5.5.vss WRFFigure5.6.vss \
	WRFFigure5.7.vss

INSTALL_WRF_VSS_DIR = $(INSTALL_SHAREDIR)/examples/WRFCaseStudy

include ${TOP}/make/config/base.mk

install:: 
	@$(ECHO) "Installing WRFCaseStudy session files $(WRF_VSS) in $(INSTALL_WRF_VSS_DIR)."
	@if test ! -d $(INSTALL_WRF_VSS_DIR); then $(MKDIR) $(INSTALL_WRF_VSS_DIR); fi
	@for i in $(WRF_VSS); do $(INSTALL_NONEXEC) $$i $(INSTALL_WRF_VSS_DIR); done
