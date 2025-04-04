# Note on building VDC only:
# These packages from Ubuntu repo are required to build VAPOR with only VDC enabled:
# (tested on Ubuntu 18.04)
# 
# libnetcdf-dev,  libudunits2-dev,  libproj-dev

cmake_minimum_required (VERSION 3.17)
project (VAPOR3)

include (buildutils/UtilityFunctions.cmake)
include (buildutils/GetGitRevisionDescription.cmake)
include (buildutils/OpenMPInstaller.cmake)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "DEBUG" "RELEASE" "RELWITHDEBINFO")
endif()

if(APPLE)
    if("${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
        # Check if the target architecture is arm64
        include(CheckCXXSourceRuns)
        set(CMAKE_REQUIRED_FLAGS "-arch arm64")
        set(CHECK_ARM64_SOURCE_CODE "int main() { return 0; }")
        check_cxx_source_runs("${CHECK_ARM64_SOURCE_CODE}" IS_ARM64)
        if(IS_ARM64)
            set(CMAKE_OSX_ARCHITECTURES arm64)
        else()
            set(CMAKE_OSX_ARCHITECTURES x86_64)
        endif()
    endif()

    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version" FORCE)
    if (CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        message("Building on macOS M1 architecture (arm64)")
    else()
        message("Building on macOS x86 architecture")
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "macOS build architecture" FORCE)
    endif()
endif()

set (VERSION_MAJOR 3)
set (VERSION_MINOR 9)
set (VERSION_MICRO 4)
set (VERSION_RC )
message("CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
	get_git_head_revision (GIT_REFSPEC VERSION_COMMIT)
    message("VERSION_COMMIT ${VERSION_COMMIT}")
	execute_process (
		COMMAND git rev-parse --short ${VERSION_COMMIT}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE VERSION_COMMIT 
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
    message("VERSION_COMMIT2 ${VERSION_COMMIT}")
endif ()
string (TIMESTAMP VERSION_DATE UTC)
if (VERSION_RC)
	set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_RC})
else ()
	set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
endif ()
set (VERSION_STRING_FULL ${VERSION_STRING}.${VERSION_COMMIT})

if (APPLE)
	add_definitions (-DDarwin)
elseif (WIN32)
    add_definitions (-DWIN32 -DNOMINMAX)
    add_definitions (-DGLAD_API_CALL_EXPORT)
    add_compile_definitions(_HAS_STD_BYTE=0)
endif()

# compiler warning flags
if (NOT WIN32)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -Wno-overloaded-virtual -Wno-parentheses")
  SET (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Wno-sign-compare")
else ()
  # Enable multithread compiling on Visual Studio
  # This feature is glitchy so you may need to re-run
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  SET (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP")
endif()

if (CMAKE_GENERATOR STREQUAL "Xcode")
	set (DEFAULT_BUILD_UTILITIES OFF)
else ()
	set (DEFAULT_BUILD_UTILITIES ON)
endif ()

option (BUILD_VDC "Build VDC library and utilities" ON)
option (BUILD_GUI "Build Vapor GUI" ON)
option (BUILD_PYTHON "Build Vapor Python Library" OFF)
option (BUILD_OSP "Build OSPRay" ON)
option (BUILD_UTL "Build conversion and utility applications" ${DEFAULT_BUILD_UTILITIES})
option (BUILD_DOC "Build Vapor Doxygen documentation" ON)
option (BUILD_TEST_APPS "Build test applications" OFF)
option (DIST_INSTALLER "Generate installer for distributing vapor binaries. Will generate standard make install if off" OFF)
option (USE_OMP "Use OpenMP on some calculations" OFF)
option (CONDA_BUILD "Use Conda to build" OFF)
if (UNIX AND NOT APPLE)
    include (CMakeDependentOption)
    cmake_dependent_option (DIST_APPIMAGE "Generate an AppImage for VAPOR's installation across multiple Linux platforms" OFF "DIST_INSTALLER" ON)
endif (UNIX AND NOT APPLE)

if( USE_OMP )
    find_package(OpenMP REQUIRED)
    if( OpenMP_CXX_FOUND AND OpenMP_CXX_FLAGS )
        message(STATUS "OpenMP found! (${OpenMP_CXX_LIB_NAMES})")
    else()
        message(STATUS "OpenMP NOT found! Are you using Apple Clang?")
    endif()
endif()

set (GENERATE_FULL_INSTALLER ON)
if (BUILD_GUI)
	set (BUILD_VDC ON)
endif ()

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (test_output_dir ${CMAKE_BINARY_DIR}/test_binaries)
set (debug_output_dir ${CMAKE_BINARY_DIR}/debug_binaries)

set (QTDIR )
set (OSPRAYDIR )
set (PYTHONDIR )
set (PYTHONVERSION )
set (PYTHONPATH )
set (NUMPY_INCLUDE_DIR )
set (THIRD_PARTY_DIR )
set (THIRD_PARTY_LIB_DIR )
set (THIRD_PARTY_INC_DIR )
set (MAP_IMAGES_PATH )

include (site_files/site.NCAR OPTIONAL)
include (site.local           OPTIONAL)

if (CONDA_BUILD)
    unset (QTDIR)
    unset (OSPRAYDIR)
    unset (PYTHONDIR)
    unset (PYTHONVERSION)
    unset (PYTHONPATH)
    unset (NUMPY_INCLUDE_DIR)
    unset (THIRD_PARTY_DIR)
    unset (THIRD_PARTY_LIB_DIR)
    unset (THIRD_PARTY_INC_DIR)
    if (DEFINED ENV{MAP_IMAGES_PATH})
        set (MAP_IMAGES_PATH $ENV{MAP_IMAGES_PATH})
    endif()
else()
    message("THIRD_PARTY_DIR = ${THIRD_PARTY_DIR}")
    include_directories (SYSTEM ${THIRD_PARTY_INC_DIR})
    include_directories (SYSTEM ${THIRD_PARTY_INC_DIR}/freetype2)
    link_directories (${THIRD_PARTY_LIB_DIR})
    link_directories (${PYTHONPATH})

    list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_LIB_DIR})
    list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_DIR})
    list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_DIR}/HDF_Group/HDF5/1.12.2/lib)

    if (APPLE)
        set(CMAKE_MODULE_PATH /opt/local/lib/libomp)
    endif()
