#
#
# See the file LICENSE.txt for information on redistributing this software.

# This file lets one set various compile-time options.

#
#
#


# Installation prefix directory. Vapor executables will be installed
# in $(INSTALL_PREFIX_DIR)/bin, libraries in $(INSTALL_PREFIX_DIR)/lib
# etc,.
#
INSTALL_PREFIX_DIR=/usr/local

# Set RELEASE to 1 to compile with optimizations and without debug info.
RELEASE=0

# Set DEBUG to 1 if you want diagnostic messages turned on
DEBUG=0

#
# Specify an alternate C++ and C compiler. The default compiler is 
# platform dependent
#
# CXX = 
# CC = 

# Set BENCHMARK or PROFILE to 1 for framerate diagnostics
BENCHMARK=0
PROFILING=0

# Set LIB_SEARCH_DIRS to a list of directories containing libraries
# not on the default search path for the linker. Typically 3rd party 
# dependencies (e.g. netCDF) are not installed in a location where 
# linker normally checks. The linker will search the directories
# in the order specified.
#
LIB_SEARCH_DIRS =  /opt/local/lib

# Set INC_SEARCH_DIRS to a list of directories containing include files
# not on the default search path for the compiler. Typically 3rd party 
# dependencies (e.g. netCDF, IDL) are not installed in a location where 
# compiler normally checks. The compiler will search the directories 
# in the order specified.  
INC_SEARCH_DIRS = /opt/local/include

# Set NETCDF_LIBS to the name of your netCDF library and any library
# dependencies that netcdf has. The default is simply 'netcdf'. Starting
# with netCDF version 4, depending on the compile time options used to
# build netCDF, one or more additional libraries (e.g. hdf and hdf5_hl)
# might be needed
#
NETCDF_LIBS = netcdf

# Set FREETYPE2_INC_DIR to the path to the Freetype2 directory if not
# in a standard location. This is the directory that contains the file
# ft2build.h
FREETYPE2_INC_DIR = 

# Set to 1 if you want the VAPoR VDC library to be built. Otherwise only the 
# WASP libraries and support utilities are compiled
#
BUILD_VDC = 0

# Set to 1 if you want the VAPoR GUI to be built. Otherwise only the 
# VAPoR libraries and support utilities are compiled
#
BUILD_GUI = 0

# If BUILD_GUI is set to 1, set QTDIR to the root of the QT directory 
# where the sub directories 'bin', 'lib', and 'include' may be found. Qt 
# refers to Trolltech's Qt, available (with some amount of hunting) from
# http://www.trolltech.com, and when possible, from the vapor
# web site: www.vapor.ucar.edu. Qt version 4.6.1 or higher is required.
#
QTDIR =

# If BUILD_GUI is set to 1 **and** this is a Mac system, set HAVE_QT_FRAMEWORK
# to 1 if your Qt libraries are built as Mac Frameworks, or to 0 if 
# your Qt libraries are built as older UNIX style dynamic libraries.
#
HAVE_QT_FRAMEWORK = 1

# Set to 1 if you have IDL installed on your system and you would
# like to build the VAPoR IDL commands. IDL refers to RSI's IDL, available
# for fee from www.rsinc.com. IDL is not required by vapor, however the 
# analysis and data processing capabilities of vapor are greatly limitted
# without IDL
#
BUILD_IDL_WRAPPERS = 0

# If BUILD_IDL_WRAPPERS is set to 1, set IDLDIR to the root of the 
# IDL directory 
# where the sub directories 'bin', 'lib', and 'include' may be found. 
#
IDLDIR =

# Set PYTHONDIR to the root of the 
# Python directory where the sub directories 'lib', and 'include' 
# may be found. Python version 2.6.5 or higher is required. 
#
PYTHONDIR =

# If BUILD_PYTHON is set to 1, set PYTHONVERSION to the version 
# number (both major and minor, e.g. "2.6") of Python.
# The build system will look for Python modules and include files
# under $PYTHONDIR/lib/python${PYTHONVERSION} and 
# $PYTHONDIR/include/python${PYTHONVERSION}, respectively.
#
PYTHONVERSION = 2.7


# Set to 1 if you want to add support for Adaptive Mesh Refinement grids
#
BUILD_AMR_SUPPORT = 1

# Set to 1 if you want to add support for displaying 3D model geometry
# within a vaporgui scene
#
BUILD_MODELS = 1

# Set to 1 if you want to generate documentation for programmatic API
# with Doxygen (requires doxygen command)
#
BUILD_DOXYGEN = 0

# Uncomment the line below to use assimp 2.x.
# Default is to use assimp 3.x
#
# ASSIMP_2 = 1

##
##
##	PLATFORM SPECIFIC MACROS
##
##


##
##	Linux
##

# Set to 1 if intel compilers are available on linux systems and you
# wish to compile with them instead of gcc
#
HAVE_INTEL_COMPILERS = 

##
##	Linux & Mac
##

# Set WORD_SIZE to either '32' or '64' to force compilation of 32bit or 64bit
# code, respectively. By default, the build system attempts to 
# detect the word size of the processor (32 or 64 bit) and generate
# code accordinly. 
#
FORCE_WORD_SIZE = 

#
#	If the file `site.mk' exists, include it. It contains site-specific
#	(host or platform specific) make variables that may override 
#	values defined above. The site.mk file is NOT part of the vapor
#	distribution. But you may define one that sets the variables above
#	based on host name, OS, etc.
#
-include $(TOP)/site.mk

