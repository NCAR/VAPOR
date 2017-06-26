cmake_minimum_required (VERSION 3.1)
project (VAPOR)

set (CMAKE_CXX_STANDARD 11)

# set(CMAKE_VERBOSE_MAKEFILE on)

set (VERSION_MAJOR 3)
set (VERSION_MINOR 0)
set (VERSION_MICRO 0)
set (VERSION_RC alpha0)
string (TIMESTAMP VERSION_DATE UTC)
set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_RC})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_definitions (-DDarwin)
endif()


option (BUILD_VDC "Build VDC library and utilities" ON)
option (BUILD_GUI "Build Vapor GUI" ON)
option (BUILD_DOC "Build Vapor Doxygen documentation" ON)

include_directories ("${PROJECT_BINARY_DIR}/include")
include_directories ("${PROJECT_SOURCE_DIR}/include")

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# These need to be updatd to change for install build
set (DSO_DIR ${PROJECT_BINARY_DIR}/bin)
set (BINDIR ${PROJECT_BINARY_DIR}/bin)
set (ABS_TOP ${PROJECT_SOURCE_DIR})

set (QTDIR )
set (PYTHONDIR )
set (PYTHONVERSION )
set (THIRD_PARTY_LIB_DIR )
set (THIRD_PARTY_INC_DIR )

include (site.local OPTIONAL)

include_directories (${THIRD_PARTY_INC_DIR})
link_directories (${THIRD_PARTY_LIB_DIR})
list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_LIB_DIR})
LIST (APPEND CMAKE_PROGRAM_PATH ${QTDIR}/bin)

add_subdirectory (apps)
add_subdirectory (lib)
add_subdirectory (include)
add_subdirectory (share/Doxygen)

