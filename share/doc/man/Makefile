TOP = ../../..

include ${TOP}/make/config/prebase.mk


include ${TOP}/make/config/base.mk

DOCS = $(subst .pod, , $(wildcard *.pod))

all::
	@$(ECHO) "Building man pages"
	@if test ! -d $(DOCDIR)/commands/html; then $(MKDIR) $(DOCDIR)/commands/html; fi
	@for i in $(DOCS); do echo "Processing $$i.pod"; pod2html --infile $$i.pod --outfile $(DOCDIR)/commands/html/$$i.html; done
	@if test ! -d $(DOCDIR)/commands/man1; then $(MKDIR) $(DOCDIR)/commands/man1; fi
	@for i in $(DOCS); do echo "Processing $$i.pod"; pod2man $$i.pod $(DOCDIR)/commands/man1/$$i.1; done


install:: all
	@$(ECHO) "Installing man pages in $(INSTALL_MANDIR)/man1."
	@if test ! -d $(INSTALL_MANDIR)/man1; then $(MKDIR) $(INSTALL_MANDIR)/man1; fi
	@for i in $(DOCS); do $(INSTALL_NONEXEC) $(DOCDIR)/commands/man1/$$i.1 $(INSTALL_MANDIR)/man1; done


clean::
	@if test ! -d  pod2htmd.tmp; then $(RM) pod2htmd.tmp; fi
	@if test ! -d  pod2htmi.tmp; then $(RM) pod2htmi.tmp; fi

clobber::
