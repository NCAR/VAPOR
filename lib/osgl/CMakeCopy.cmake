cmake_policy(SET CMP0057 NEW)

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

set(COPY_HELPER_TARGET_COUNTER 0)

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

    math(EXPR COPY_HELPER_TARGET_COUNTER "${COPY_HELPER_TARGET_COUNTER}+1")
    set(COPY_HELPER_TARGET_COUNTER ${COPY_HELPER_TARGET_COUNTER} PARENT_SCOPE)

    add_custom_target(
        copy_helper_${COPY_HELPER_TARGET_COUNTER}
        ALL
        DEPENDS ${DEP_LIST}
    )
endfunction()

