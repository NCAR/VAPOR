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
		doc
		fonts
		images
		shaders
		udunits
		python
		palettes
		examples
	)
    if (NOT INSTALLER_OMIT_MAPS)
	    set (SHARE_DIRS ${SHARE_DIRS} "${MAP_IMAGES_PATH}")
    endif()
	install (
		DIRECTORY ${SHARE_DIRS}
		DESTINATION ${INSTALL_SHARE_DIR}
		COMPONENT Share
	)
endif ()
