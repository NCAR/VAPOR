TOP = ../

include ${TOP}/make/config/prebase.mk


include ${TOP}/make/config/base.mk

HEADER_FILES = $(wildcard vapor/*.h)

all::

install:: all
	@$(ECHO) "Installing header files $(HEADER_FILES) in $(INSTALL_INCDIR)."
	@$(MAKE_INSTALL_INCDIR)
	@for i in $(HEADER_FILES); do $(INSTALL_NONEXEC) $$i $(INSTALL_INCDIR); done


clean::

clobber::
