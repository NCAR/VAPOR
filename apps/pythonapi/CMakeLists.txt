message("Python Found: '${Python_VERSION}' (${Python_LIBRARIES})")

SET(PYTHON_API_DIR ${CMAKE_BINARY_DIR}/python)

# CMake does not have proper support for copying files and its scripting language does not make adding it easy.

function(COPY_HELPER FILE DESTINATION DEP_LIST)
    get_filename_component(BASENAME "${FILE}" NAME)
    if ("${BASENAME}" IN_LIST COPY_EXCLUDE)
        return()
    endif()
    if (IS_DIRECTORY "${FILE}")
        file(GLOB SUBFILES "${FILE}/*")
        foreach (SUBFILE ${SUBFILES})
            COPY_HELPER("${SUBFILE}" "${DESTINATION}/${BASENAME}" "${DEP_LIST}")
        endforeach()
    else()
        set(OUTFILE "${DESTINATION}/${BASENAME}")
        add_custom_command(
            OUTPUT "${OUTFILE}"
            COMMAND ${CMAKE_COMMAND} -E copy "${FILE}" "${OUTFILE}"
            MAIN_DEPENDENCY "${FILE}"
            )
        list(APPEND DEP_LIST "${OUTFILE}")
        # of course this does not work for custom targets
        # add_dependencies(target ${OUTFILE})
    endif()
    set(DEP_LIST "${DEP_LIST}" PARENT_SCOPE)
endfunction()

set_property(GLOBAL PROPERTY COPY_HELPER_TARGET_COUNTER_PROPERTY "0")

function(COPY FILE DESTINATION)
    include(CMakeParseArguments)
    cmake_parse_arguments(PARSE_ARGV 2
        "COPY"
        ""
        ""
        "EXCLUDE"
        )

    get_filename_component(FILE "${FILE}" REALPATH)

    list(APPEND DEP_LIST "")
    COPY_HELPER("${FILE}" "${DESTINATION}" "${DEP_LIST}")

    get_property(COPY_HELPER_TARGET_COUNTER GLOBAL PROPERTY COPY_HELPER_TARGET_COUNTER_PROPERTY)
    math(EXPR COPY_HELPER_TARGET_COUNTER "${COPY_HELPER_TARGET_COUNTER}+1")
    set_property(GLOBAL PROPERTY COPY_HELPER_TARGET_COUNTER_PROPERTY ${COPY_HELPER_TARGET_COUNTER})

    add_custom_target(
        copy_helper_${COPY_HELPER_TARGET_COUNTER}
        ALL
        DEPENDS ${DEP_LIST}
    )
endfunction()


copy(vapor ${PYTHON_API_DIR} EXCLUDE cmake.py __pycache__ widget.js jquery.js)
copy(setup.py ${PYTHON_API_DIR})
copy(examples ${PYTHON_API_DIR} EXCLUDE CMakeLists.txt)
add_subdirectory (examples)

add_custom_command(
    OUTPUT "${PYTHON_API_DIR}/vapor/widget.js"
    COMMAND esbuild --log-level=warning --format=esm "${CMAKE_CURRENT_SOURCE_DIR}/vapor/widget.js" --bundle --outfile="${PYTHON_API_DIR}/vapor/widget.js"
    MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/vapor/widget.js"
)
add_custom_target(
    widget_js
    ALL
    DEPENDS "${PYTHON_API_DIR}/vapor/widget.js"
)

# copy(bld.bat ${PYTHON_API_DIR})
# copy(build.sh ${PYTHON_API_DIR})
# configure_file (meta.yaml ${PYTHON_API_DIR}/meta.yaml)

# get_target_property(VAPI_PRIVATE_COMPILE_DEFS vapi COMPILE_DEFINITIONS)
get_target_property(VAPI_COMPILE_DEFS vapi INTERFACE_COMPILE_DEFINITIONS)
get_property(GLOBAL_COMPILE_DEFS DIRECTORY PROPERTY COMPILE_DEFINITIONS)
# These don't quite work
# get_target_property(VAPI_DEFS vapi COMPILE_DEFINITIONS)
# get_target_property(VAPI_INT_DEFS vapi INTERFACE_COMPILE_DEFINITIONS)
# get_property(DIR_DEFS DIRECTORY PROPERTY COMPILE_DEFINITIONS)

configure_file (cmake.py ${PYTHON_API_DIR}/cmake.py)
configure_file (vapor/cmake.py ${PYTHON_API_DIR}/vapor/cmake.py)


if (CONDA_BUILD)
    set(CONDA_SITE_PACKAGE_DIR "$ENV{SP_DIR}")
    install(
        DIRECTORY "${PYTHON_API_DIR}/vapor"
		DESTINATION "${CONDA_SITE_PACKAGE_DIR}"
		COMPONENT Dependencies
	)
endif()

# Fails to find either despite both being installed
# list(APPEND CMAKE_MODULE_PATH "/usr/local/Caskroom/miniconda/base/envs/test/lib/python3.9/site-packages/cppyy_backend/cmake")
# find_package (LibClang REQUIRED)
# find_package (Cppyy REQUIRED)