endif()

include_directories ("${PROJECT_SOURCE_DIR}/lib/osgl/include")
include_directories ("${PROJECT_SOURCE_DIR}/lib/osgl/glad/include")

if (WIN32)
	get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
endif ()
if (BUILD_VDC AND EXISTS ${PYTHONPATH}/lib-dynload)
	link_directories (${PYTHONPATH}/lib-dynload)
endif ()

find_library(NETCDF netcdf)
find_library(UDUNITS2 udunits2)
find_library(FREETYPE freetype)
find_library(GEOTIFF geotiff)
find_library(JPEG jpeg)
find_library(HDF5_LIB hdf5)
find_library(EXPAT expat)

message("Library NETCDF   = ${NETCDF}")
message("Library UDUNITS2 = ${UDUNITS2}")
message("Library FREETYPE = ${FREETYPE}")
message("Library GEOTIFF  = ${GEOTIFF}")
message("Library JPEG     = ${JPEG}")
message("Library HDF5_LIB = ${HDF5_LIB}")
message("Library EXPAT = ${EXPAT}")

# find_package(Python)
#   Output
#       ${Python_VERSION}
#       ${Python_LIBRARIES}
#       ${Python_INCLUDE_DIRS}
#       ${Python_SITELIB}
#       ${Python_NumPy_VERSION}
#       ${Python_NumPy_INCLUDE_DIRS}
#   https://cmake.org/cmake/help/v3.12/module/FindPython.html

