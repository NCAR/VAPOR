add_executable (vaporpychecker vaporpychecker.cpp)
set_target_properties(vaporpychecker PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/debug_binaries")

target_link_libraries (vaporpychecker common render)

OpenMPInstall (
	TARGETS vaporpychecker
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
