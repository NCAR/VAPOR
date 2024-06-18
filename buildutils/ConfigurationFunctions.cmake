function(configure_apple_deployment_target)
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

    # No need for PARENT_SCOPE here, since we are caching the variable and therefore making it global
    if (CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        message("Building on macOS M1 architecture (arm64)")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version" FORCE)
    else()
        message("Building on macOS x86 architecture")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "macOS build architecture" FORCE)
    endif()
endfunction()

function(tag_commit_hash)
    #if (CMAKE_BUILD_TYPE STREQUAL "Release")
        get_git_head_revision (GIT_REFSPEC VERSION_COMMIT)
        execute_process (
            COMMAND git rev-parse --short ${VERSION_COMMIT}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE VERSION_COMMIT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    #endif ()
    string (TIMESTAMP VERSION_DATE UTC)
    if (VERSION_RC)
        set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_RC})
    else ()
        set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
    endif ()
    set (VERSION_STRING_FULL ${VERSION_STRING}.${VERSION_COMMIT} PARENT_SCOPE)
endfunction()

function(set_compiler_flags)
    if (APPLE)
        add_definitions (-DDarwin)
    elseif (WIN32)
        add_definitions (-DWIN32 -DNOMINMAX)
        add_definitions (-DGLAD_API_CALL_EXPORT)
    endif()

    # compiler warning flags
    if (NOT WIN32)
      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -Wno-overloaded-virtual -Wno-parentheses" PARENT_SCOPE)
      SET (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Wno-sign-compare" PARENT_SCOPE)
    else ()
      # Enable multithread compiling on Visual Studio
      # This feature is glitchy so you may need to re-run
      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" PARENT_SCOPE)
      SET (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP" PARENT_SCOPE)
    endif()
endFunction()

function(find_bundled_python)
    # FindPython supports Python_ROOT_DIR however vapor's bundled python distribution
    # does not conform to its requirements so this manually configures the results
    message("Using bundled python ${PYTHONVERSION}...")
    set(Python_VERSION "${PYTHONVERSION}")
    set(Python_NumPy_INCLUDE_DIRS "${NUMPY_INCLUDE_DIR}")
    unset(Python_LIBRARIES) # This is required for find_library to work in certain cases
    find_library(
        Python_LIBRARIES
    NAMES python${PYTHONVERSION} python${PYTHONVERSION}m
    PATHS ${THIRD_PARTY_LIB_DIR} ${PYTHONPATH}
        NO_DEFAULT_PATH
    )
    if (WIN32)
        set(Python_SITELIB "${PYTHONPATH}/Lib/site-packages")
        set(Python_INCLUDE_DIRS "${THIRD_PARTY_DIR}/Python${PYTHONVERSION}/include")
    else()
        set(Python_SITELIB "${PYTHONPATH}/site-packages" PARENT_SCOPE)
        if (NOT DEFINED Python_INCLUDE_DIRS)
            set(Python_INCLUDE_DIRS "${THIRD_PARTY_INC_DIR}/python${PYTHONVERSION}")
            message("Python_INCLUDE_DIRS ${THIRD_PARTY_INC_DIR}/python${PYTHONVERSION}")
        endif()
    endif()

    set(Python_VERSION "${Python_VERSION}" PARENT_SCOPE)
    set(Python_LIBRARIES "${Python_LIBRARIES}" PARENT_SCOPE)
    set(Python_INCLUDE_DIRS "${Python_INCLUDE_DIRS}" PARENT_SCOPE)
    set(Python_SITELIB "${Python_SITELIB}" PARENT_SCOPE)
    set(Python_NumPy_VERSION "UNUSED IN BUNDLED PYTHON" PARENT_SCOPE)
    set(Python_NumPy_INCLUDE_DIRS "${Python_NumPy_INCLUDE_DIRS}" PARENT_SCOPE)
endfunction()

function(dump_found_python)
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

function(configure_apple_compilation)
    # if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if (DIST_INSTALLER AND NOT BUILD_PYTHON)
        set (CMAKE_INSTALL_PREFIX /Applications PARENT_SCOPE)
        set (INSTALL_BIN_DIR ./vapor.app/Contents/MacOS PARENT_SCOPE)
        set (INSTALL_SHARE_DIR ./vapor.app/Contents/share PARENT_SCOPE)
        #set (INSTALL_LIB_DIR ./vapor.app/Contents/lib)
        set (INSTALL_LIB_DIR ./vapor.app/Contents/Frameworks PARENT_SCOPE)
        set (INSTALL_INCLUDE_DIR ./vapor.app/Contents/include/vapor PARENT_SCOPE)
    else ()
        set (INSTALL_BIN_DIR bin PARENT_SCOPE)
        set (INSTALL_LIB_DIR lib PARENT_SCOPE)
        set (INSTALL_SHARE_DIR share PARENT_SCOPE)
        set (INSTALL_INCLUDE_DIR include/vapor PARENT_SCOPE)
    endif ()

    if (BUILD_PYTHON)
        set (CMAKE_INSTALL_RPATH "@loader_path" PARENT_SCOPE)
    else ()
        #set (CMAKE_INSTALL_RPATH "@executable_path/../lib")
        set (CMAKE_INSTALL_RPATH "@executable_path/../Frameworks" PARENT_SCOPE)
    endif ()

    if (DIST_INSTALLER AND USE_OMP)
        message (WARNING "The build mode is set to distributable installer with OpenMP enabled and will not run from source")
        set (INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib" PARENT_SCOPE)
        set (CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE PARENT_SCOPE)
        set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE PARENT_SCOPE)
        set (CMAKE_SKIP_BUILD_RPATH FALSE PARENT_SCOPE)
    endif()
endfunction()
