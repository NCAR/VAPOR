add_executable (vapor_check_udunits vapor_check_udunits.cpp)
set_target_properties( vapor_check_udunits PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/debug_binaries")

target_link_libraries (vapor_check_udunits common vdc)

OpenMPInstall (
	TARGETS vapor_check_udunits
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
