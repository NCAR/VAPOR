#! /bin/sh 

arch=SYSTEM_ARCH
root=INSTALL_PREFIX_DIR
idl=BUILD_IDL_WRAPPERS
bindir=INSTALL_BINDIR
mandir=INSTALL_MANDIR
lib_search_dirs=LIB_SEARCH_DIRS

VAPOR_HOME="$root"; export VAPOR_HOME

if [ -z "${PATH}" ]
then
    PATH="$bindir"; export PATH
else
    PATH="$bindir:$PATH"; export PATH
fi

if [ "$arch" = "Darwin" ]
then
	if [ -z "${DYLD_FALLBACK_LIBRARY_PATH}" ]
	then
	    DYLD_FALLBACK_LIBRARY_PATH="${lib_search_dirs}"; export DYLD_FALLBACK_LIBRARY_PATH
	else
	    DYLD_FALLBACK_LIBRARY_PATH="${lib_search_dirs}:$DYLD_FALLBACK_LIBRARY_PATH"; export DYLD_FALLBACK_LIBRARY_PATH
	fi
else
if [ "$arch" = "AIX" ]
then
	if [ -z "${LIBPATH}" ]
	then
	    LIBPATH="${lib_search_dirs}"; export LIBPATH
	else
	    LIBPATH="${lib_search_dirs}:$LIBPATH"; export LIBPATH
	fi
else
	if [ -z "${LD_LIBRARY_PATH}" ]
	then
	    LD_LIBRARY_PATH="${lib_search_dirs}"; export LD_LIBRARY_PATH
	else
	    LD_LIBRARY_PATH="${lib_search_dirs}:$LD_LIBRARY_PATH"; export LD_LIBRARY_PATH
	fi
fi
fi


if [ -z "${MANPATH}" ]
then
	if [ "$arch" = "AIX" ]
	then
		MANPATH="$mandir"; export MANPATH
	else
		MANPATH="$mandir":$(man -w); export MANPATH
	fi
else
    MANPATH="$mandir:${MANPATH}"; export MANPATH
fi

if [ "$idl" -eq 1 ]
then
	if [ -z "${IDL_DLM_PATH}" ]
	then
		IDL_DLM_PATH="${lib_search_dirs}:<IDL_DEFAULT>"; export IDL_DLM_PATH
	else
		IDL_DLM_PATH="${lib_search_dirs}:$IDL_DLM_PATH"; export IDL_DLM_PATH
	fi
fi
