# Try to find VAPOR headers and libraries.
#
# Usage of this module as follows:
#
#     find_package(VAPOR)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  VAPOR_PREFIX         Set this variable to the root installation of
#                      libpapi if the module has problems finding the
#                      proper installation path.
#
# Variables defined by this module:
#
#  VAPOR_FOUND              System has VAPOR libraries and headers
#  VAPOR_LIBRARIES          The VAPOR library
#  VAPOR_INCLUDE_DIRS       The location of VAPOR headers

find_path(VAPOR_PREFIX
    NAMES install vapor
    NO_DEFAULT_PATH
#    DOC "Path to the root directory of the VAPOR installation"
)

find_path(VAPOR_INCLUDE_DIR
    NAMES vapor/Advection.h
    HINTS ${VAPOR_PREFIX}/include
#    DOC "Path to the VAPOR include directory [should autocomplete given prefix]"
)

find_library(FLOW NAMES flow libflow HINTS ${VAPOR_PREFIX}/lib)
find_library(COMMON NAMES common libcommon HINTS ${VAPOR_PREFIX}/lib)
find_library(PARAMS NAMES params libparams HINTS ${VAPOR_PREFIX}/lib)
find_library(RENDER NAMES render librender HINTS ${VAPOR_PREFIX}/lib)
find_library(VDC NAMES vdc libvdc HINTS ${VAPOR_PREFIX}/lib)
find_library(WASP NAMES wasp libwasp HINTS ${VAPOR_PREFIX}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VAPOR DEFAULT_MSG
  FLOW COMMON PARAMS RENDER VDC WASP VAPOR_INCLUDE_DIR
)

mark_as_advanced(
    VAPOR_PREFIX FLOW COMMON PARAMS RENDER VDC WASP VAPOR_INCLUDE_DIR
)

set(VAPOR_INCLUDE_DIRS ${VAPOR_INCLUDE_DIR})
set(VAPOR_LIBRARIES ${FLOW} ${COMMON} ${PARAMS} ${REMER} ${VDC} ${WASP})