function(FIND_BUNDLED_PYTHON)
    # FindPython supports Python_ROOT_DIR however vapor's bundled python distribution
    # does not conform to its requirements so this manually configures the results
    message("Using bundled python")
    message("    PYTHONDIR ${PYTHONDIR}")
    message("    PYTHONPATH ${PYTHONPATH}")
    set(Python_VERSION "${PYTHONVERSION}")
    set(Python_NumPy_INCLUDE_DIRS "${NUMPY_INCLUDE_DIR}")
    unset(Python_LIBRARIES) # This is required for find_library to work in certain cases
    if (APPLE)
        set(PYTHON_LIB_DIR "${PYTHONDIR}/lib")
    else()
        set(PYTHON_LIB_DIR "${PYTHONPATH}")
    endif()
    message("    PYTHON_LIB_DIR ${PYTHON_LIB_DIR}")
    find_library(
        Python_LIBRARIES
        NAMES python${PYTHONVERSION} python${PYTHONVERSION}m
        PATHS ${THIRD_PARTY_LIB_DIR} ${PYTHON_LIB_DIR}
        NO_DEFAULT_PATH
    )

    if (WIN32)
        set(Python_SITELIB "${PYTHONPATH}/Lib/site-packages")
    	set(Python_INCLUDE_DIRS "${THIRD_PARTY_DIR}/Python${PYTHONVERSION}/include")
    else()
        set(Python_SITELIB "${PYTHONPATH}/site-packages")
        if (NOT DEFINED Python_INCLUDE_DIRS)
            if (APPLE)
                set(Python_INCLUDE_DIRS "${PYTHONDIR}/include/python${PYTHONVERSION}")
            else()
                set(Python_INCLUDE_DIRS "${THIRD_PARTY_INC_DIR}/python${PYTHONVERSION}")
            endif()
        endif()
    endif()

    set(Python_VERSION "${Python_VERSION}" PARENT_SCOPE)
    set(Python_LIBRARIES "${Python_LIBRARIES}" PARENT_SCOPE)
    set(Python_INCLUDE_DIRS "${Python_INCLUDE_DIRS}" PARENT_SCOPE)
    set(Python_SITELIB "${Python_SITELIB}" PARENT_SCOPE)
    set(Python_NumPy_VERSION "UNUSED IN BUNDLED PYTHON" PARENT_SCOPE)
    set(Python_NumPy_INCLUDE_DIRS "${Python_NumPy_INCLUDE_DIRS}" PARENT_SCOPE)
endfunction()

function(DUMP_FOUND_PYTHON)
    set(PATHS "")
    list(APPEND PATHS
        Python_LIBRARIES
        Python_INCLUDE_DIRS
        Python_SITELIB
        Python_NumPy_INCLUDE_DIRS
    )
    message("Python Found ${ARGV0}")
    message("\tPython_VERSION = '${Python_VERSION}'")
    message("\tPython_NumPy_VERSION = '${Python_NumPy_VERSION}'")
    foreach(V ${PATHS})
        if (EXISTS "${${V}}")
            set(VE "OK")
        else()
            set(VE "**NOT FOUND**")
        endif()
        message("\t${V} = '${${V}}' ${VE}")
    endforeach()
endfunction()

# TODO
# - Replace PYTHONVERSION with Python_VERSION

if (BUILD_PYTHON)
    find_package(Python COMPONENTS Interpreter Development NumPy)
    # find_library (GETTEXT intl)
else()
    find_bundled_python()
endif()
dump_found_python()

include_directories ("${Python_INCLUDE_DIRS}")
include_directories ("${Python_NumPy_INCLUDE_DIRS}")


# if (BUILD_GUI OR BUILD_PYTHON)
# 	find_package (OpenGL REQUIRED)
# 	include_directories (${OPENGL_INCLUDE_DIRS})
# endif ()

if (WIN32)
	find_library(ASSIMP assimp-vc140-mt)
    find_library(TIFF libtiff)
    find_library(PROJ proj_6_1)
else ()
	find_library(ASSIMP assimp)
    find_library(TIFF tiff)
    find_library(PROJ proj)
endif()

