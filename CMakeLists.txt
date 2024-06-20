# Note on building VDC only:
# These packages from Ubuntu repo are required to build VAPOR with only VDC enabled:
# (tested on Ubuntu 18.04)
# 
# libnetcdf-dev,  libudunits2-dev,  libproj-dev

cmake_minimum_required (VERSION 3.17)
project (TESTAPP3)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "DEBUG" "RELEASE" "RELWITHDEBINFO")
endif()

if(APPLE)
    if(NOT DEFINED ${CMAKE_OSX_ARCHITECTURES})
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

    if (CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        message("Building on macOS M1 architecture (arm64)")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version" FORCE)
    else()
        message("Building on macOS x86 architecture")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
    endif()
endif()

set (VERSION_MAJOR 3)
set (VERSION_MINOR 9)
set (VERSION_MICRO 1)
set (VERSION_RC )
if (VERSION_RC)
	set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_RC})
else ()
	set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
endif ()
set (VERSION_STRING_FULL ${VERSION_STRING}.${VERSION_COMMIT})

	add_definitions (-DDarwin)

# compiler warning flags
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -Wno-overloaded-virtual -Wno-parentheses")
  SET (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Wno-sign-compare")

if (CMAKE_GENERATOR STREQUAL "Xcode")
	set (DEFAULT_BUILD_UTILITIES OFF)
else ()
	set (DEFAULT_BUILD_UTILITIES ON)
endif ()

option (BUILD_VDC "Build VDC library and utilities" ON)
#option (BUILD_GUI "Build Vapor GUI" ON)
option (BUILD_GUI "Build Vapor GUI" OFF)
option (BUILD_OSP "Build OSPRay" ON)
option (BUILD_UTL "Build conversion and utility applications" ${DEFAULT_BUILD_UTILITIES})
option (BUILD_DOC "Build Vapor Doxygen documentation" ON)
option (BUILD_TEST_APPS "Build test applications" OFF)
option (DIST_INSTALLER "Generate installer for distributing vapor binaries. Will generate standard make install if off" ON)
option (USE_OMP "Use OpenMP on some calculations" OFF)
option (CONDA_BUILD "Use Conda to build" OFF)

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

    message("THIRD_PARTY_DIR = ${THIRD_PARTY_DIR}")
    include_directories (SYSTEM ${THIRD_PARTY_INC_DIR})
    include_directories (SYSTEM ${THIRD_PARTY_INC_DIR}/freetype2)
    link_directories (${THIRD_PARTY_LIB_DIR})
    link_directories (${PYTHONPATH})

    list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_LIB_DIR})
    list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_DIR})
    #list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_DIR}/HDF_Group/HDF5/1.12.2/lib)
    list (APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_DIR}/h/lib)

    if (APPLE)
        set(CMAKE_MODULE_PATH /opt/local/lib/libomp)
    endif()

include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories ("${PROJECT_SOURCE_DIR}/lib/osgl/include")
include_directories ("${PROJECT_SOURCE_DIR}/lib/osgl/glad/include")

	#link_directories (${PYTHONPATH}/lib-dynload)

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

	find_library(ASSIMP assimp)
    find_library(TIFF tiff)
    find_library(PROJ proj)

# PYTHON
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

        message("Using bundled python")
        set(Python_VERSION "${PYTHONVERSION}")
        set(Python_NumPy_INCLUDE_DIRS "${NUMPY_INCLUDE_DIR}")
        unset(Python_LIBRARIES) # This is required for find_library to work in certain cases
        find_library(
            Python_LIBRARIES
            NAMES python${PYTHONVERSION} python${PYTHONVERSION}m
            PATHS ${THIRD_PARTY_LIB_DIR} ${PYTHONPATH}
            NO_DEFAULT_PATH
        )
        set(Python_SITELIB "${PYTHONPATH}/site-packages")
        if (NOT DEFINED Python_INCLUDE_DIRS)
            set(Python_INCLUDE_DIRS "${THIRD_PARTY_INC_DIR}/python${PYTHONVERSION}")
        endif()
        set(Python_VERSION "${Python_VERSION}" PARENT_SCOPE)
        set(Python_LIBRARIES "${Python_LIBRARIES}" PARENT_SCOPE)
        set(Python_INCLUDE_DIRS "${Python_INCLUDE_DIRS}" PARENT_SCOPE)
        set(Python_SITELIB "${Python_SITELIB}" PARENT_SCOPE)
        set(Python_NumPy_VERSION "UNUSED IN BUNDLED PYTHON" PARENT_SCOPE)
        set(Python_NumPy_INCLUDE_DIRS "${Python_NumPy_INCLUDE_DIRS}" PARENT_SCOPE)
        dump_found_python()
        include_directories ("${Python_INCLUDE_DIRS}")
        include_directories ("${Python_NumPy_INCLUDE_DIRS}")
