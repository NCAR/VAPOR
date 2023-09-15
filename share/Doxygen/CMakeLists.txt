
set (DOC_DEST_DIR ${PROJECT_BINARY_DIR}/doc)

if (BUILD_DOC)
	find_package (Doxygen)
	if (DOXYGEN_FOUND)

        if (BUILD_PYTHON)
            set (DOXYGEN_GENERATE_XML "YES")
            set (DOXYGEN_GENERATE_HTML "NO")
        else()
            set (DOXYGEN_GENERATE_XML "NO")
            set (DOXYGEN_GENERATE_HTML "YES")
        endif ()

		configure_file (Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
		add_custom_target(doc
			${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
			DEPENDS Doxyfile.in
			COMMENT "Generating API documentation with Doxygen"
		)

		add_custom_command(
		OUTPUT ${DOC_DEST_DIR}
		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		COMMENT "Generating API documentation with Doxygen" VERBATIM
		)

	# add_custom_command(
	# TARGET ${DOC_DEST_DIR}
	# COMMAND ${DOXYGEN_EXECUTABLE} 
	# ARGS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	# )

        if (BUILD_PYTHON)
	        install (
		        DIRECTORY ${DOC_DEST_DIR}
		        DESTINATION ${INSTALL_SHARE_DIR}
		        COMPONENT Share
	        )
        endif ()
		

	endif (DOXYGEN_FOUND)
endif (BUILD_DOC)
