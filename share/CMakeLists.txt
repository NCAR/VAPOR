add_subdirectory (Doxygen)

if (BUILD_VDC)
	file (GLOB SHADERS_SRC shaders/*.vert shaders/*.frag shaders/*.geom)
	add_custom_target (
		shaders ALL
		SOURCES ${SHADERS_SRC}
	)
endif ()

if (BUILD_VDC AND ${GENERATE_FULL_INSTALLER})
	set (SHARE_DIRS
		fonts
		images
		shaders
		udunits
		python
		palettes
		examples
		${THIRD_PARTY_DIR}/share/images
	)
	install (
		DIRECTORY ${SHARE_DIRS}
		DESTINATION ${INSTALL_SHARE_DIR}
		COMPONENT Share
	)
endif ()
