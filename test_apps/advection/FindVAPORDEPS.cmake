# Try to find VAPORDEPS headers and libraries.
#
# Usage of this module as follows:
#
#     find_package(VAPORDEPS)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  VAPORDEPS_PREFIX         Set this variable to the root installation of
#                      libpapi if the module has problems finding the
#                      proper installation path.
#
# Variables defined by this module:
#
#  VAPORDEPS_FOUND              System has VAPOR libraries and headers
#  VAPORDEPS_LIBRARIES          The VAPOR library
#  VAPORDEPS_INCLUDE_DIRS       The location of VAPOR headers

find_path(VAPORDEPS_PREFIX
    NAMES include
    NO_DEFAULT_PATH
#    DOC "Path to the root directory of the VAPORDEPS installation"
)

find_path(VAPORDEPS_INCLUDE_DIR
    NAMES glm/glm.hpp
    HINTS ${VAPORDEPS_PREFIX}/include
#    DOC "Path to the VAPORDEPS include directory [should autocomplete given prefix]"
)

#find_library(FLOW NAMES flow libflow HINTS ${VAPORDEPS_PREFIX}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VAPORDEPS DEFAULT_MSG
  VAPORDEPS_INCLUDE_DIR
)

mark_as_advanced(
    VAPORDEPS_PREFIX VAPOR_INCLUDE_DIR
)

set(VAPORDEPS_INCLUDE_DIRS ${VAPORDEPS_INCLUDE_DIR})
#set(VAPORDEPS_LIBRARIES ${FLOW} ${COMMON} ${PARAMS} ${REMER} ${VDC} ${WASP})


