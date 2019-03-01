add_subdirectory (common)
add_subdirectory (wasp)
export (TARGETS common wasp NAMESPACE VAPOR:: FILE vapor-exports.cmake)

if (BUILD_VDC OR BUILD_GUI)
	add_subdirectory (vdc)
	export (TARGETS vdc NAMESPACE VAPOR:: APPEND FILE vapor-exports.cmake)
endif()


if (BUILD_GUI)
	add_subdirectory (params)
    add_subdirectory( flow )
	add_subdirectory (render)
	export (TARGETS render params flow NAMESPACE VAPOR:: APPEND FILE vapor-exports.cmake)
endif()

