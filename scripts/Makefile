TOP = ..

include ${TOP}/make/config/prebase.mk


include ${TOP}/make/config/base.mk

all:: $(BINDIR)/vapor-setup.sh $(BINDIR)/vapor-setup.csh vdfbkup.pl \
getWMSImage.sh 

libs =  $(shell echo $(strip $(INSTALL_LIBDIR) $(LIB_SEARCH_DIRS) $(QTDIR)/lib) | sed -e  's/ /\:/g')
$(BINDIR)/vapor-setup.sh: FRC
	sed -e s#SYSTEM_ARCH#$(ARCH)# \
		-e s#INSTALL_PREFIX_DIR#$(INSTALL_PREFIX_DIR)# \
		-e s#INSTALL_BINDIR#$(INSTALL_BINDIR)# \
		-e s#INSTALL_MANDIR#$(INSTALL_MANDIR)# \
		-e s#LIB_SEARCH_DIRS#$(libs)# \
		-e s#BUILD_IDL_WRAPPERS#$(BUILD_IDL_WRAPPERS)# \
		< vapor-setup.sh.sed > $@

vdfbkup.pl: FRC

install:: $(BINDIR)/vapor-setup.sh vdfbkup.pl getWMSImage.sh
	@$(INSTALL_EXEC) $(BINDIR)/vapor-setup.sh $(INSTALL_BINDIR)
	@$(INSTALL_EXEC) vdfbkup.pl $(INSTALL_BINDIR)
	@$(INSTALL_EXEC) getWMSImage.sh $(INSTALL_BINDIR)
	@$(INSTALL_EXEC) vdccp.py $(INSTALL_BINDIR)

$(BINDIR)/vapor-setup.csh: FRC
	sed -e s#SYSTEM_ARCH#$(ARCH)# \
		-e s#INSTALL_PREFIX_DIR#$(INSTALL_PREFIX_DIR)# \
		-e s#INSTALL_BINDIR#$(INSTALL_BINDIR)# \
		-e s#INSTALL_MANDIR#$(INSTALL_MANDIR)# \
		-e s#LIB_SEARCH_DIRS#$(libs)# \
		-e s#BUILD_IDL_WRAPPERS#$(BUILD_IDL_WRAPPERS)# \
		< vapor-setup.csh.sed > $@

FRC:

install:: $(BINDIR)/vapor-setup.csh
	@$(INSTALL_EXEC) $(BINDIR)/vapor-setup.csh $(INSTALL_BINDIR)

clean::
	@$(RM) $(BINDIR)/vapor-setup.sh $(BINDIR)/vapor-setup.csh 

subdirs::

clobber::	clean