# /PYTHON

    #if (DIST_INSTALLER AND NOT BUILD_PYTHON)
		set (CMAKE_INSTALL_PREFIX /Applications)
		set (INSTALL_BIN_DIR ./testApp.app/Contents/MacOS)
		set (INSTALL_SHARE_DIR ./testApp.app/Contents/share)
		set (INSTALL_LIB_DIR ./testApp.app/Contents/Frameworks)
		set (INSTALL_INCLUDE_DIR ./testApp.app/Contents/include/vapor)

	    set (CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")

add_subdirectory (lib)
add_subdirectory (apps)
add_subdirectory (include)
add_subdirectory (share)
#add_subdirectory (scripts)
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
set (CPACK_PACKAGE_EXECUTABLES testApp;testApp)
set (CPACK_BINARY_STGZ OFF)
set (CPACK_BINARY_TGZ OFF)
set (CPACK_BINARY_TZ OFF)
set (CPACK_BINARY_DRAGNDROP OFF)
if (APPLE)
	set (CPACK_BINARY_DRAGNDROP ON)
	if (DIST_INSTALLER AND GENERATE_FULL_INSTALLER)
		#file (GLOB INSTALL_LIBS ${THIRD_PARTY_LIB_DIR}/libhdf5*.dylib) 
		file (GLOB INSTALL_LIBS ${THIRD_PARTY_LIB_DIR}/*.dylib) 
		install (
			FILES ${INSTALL_LIBS}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)

        # Get the list of libraries linked to MyApp
        get_target_property(linked_libraries testApp LINK_LIBRARIES)

        # Filter the list to include only Qt frameworks
        #set(FRAMEWORKS)
        #foreach(lib IN LISTS linked_libraries)
        #    if("${lib}" MATCHES "Qt5::") # Adjust this pattern based on your Qt version
        #        # Resolve the imported target to get the actual framework path
        #        get_target_property(lib_path ${lib} IMPORTED_LOCATION)
        #        list(APPEND FRAMEWORKS ${lib_path})
        #    endif()
        #endforeach()


        set (FRAMEWORKS Core OpenGL Widgets Gui DBus Network PrintSupport)
        foreach(item IN LISTS FRAMEWORKS)
            list(APPEND FRAMEWORKS "${THIRD_PARTY_LIB_DIR}/Qt${item}.framework")
            list(REMOVE_ITEM FRAMEWORKS ${item})
        endforeach(item IN LISTS ${FRAMEWORKS})


		#file (GLOB INSTALL_FRAMEWORKS ${THIRD_PARTY_LIB_DIR}/*.framework)
        #file (GLOB INSTALL_GUI_FRAMEWORKS ${PYTHONPATH})
		install (
			DIRECTORY ${FRAMEWORKS}
			DESTINATION ${INSTALL_LIB_DIR}
			COMPONENT Dependencies
			)

        #install (
        #    DIRECTORY ${INSTALL_GUI_FRAMEWORKS}
        #    DESTINATION ${INSTALL_LIB_DIR}
        #    COMPONENT Dependencies
        #    )
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

        #file (GLOB_RECURSE HDF5_PLUGINS "${THIRD_PARTY_DIR}/share/plugins/*.so")
        #message (STATUS "plugins ${HDF5_PLUGINS}")
		#install (
		#	FILES ${HDF5_PLUGINS}
		#	#DESTINATION "${INSTALL_SHARE_DIR}/plugins"
		#	DESTINATION "./vapor.app/Contents/PlugIns"
		#	COMPONENT Dependencies
		#	)

        #file (GLOB HDF5_LIBS ${THIRD_PARTY_DIR}/HDF_Group/HDF5/1.12.2/lib/*.dylib) 
        file (GLOB HDF5_LIBS ${THIRD_PARTY_DIR}/h/lib/*.dylib) 
        foreach(item IN LISTS HDF5_LIBS)
            message("item ${item}")
            #list(APPEND FRAMEWORKS "${THIRD_PARTY_LIB_DIR}/Qt${item}.framework")
            #list(REMOVE_ITEM FRAMEWORKS ${item})
        endforeach()
        install (
            FILES ${HDF5_LIBS}
            DESTINATION ${INSTALL_LIB_DIR}
            COMPONENT Dependencies
            )

    endif ()


    ## Modify our codesigning script to reference local
    ## environment variable $CMAKE_RUNTIME_OUTPUT_DIRECTORY
    #set(CODESIGN_RAW_SCRIPT     "${CMAKE_SOURCE_DIR}/buildutils/macCodeSign.cmake")
    #set(CODESIGN_UPDATED_SCRIPT "${CMAKE_SOURCE_DIR}/buildutils/macCodeSign_updated.cmake")
    #configure_file(
    #    ${CODESIGN_RAW_SCRIPT}
    #    ${CODESIGN_UPDATED_SCRIPT}
    #    @ONLY
    #)

    # Configure our modified codesigning script to run during "make install"
    #set(CPACK_INSTALL_SCRIPTS ${CODESIGN_UPDATED_SCRIPT})

endif (APPLE)

if (DIST_INSTALLER)
		add_custom_target (
			installer
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
			COMMAND cpack .
			)
endif ()

include (CPack)
