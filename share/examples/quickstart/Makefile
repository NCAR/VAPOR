TOP = ../../..

include ${TOP}/make/config/prebase.mk

QUICKSTART_VSS = 8spothighres.vss guideFigure1.10.vss guideFigure1.11.vss \
	guideFigure1.5.vss guideFigure1.7.vss guideFigure1.8.vss \
	guideFigure2.1.vss guideFigure2.10.vss guideFigure2.2.vss \
	guideFigure2.3.vss guideFigure2.4.vss guideFigure2.5.vss \
	guideFigure2.6.vss guideFigure2.7.vss guideFigure2.8.vss \
	guideFigure2.9.vss guideFigure3.1.vss guideFigure3.2.vss \
	guideFigure3.3.vss guideFigure3.4.vss guideFigure3.5.vss \
	guideFigure3.6.vss guideFigure3.7.vss guideFigure3.8.vss \
	guideFigure3.9.vss guideFigure4.1.vss guideFigure4.10.vss \
	guideFigure4.2.vss guideFigure4.3.vss guideFigure4.4.vss \
	guideFigure4.5.vss guideFigure4.6.vss guideFigure4.7.vss \
	guideFigure4.8.vss guideFigure4.9.vss guideFigure5.1.vss \
	guideFigure5.2.vss guideFigure5.3.vss guideFigure5.4.vss \
	guideFigure5.5.vss saved8spot.vss


INSTALL_QS_VSS_DIR = $(INSTALL_SHAREDIR)/examples/quickstart

include ${TOP}/make/config/base.mk

install:: 
	@$(ECHO) "Installing Quickstart session files $(QUICKSTART_VSS) in $(INSTALL_QS_VSS_DIR)."
	@if test ! -d $(INSTALL_QS_VSS_DIR); then $(MKDIR) $(INSTALL_QS_VSS_DIR); fi
	@for i in $(QUICKSTART_VSS); do $(INSTALL_NONEXEC) $$i $(INSTALL_QS_VSS_DIR); done
