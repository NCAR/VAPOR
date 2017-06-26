add_subdirectory (common)
add_subdirectory (wasp)

if (BUILD_VDC OR BUILD_GUI)
	add_subdirectory (vdc)
endif()

if (BUILD_GUI)
	add_subdirectory (params)
	add_subdirectory (render)
endif()
