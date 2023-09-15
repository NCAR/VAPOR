add_subdirectory (common)
add_subdirectory (wasp)
export (TARGETS common wasp NAMESPACE VAPOR:: FILE vapor-exports.cmake)

if (BUILD_VDC OR BUILD_GUI)
	add_subdirectory (vdc)
	export (TARGETS vdc NAMESPACE VAPOR:: APPEND FILE vapor-exports.cmake)
endif()


if (BUILD_GUI OR BUILD_PYTHON)
    add_subdirectory(osgl)
	add_subdirectory (params)
    add_subdirectory( flow )
	add_subdirectory (render)
    if (NOT WIN32)
        add_subdirectory (vapi)
        export (TARGETS vapi NAMESPACE VAPOR:: APPEND FILE vapor-exports.cmake)
    endif()
    export (TARGETS render params flow osgl NAMESPACE VAPOR:: APPEND FILE vapor-exports.cmake)
endif()

