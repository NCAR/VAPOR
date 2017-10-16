cmake_minimum_required (VERSION 3.1)
project (VAPOR)

set (CMAKE_CXX_STANDARD 11)

# set(CMAKE_VERBOSE_MAKEFILE on)
set (CMAKE_BUILD_TYPE Debug)

set (VERSION_MAJOR 3)
set (VERSION_MINOR 0)
set (VERSION_MICRO 0)
set (VERSION_RC alpha0)
string (TIMESTAMP VERSION_DATE UTC)
set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_RC})

if (APPLE)
	add_definitions (-DDarwin)
elseif (WIN32)
	add_definitions (-DWIN32)
endif()


option (BUILD_VDC "Build VDC library and utilities" ON)
option (BUILD_GUI "Build Vapor GUI" ON)
option (BUILD_DOC "Build Vapor Doxygen documentation" OFF)
option (BUILD_TEST_APPS "Build test applications" OFF)
option (GENERATE_FULL_INSTALLER "Include third party library DLLs and other large resource files when building installer" ON)

include_directories ("${PROJECT_BINARY_DIR}/include")
include_directories ("${PROJECT_SOURCE_DIR}/include")

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# These need to be updatd to change for install build set (DSO_DIR ${PROJECT_BINARY_DIR}/bin)
set (BINDIR ${PROJECT_BINARY_DIR}/bin)
set (ABS_TOP ${PROJECT_SOURCE_DIR})

set (QTDIR )
set (PYTHONDIR )
set (PYTHONVERSION )
set (NUMPY_INCLUDE_DIR )
set (THIRD_PARTY_LIB_DIR )
set (THIRD_PARTY_INC_DIR )

include (buildutils/UtilityFunctions.cmake)
include (site.local OPTIONAL)

include_directories (${THIRD_PARTY_INC_DIR})
include_directories (${THIRD_PARTY_INC_DIR}/freetype2)
link_directories (${THIRD_PARTY_LIB_DIR})
list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_LIB_DIR})
list (APPEND CMAKE_PROGRAM_PATH ${QTDIR}/bin)
include_directories (${THIRD_PARTY_INC_DIR}/python${PYTHONVERSION})
include_directories (${NUMPY_INCLUDE_DIR})
link_directories (${PYTHONDIR}/lib-dynload)

find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIRS})

if (WIN32)
	find_library (GLEW glew32)
	set (INSTALL_BIN_DIR .)
	set (INSTALL_LIB_DIR .)
	set (INSTALL_SHARE_DIR share)
elseif (APPLE)
	find_library (GLEW GLEW)
	set (INSTALL_BIN_DIR ./vaporgui.app/Contents/utilities)
	set (INSTALL_SHARE_DIR ./vaporgui.app/Contents/share)
	set (INSTALL_LIB_DIR ./vaporgui.app/Contents/lib)
	set (CMAKE_INSTALL_RPATH "@executable_path/../lib")
	set (CMAKE_INSTALL_PREFIX /Applications)
else ()
	find_library (GLEW GLEW)
	set (INSTALL_BIN_DIR lib)
	set (INSTALL_LIB_DIR lib)
	set (INSTALL_LAUNCHER_DIR bin)
	set (INSTALL_SHARE_DIR share)
	set (CMAKE_INSTALL_PREFIX "$ENV{HOME}/install")
endif ()

add_subdirectory (apps)
add_subdirectory (lib)
add_subdirectory (include)
add_subdirectory (share)
add_subdirectory (test_apps)

###############################################################################
#                            CPack Installation                               #
###############################################################################

set (CPACK_PACKAGE_NAME ${PROJECT_NAME})
set (CPACK_PACKAGE_VENDOR "NCAR")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "VAPOR - DESCRIPTION")
set (CPACK_PACKAGE_VERSION ${VERSION_STRING})
set (CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${VERSION_RC})
set (CPACK_PACKAGE_INSTALL_DIRECTORY "VAPOR")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set (CPACK_PACKAGE_EXECUTABLES vaporgui;VaporGUI)
set (CPACK_BINARY_STGZ OFF)
set (CPACK_BINARY_TGZ OFF)
set (CPACK_BINARY_TZ OFF)

