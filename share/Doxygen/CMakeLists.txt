
set (DOC_DEST_DIR ${PROJECT_BINARY_DIR}/doc)

if (BUILD_DOC)
	find_package (Doxygen)
	if (DOXYGEN_FOUND)
		configure_file (Doxyfile.in Doxyfile @ONLY)
		add_custom_target(doc
			${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
			DEPENDS Doxyfile.in
			COMMENT "Generating API documentation with Doxygen"
		)

		# add_custom_command(
		# OUTPUT ${PROJECT_BINARY_DIR}/doc
		# COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		# COMMENT "Generating API documentation with Doxygen" VERBATIM
		# )

		# add_custom_command(
		# TARGET ${DOC_DEST_DIR}
		# COMMAND ${DOXYGEN_EXECUTABLE} 
		# ARGS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		# )
		

	endif (DOXYGEN_FOUND)
endif (BUILD_DOC)