if (WIN32)
	set (INSTALL_BIN_DIR .)
	set (INSTALL_LIB_DIR .)
	set (INSTALL_SHARE_DIR share)
	set (INSTALL_INCLUDE_DIR include/vapor)
elseif (APPLE)

	# if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if (DIST_INSTALLER AND NOT BUILD_PYTHON)
		set (CMAKE_INSTALL_PREFIX /Applications)
		set (INSTALL_BIN_DIR ./vapor.app/Contents/MacOS)
		set (INSTALL_SHARE_DIR ./vapor.app/Contents/share)
		set (INSTALL_INCLUDE_DIR ./vapor.app/Contents/include/vapor)
		set (INSTALL_LIB_DIR ./vapor.app/Contents/Frameworks)
	else ()
		set (INSTALL_BIN_DIR bin)
		set (INSTALL_LIB_DIR lib)
		set (INSTALL_SHARE_DIR share)
		set (INSTALL_INCLUDE_DIR include/vapor)
	endif ()

    if (BUILD_PYTHON)
        set (CMAKE_INSTALL_RPATH "@loader_path")
    else()
        set (CMAKE_INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../Resources/lib")
    endif ()

    if (DIST_INSTALLER AND USE_OMP)
		message (WARNING "The build mode is set to distributable installer with OpenMP enabled and will not run from source")
		set (INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
		set (CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
		set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
		set (CMAKE_SKIP_BUILD_RPATH FALSE)
	endif()
else ()
	if (DIST_INSTALLER)
		set (INSTALL_BIN_DIR lib)
	else ()
		set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
		set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
		set (INSTALL_BIN_DIR bin)
	endif ()
	set (INSTALL_LIB_DIR lib)
	set (INSTALL_INCLUDE_DIR include/vapor)
	set (INSTALL_LAUNCHER_DIR bin)
	set (INSTALL_SHARE_DIR share)
endif ()

add_subdirectory (lib)
add_subdirectory (apps)
add_subdirectory (include)
add_subdirectory (share)
add_subdirectory (scripts)
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
set (CPACK_PACKAGE_EXECUTABLES vapor;vapor)
set (CPACK_BINARY_STGZ OFF)
set (CPACK_BINARY_TGZ OFF)
set (CPACK_BINARY_TZ OFF)
set (CPACK_BINARY_DRAGNDROP OFF)

if (WIN32)
	set (PRE_INSTALL_LOCAL_PATH "buildutils/NSIS.preInstall.ini")
	set (PRE_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PRE_INSTALL_LOCAL_PATH}")
	configure_file ("${PRE_INSTALL_LOCAL_PATH}.in" "${PRE_INSTALL_PATH}" @ONLY)

	set (PRE_UNINSTALL_LOCAL_PATH "buildutils/NSIS.preUnInstall.ini")
	set (PRE_UNINSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PRE_UNINSTALL_LOCAL_PATH}")
	configure_file ("${PRE_UNINSTALL_LOCAL_PATH}.in" "${PRE_UNINSTALL_PATH}" @ONLY)
	
	if (GENERATE_FULL_INSTALLER)
		set (LIB_DIR ${THIRD_PARTY_DIR}/lib)
		file (GLOB WIN_INSTALL_DLLS ${LIB_DIR}/*.dll ${QTDIR}/bin/*.dll ${OSPRAYDIR}/bin/*.dll)

		install (
			FILES ${WIN_INSTALL_DLLS}
			DESTINATION ${INSTALL_BIN_DIR}
			COMPONENT Dependencies
			)
		install (
			FILES ${PYTHONPATH}/python${PYTHONVERSION}.dll
			DESTINATION ${INSTALL_BIN_DIR}
			COMPONENT Dependencies
			)
		install (
			DIRECTORY ${PYTHONPATH}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)
		install(FILES
			${QTDIR}/plugins/platforms/qwindows.dll
			DESTINATION platforms
		)
		install (
			DIRECTORY ${THIRD_PARTY_DIR}/share/plugins
			DESTINATION ${INSTALL_LIB_DIR}/share
			COMPONENT Dependencies
		)
	endif (GENERATE_FULL_INSTALLER)

	set (CPACK_NSIS_MODIFY_PATH OFF)
	set (CPACK_NSIS_URL_INFO_ABOUT "https://www.vapor.ucar.edu")
	set (CPACK_NSIS_EXECUTABLES_DIRECTORY ".") # Manually fixed in template file
	set (CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/share/images/vapor-win-icon.ico")
	set (CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
	
	set (CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS  "${PRE_INSTALL_PATH}")
	set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS  "${PRE_UNINSTALL_PATH}")
	#string (REGEX REPLACE "/" "\\\\" CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS  "${CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS}")
	#string (REGEX REPLACE "/" "\\\\" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS  "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}")
    file(TO_CMAKE_PATH "${PRE_INSTALL_PATH}" CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS)
    file(TO_CMAKE_PATH "${PRE_UNINSTALL_PATH}" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)

	
	set (CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
	include (InstallRequiredSystemLibraries)

	set (CPACK_MODULE_PATH "${CMAKE_SOURCE_DIR}/buildutils")
endif (WIN32)

if (APPLE)
	set (CPACK_BINARY_DRAGNDROP ON)
	if (DIST_INSTALLER AND GENERATE_FULL_INSTALLER)
		file (GLOB INSTALL_LIBS ${THIRD_PARTY_LIB_DIR}/*.dylib) 
		install (
			FILES ${INSTALL_LIBS}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)

        set (FRAMEWORKS Core OpenGL Widgets Gui DBus Network PrintSupport)
        foreach(item IN LISTS FRAMEWORKS)
            list(APPEND FRAMEWORKS "${THIRD_PARTY_LIB_DIR}/Qt${item}.framework")
            list(REMOVE_ITEM FRAMEWORKS ${item})
        endforeach(item IN LISTS ${FRAMEWORKS})
        install (
            DIRECTORY ${FRAMEWORKS}
            DESTINATION ${INSTALL_LIB_DIR}
            COMPONENT Dependencies
        )

        if (BUILD_OSP)
            file (GLOB INSTALL_OSP_LIBS ${OSPRAYDIR}/lib/*.dylib) 
            message (STATUS "OSPRAYDIR ${OSPRAYDIR}")
		    install (
		    	FILES ${INSTALL_OSP_LIBS}
		    	DESTINATION ${INSTALL_LIB_DIR}
		    	COMPONENT Dependencies
		    	)
        endif ()

        if (NOT BUILD_PYTHON)
                file (GLOB INSTALL_GUI_FRAMEWORKS ${PYTHONPATH})
                set (PYTHON_DESTINATION "${INSTALL_LIB_DIR}/../Resources/lib")
		    install (
                DIRECTORY ${INSTALL_GUI_FRAMEWORKS}
                DESTINATION ${PYTHON_DESTINATION}
                COMPONENT Dependencies
                PATTERN "bin" EXCLUDE
		    	)
            install (
		    	FILES ${Python_LIBRARIES}
		    	DESTINATION ${PYTHON_DESTINATION}
		    	COMPONENT Dependencies
		    	)
            file (GLOB COCOA_LIBS ${THIRD_PARTY_DIR}/plugins/platforms/libqcocoa.dylib)
	        install (
                FILES ${COCOA_LIBS}
                DESTINATION ${INSTALL_BIN_DIR}/platforms
                COMPONENT Dependencies
                )
            file (GLOB STYLE_LIBS ${THIRD_PARTY_DIR}/plugins/styles/libqmacstyle.dylib)
	        install (
                FILES ${STYLE_LIBS}
                DESTINATION ${INSTALL_BIN_DIR}/styles
                COMPONENT Dependencies
                )
        endif ()

        if (USE_OMP)
            get_filename_component(OMP_PATH ${OpenMP_CXX_LIBRARIES} REALPATH)
            install (
                FILES ${OMP_PATH}
                DESTINATION ${INSTALL_LIB_DIR}
                COMPONENT Dependencies
                )
        endif ()
        file (GLOB_RECURSE HDF5_PLUGINS "${THIRD_PARTY_DIR}/share/plugins/*.so")
        message (STATUS "plugins ${HDF5_PLUGINS}")
		install (
			FILES ${HDF5_PLUGINS}
			DESTINATION "${INSTALL_SHARE_DIR}/plugins"
			COMPONENT Dependencies
			)
        file (GLOB HDF5_LIBS ${THIRD_PARTY_DIR}/HDF_Group/HDF5/1.12.2/lib/*.dylib) 
        install (
            FILES ${HDF5_LIBS}
            DESTINATION ${INSTALL_LIB_DIR}
            COMPONENT Dependencies
            )

    endif ()
endif (APPLE)

if (UNIX AND NOT APPLE)
	set (CPACK_BINARY_STGZ ON)
	if (BUILD_VDC)
		set (EXTRA_LIBS_SEARCH ${EXTRA_LIBS_SEARCH} GLU glut expat omp)
	endif ()
	if (BUILD_GUI)
		set (EXTRA_LIBS_SEARCH ${EXTRA_LIBS_SEARCH} quadmath)
	endif ()


	if (DIST_INSTALLER)
        set (PARSE_BINARY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vapor")
        if (BUILD_PYTHON)
            set (PARSE_BINARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libvapi.so")
        endif()
        message( STATUS "${CMAKE_SOURCE_DIR}/buildutils/gen_linux_shared_libs.pl
            ${PARSE_BINARY} ${EXTRA_LIBS_SEARCH} >
            ${CMAKE_BINARY_DIR}/LinuxInstallLibs.txt")
		add_custom_target (
			linuxpreinstall
			COMMAND ${CMAKE_SOURCE_DIR}/buildutils/gen_linux_shared_libs.pl
			${PARSE_BINARY} ${EXTRA_LIBS_SEARCH} >
			${CMAKE_BINARY_DIR}/LinuxInstallLibs.txt
			COMMAND touch ${CMAKE_SOURCE_DIR}/CMakeLists.txt
			)
	endif ()
	if (DIST_INSTALLER AND GENERATE_FULL_INSTALLER)
		if (EXISTS ${CMAKE_BINARY_DIR}/LinuxInstallLibs.txt)
			file (STRINGS ${CMAKE_BINARY_DIR}/LinuxInstallLibs.txt INSTALL_LIBS)
		endif ()

		install (
			FILES ${INSTALL_LIBS}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)

        if (NOT BUILD_PYTHON)
		    install (
		    	DIRECTORY ${THIRD_PARTY_LIB_DIR}/python${PYTHONVERSION}
		    	DESTINATION ${INSTALL_LIB_DIR}
		    	COMPONENT Dependencies
		    )

            # Include libxcb-xinerama for Ubuntu 20/22 by reading /etc/issue
            if (EXISTS /etc/issue)
                file(READ "/etc/issue" ETC_ISSUE)
                string(REGEX MATCH "Ubuntu" DIST ${ETC_ISSUE})
                if(DIST STREQUAL "Ubuntu")
                    file (GLOB XCB_FILES ${THIRD_PARTY_LIB_DIR}/libxcb-xinerama.*)
                    install (
                        FILES ${XCB_FILES}
                        DESTINATION ${INSTALL_LIB_DIR}
                        COMPONENT Dependencies
                    )
                endif(DIST STREQUAL "Ubuntu")
            endif (EXISTS /etc/issue)

            install (
                FILES ${THIRD_PARTY_DIR}/plugins/platforms/libqxcb.so
                DESTINATION plugins/platforms
                COMPONENT Dependencies
            )

            install (
                DIRECTORY ${THIRD_PARTY_DIR}/plugins/xcbglintegrations
                DESTINATION ${INSTALL_LIB_DIR}
                COMPONENT Dependencies
            )

            file (GLOB XCB_FILES ${THIRD_PARTY_LIB_DIR}/libxcb-xinput.*)
            install (
                FILES ${XCB_FILES}
                DESTINATION ${INSTALL_LIB_DIR}
                COMPONENT Dependencies
            )

            file (GLOB XCBQPA_FILES ${THIRD_PARTY_LIB_DIR}/libQt5XcbQpa.*)
            install (
                FILES ${XCBQPA_FILES}
                DESTINATION ${INSTALL_LIB_DIR}
                COMPONENT Dependencies
            )

            if (BUILD_OSP)
                file (GLOB INSTALL_OSPRAY_LIBS ${OSPRAYDIR}/lib/*.so*)
                install (
                    FILES ${INSTALL_OSPRAY_LIBS}
                    DESTINATION ${INSTALL_LIB_DIR}
                    COMPONENT Dependencies
                    )
            endif (BUILD_OSP)

            file (GLOB HDF5_PLUGINS "${THIRD_PARTY_DIR}/HDF_Group/HDF5/1.12.2/lib/plugin/*.so")
            install (
                FILES ${HDF5_PLUGINS}
                DESTINATION "${INSTALL_SHARE_DIR}/plugins"
                COMPONENT Dependencies
            )
        endif (NOT BUILD_PYTHON)
	endif (DIST_INSTALLER AND GENERATE_FULL_INSTALLER)
endif (UNIX AND NOT APPLE)

if (DIST_INSTALLER)
	if (UNIX AND NOT APPLE)
        if(DIST_APPIMAGE)
            set (
                APPIMAGE_COMMAND 
                bash 
                ${CMAKE_SOURCE_DIR}/buildutils/genAppImage.sh 
                ${VERSION_STRING}
                ${CMAKE_SOURCE_DIR}
            )
        else()
            set (APPIMAGE_COMMAND echo Skipping AppImage generation)
        endif()
		
        add_custom_target (
			installer
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
			COMMAND cpack .
			COMMAND ${APPIMAGE_COMMAND}
			DEPENDS linuxpreinstall
        )
	else ()
		add_custom_target (
			installer
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
			COMMAND cpack .
			)
	endif ()
endif ()

if (BUILD_PYTHON)
    # message("GENERATORS = '${CPACK_GENERATOR}'")
    set(CPACK_GENERATOR "External")
    # message("GENERATORS = '${CPACK_GENERATOR}'")
    # message("CPACK_TOPLEVEL_DIRECTORY = '${CPACK_TOPLEVEL_DIRECTORY}'")
    # message("CPACK_TEMPORARY_DIRECTORY = '${CPACK_TEMPORARY_DIRECTORY}'")
    # message("CPACK_PACKAGE_NAME = '${CPACK_PACKAGE_NAME}'")
    # message("CPACK_PACKAGE_FILE_NAME = '${CPACK_PACKAGE_FILE_NAME}'")
    # message("CPACK_PACKAGE_VERSION = '${CPACK_PACKAGE_VERSION}'")
    # These only work for some things, dont work for others, and they break other things still
    # set (CPACK_TOPLEVEL_DIRECTORY "${CMAKE_BINARY_DIR}/CPACK_TOPLEVEL_DIRECTORY")
    # set (CPACK_TEMPORARY_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}/CPACK_TEMPORARY_DIRECTORY")
    # message("CPACK_TOPLEVEL_DIRECTORY = '${CPACK_TOPLEVEL_DIRECTORY}'")
    # message("CPACK_TEMPORARY_DIRECTORY = '${CPACK_TEMPORARY_DIRECTORY}'")
    set (CPACK_BINARY_DRAGNDROP OFF)
    # set (CPACK_BINARY_EXTERNAL ON)
    set (CPACK_EXTERNAL_REQUESTED_VERSIONS "1.0")
    set (CPACK_EXTERNAL_ENABLE_STAGING TRUE)

    if (NOT CONDA_BUILD)
	    install (
		    DIRECTORY ${THIRD_PARTY_DIR}/include
	        DESTINATION .
		    COMPONENT Dependencies
		)
    endif()
endif ()

include (CPack)
