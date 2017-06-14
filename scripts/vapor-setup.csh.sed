#! /bin/csh -f

set arch = SYSTEM_ARCH
set root = INSTALL_PREFIX_DIR
set idl = BUILD_IDL_WRAPPERS
set bindir = INSTALL_BINDIR
set mandir = INSTALL_MANDIR
set lib_search_dirs = LIB_SEARCH_DIRS

setenv VAPOR_HOME $root

if !($?PATH) then
    setenv PATH "$bindir"
else
    setenv PATH "${bindir}:$PATH"
endif

if ( "$arch" == "Darwin" ) then
	if !($?DYLD_FALLBACK_LIBRARY_PATH) then
	    setenv DYLD_FALLBACK_LIBRARY_PATH "$lib_search_dirs"
	else
	    setenv DYLD_FALLBACK_LIBRARY_PATH "${lib_search_dirs}:$DYLD_FALLBACK_LIBRARY_PATH"
	endif
else if ( "$arch" == "AIX" ) then
	if !($?LIBPATH) then
	    setenv LIBPATH "$lib_search_dirs"
	else
	    setenv LIBPATH "${lib_search_dirs}:$LIBPATH"
	endif
else
	if !($?LD_LIBRARY_PATH) then
	    setenv LD_LIBRARY_PATH "$lib_search_dirs"
	else
	    setenv LD_LIBRARY_PATH "${lib_search_dirs}:$LD_LIBRARY_PATH"
	endif
endif

if !($?MANPATH) then
	if ( "$arch" == "AIX" ) then
		setenv MANPATH "$mandir"
	else
		setenv MANPATH "${mandir}":`man -w`
	endif
else
    setenv MANPATH "${mandir}:${MANPATH}"
endif

if ( "$idl" == 1 ) then
	if !($?IDL_DLM_PATH) then
		setenv IDL_DLM_PATH "${lib_search_dirs}:<IDL_DEFAULT>"
	else
		setenv IDL_DLM_PATH "${lib_search_dirs}:$IDL_DLM_PATH"
	endif
endif
