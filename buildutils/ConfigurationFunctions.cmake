function(configure_apple_deployment_target)
    if("${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
        # Check if the target architecture is arm64
        include(CheckCXXSourceRuns)
        set(CMAKE_REQUIRED_FLAGS "-arch arm64")
        set(CHECK_ARM64_SOURCE_CODE "int main() { return 0; }")
        check_cxx_source_runs("${CHECK_ARM64_SOURCE_CODE}" IS_ARM64)
        if(IS_ARM64)
            set(CMAKE_OSX_ARCHITECTURES arm64 PARENT_SCOPE)
        else()
            set(CMAKE_OSX_ARCHITECTURES x86_64 PARENT_SCOPE)
        endif()
    endif()

    if (CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        message("Building on macOS M1 architecture (arm64)")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version" FORCE PARENT_SCOPE)
    else()
        message("Building on macOS x86 architecture")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE PARENT_SCOPE)
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "macOS build architecture" FORCE PARENT_SCOPE)
    endif()
endfunction()

function(tag_commit_hash)
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
    set (VERSION_STRING_FULL ${VERSION_STRING}.${VERSION_COMMIT} PARENT_SCOPE)
endfunction()
