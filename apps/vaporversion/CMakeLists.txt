add_executable (vaporversion vaporversion.cpp)
set_target_properties(vaporversion PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/debug_binaries")

target_link_libraries (vaporversion common)

OpenMPInstall (
	TARGETS vaporversion
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
