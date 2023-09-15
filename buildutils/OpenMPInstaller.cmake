function (OpenMPInstall)
    set(options )
    set(oneValueArgs TARGETS DESTINATION COMPONENT)
    set(multiValueArgs )
    cmake_parse_arguments(VINSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (USE_OMP AND APPLE)
        get_target_property(target_type ${VINSTALL_TARGETS} TYPE)
        get_target_property(target_is_bundle ${VINSTALL_TARGETS} MACOSX_BUNDLE)
        if (target_type STREQUAL "EXECUTABLE")
            set (SRC_FILE ${VINSTALL_TARGETS})
            set (SRC_DIR  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        elseif (target_type STREQUAL "SHARED_LIBRARY")
            set (SRC_FILE lib${VINSTALL_TARGETS}.dylib)
            set (SRC_DIR  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
        else()
            message(FATAL_ERROR "Unsupported target type '${target_type}'")
        endif()

        if (${target_is_bundle})
            set (APP_BIN_DIR "${SRC_FILE}.app/Contents/MacOS")
            set (SRC_PATH "${SRC_DIR}/${APP_BIN_DIR}/${SRC_FILE}")
            set (FINAL_DESTINATION "${VINSTALL_DESTINATION}/${APP_BIN_DIR}")
            set (BUNDLE_MSG "(BUNDLE)")
        else()
            set (SRC_PATH "${SRC_DIR}/${SRC_FILE}")
            set (FINAL_DESTINATION "${VINSTALL_DESTINATION}")
            set (BUNDLE_MSG "")
        endif()

        message("Generate OpenMP install script for ${VINSTALL_TARGETS} (${target_type}) ${BUNDLE_MSG}")
        # message("\t SRC=\"${SRC_PATH}\"")

        set (MOD ${CMAKE_CURRENT_BINARY_DIR}/MOD_FOR_INST_${SRC_FILE})

        install (
            CODE
            "
            message(\"Relinking OpenMP: ${VINSTALL_TARGETS}\")
            configure_file(\"${SRC_PATH}\" \"${MOD}\" COPYONLY)
            execute_process(COMMAND sh \"${CMAKE_SOURCE_DIR}/buildutils/renameomp.sh\" \"${MOD}\")
            "
            COMPONENT ${VINSTALL_COMPONENT}
            )

        if (${target_is_bundle})
            # This has to be called first to install cf bundle metadata
            # since the install PROGRAMS just installs the binary, not the app
            install (
                TARGETS ${VINSTALL_TARGETS}
                DESTINATION ${VINSTALL_DESTINATION}
                COMPONENT ${VINSTALL_COMPONENT}
                )
        endif()

        # install (CODE "message(\"install (                           \")")
        # install (CODE "message(\"    PROGRAMS ${MOD}                 \")")
        # install (CODE "message(\"    DESTINATION ${FINAL_DESTINATION}\")")
        # install (CODE "message(\"    COMPONENT ${VINSTALL_COMPONENT} \")")
        # install (CODE "message(\"    RENAME ${SRC_FILE}              \")")
        # install (CODE "message(\")                                   \")")
        install (
            PROGRAMS ${MOD}
            DESTINATION ${FINAL_DESTINATION}
            COMPONENT ${VINSTALL_COMPONENT}
            RENAME ${SRC_FILE}
            )
    else()
        install (
            TARGETS ${VINSTALL_TARGETS}
            DESTINATION ${VINSTALL_DESTINATION}
            COMPONENT ${VINSTALL_COMPONENT}
            )
    endif()
endfunction()