if (WIN32)
	set (PRE_INSTALL_LOCAL_PATH "buildutils/NSIS.preInstall.ini")
	set (PRE_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PRE_INSTALL_LOCAL_PATH}")
	configure_file ("${PRE_INSTALL_LOCAL_PATH}.in" "${PRE_INSTALL_PATH}" @ONLY)

	set (PRE_UNINSTALL_LOCAL_PATH "buildutils/NSIS.preUnInstall.ini")
	set (PRE_UNINSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PRE_UNINSTALL_LOCAL_PATH}")
	configure_file ("${PRE_UNINSTALL_LOCAL_PATH}.in" "${PRE_UNINSTALL_PATH}" @ONLY)

	if (${GENERATE_FULL_INSTALLER})
		set (DLL_DIR "C:/vaporwin64deps2015/vapordlls")
		file (GLOB WIN_INSTALL_DLLS ${DLL_DIR}/*.dll ${QTDIR}/bin/*.dll)
		install (
			FILES ${WIN_INSTALL_DLLS}
			DESTINATION ${INSTALL_BIN_DIR}
			COMPONENT Dependencies
			)
		install (
			DIRECTORY ${PYTHONDIR}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)
	endif (${GENERATE_FULL_INSTALLER})

	set (CPACK_NSIS_MODIFY_PATH ON)
	set (CPACK_NSIS_URL_INFO_ABOUT "https://www.vapor.ucar.edu")
	set (CPACK_NSIS_EXECUTABLES_DIRECTORY ".") # Manually fixed in template file
	set (CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/Images/vapor-win-icon.ico")
	set (CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
	
	set (CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS  "${PRE_INSTALL_PATH}")
	set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS  "${PRE_UNINSTALL_PATH}")
	string (REGEX REPLACE "/" "\\\\" CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS  "${CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS}")
	string (REGEX REPLACE "/" "\\\\" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS  "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}")
	
	set (CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
	include (InstallRequiredSystemLibraries)

	set (CPACK_MODULE_PATH "${CMAKE_SOURCE_DIR}/buildutils")
endif (WIN32)

if (APPLE)
	if (${GENERATE_FULL_INSTALLER})
		file (GLOB INSTALL_LIBS ${THIRD_PARTY_LIB_DIR}/*.dylib)
		file (GLOB INSTALL_FRAMEWORKS ${THIRD_PARTY_LIB_DIR}/*.framework ${THIRD_PARTY_LIB_DIR}/python2.7)
		message (${INSTALL_FRAMEWORKS})
		install (
			FILES ${INSTALL_LIBS}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)
		install (
			DIRECTORY ${INSTALL_FRAMEWORKS}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)
	endif (${GENERATE_FULL_INSTALLER})
	set (CPACK_BINARY_DRAGNDROP ON)
endif (APPLE)

if (UNIX AND NOT APPLE)
	set (CPACK_BINARY_STGZ ON)
	set (EXTRA_LIBS_SEARCH GLU expat quadmath glut)
	add_custom_target (
		linuxpreinstall
		COMMAND ${CMAKE_SOURCE_DIR}/buildutils/gen_linux_shared_libs.pl
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vaporgui ${EXTRA_LIBS_SEARCH} >
		${CMAKE_BINARY_DIR}/LinuxInstallLibs.txt
		COMMAND touch ${CMAKE_SOURCE_DIR}/CMakeLists.txt
		)
	if (${GENERATE_FULL_INSTALLER})
		if (EXISTS ${CMAKE_BINARY_DIR}/LinuxInstallLibs.txt)
			file (STRINGS ${CMAKE_BINARY_DIR}/LinuxInstallLibs.txt INSTALL_LIBS)
		endif ()
		install (
			FILES ${INSTALL_LIBS}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)
		install (
			DIRECTORY ${THIRD_PARTY_LIB_DIR}/python2.7
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)
	endif (${GENERATE_FULL_INSTALLER})
endif (UNIX AND NOT APPLE)

include (CPack)
