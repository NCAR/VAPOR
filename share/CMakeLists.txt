add_subdirectory (Doxygen)
if (${GENERATE_FULL_INSTALLER})
	set (SHARE_DIRS
		fonts
		images
		shaders
		udunits
		python
		palettes
		examples
	)
	install (
		DIRECTORY ${SHARE_DIRS}
		DESTINATION ${INSTALL_SHARE_DIR}
		COMPONENT Share
	)
endif (${GENERATE_FULL_INSTALLER})
