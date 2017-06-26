cmake_minimum_required (VERSION 3.1)
project (VAPOR)

set (CMAKE_CXX_STANDARD 11)

# set(CMAKE_VERBOSE_MAKEFILE on)

# These need to be updatd to change for install build
set (DSO_DIR ${PROJECT_BINARY_DIR}/bin)
set (BINDIR ${PROJECT_BINARY_DIR}/bin)
set (ABS_TOP ${PROJECT_SOURCE_DIR})
set (QTDIR /glade/p/VAST/VAPOR/third-party/apps-2017/Darwin_x86_64)
set (PYTHONDIR /glade/p/VAST/VAPOR/third-party/apps-2017/Darwin_x86_64)
set (PYTHONVERSION 2.7)

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

set (THIRD_PARTY_LIB_DIR "/glade/p/VAST/VAPOR/third-party/apps-2017/Darwin_x86_64/lib")
set (THIRD_PARTY_INC_DIR "/glade/p/VAST/VAPOR/third-party/apps-2017/Darwin_x86_64/include")

include_directories (${THIRD_PARTY_INC_DIR})
link_directories (${THIRD_PARTY_LIB_DIR})
list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_LIB_DIR})
LIST (APPEND CMAKE_PROGRAM_PATH ${QTDIR}/bin)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory (apps)
add_subdirectory (lib)
add_subdirectory (include)
add_subdirectory (share/Doxygen)

