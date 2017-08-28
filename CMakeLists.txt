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
elseif (WIN32)
	add_definitions (-DWIN32)
endif()


option (BUILD_VDC "Build VDC library and utilities" ON)
option (BUILD_GUI "Build Vapor GUI" ON)
option (BUILD_DOC "Build Vapor Doxygen documentation" ON)
option (INSTALL_DLL "Include third party library DLLs when building installer" OFF)

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
list (APPEND CMAKE_PROGRAM_PATH ${QTDIR}/bin)
# message ("--------------------- ${QTDIR}/bin")

find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIRS})

if (WIN32)
	find_library (GLEW glew32)
	set (INSTALL_BIN_DIR .)
	set (INSTALL_LIB_DIR .)
else ()
	find_library (GLEW GLEW)
	set (INSTALL_BIN_DIR bin)
	set (INSTALL_LIB_DIR lib)
endif ()

add_subdirectory (apps)
add_subdirectory (lib)
add_subdirectory (include)
add_subdirectory (share/Doxygen)

###############################################################################
#                            CPack Installation                               #
###############################################################################

if (WIN32)
	set (ENV_CONFIG_FILE_NAME "NSIS.EnvConfig.ini")
	set (ENV_CONFIG_PATH "${CMAKE_CURRENT_BINARY_DIR}/buildutils/${ENV_CONFIG_FILE_NAME}")
	configure_file ("buildutils/${ENV_CONFIG_FILE_NAME}.in" "${ENV_CONFIG_PATH}")
	if (${INSTALL_DLL})
		set (DLL_DIR "C:/vaporwin64deps2015/vapordlls")
		file (GLOB WIN_INSTALL_DLLS ${DLL_DIR}/*.dll ${QTDIR}/bin/*.dll)
		install (
			FILES ${WIN_INSTALL_DLLS}
			DESTINATION ${INSTALL_BIN_DIR}
			COMPONENT Dependencies
			)
	endif (${INSTALL_DLL})
endif (WIN32)

set (CPACK_PACKAGE_NAME ${PROJECT_NAME})
set (CPACK_PACKAGE_VENDOR "NCAR")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "VAPOR - DESCRIPTION\nInstaller build 0.1")
set (CPACK_PACKAGE_VERSION ${VERSION_STRING})
set (CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${VERSION_RC})
set (CPACK_PACKAGE_INSTALL_DIRECTORY "VAPOR")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set (CPACK_PACKAGE_EXECUTABLES vaporgui;VaporGUI)

set (CPACK_NSIS_MODIFY_PATH ON)
# set (NSIS_ADDITIONAL_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/NSIS.template.in")
set (CPACK_NSIS_MUI_ICON "Images/vapor-win-icon.ico")
# set (CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "MessageBox MB_OK \"Pre Install Command\"")

include (CPack)
