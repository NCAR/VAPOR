add_executable (ncdf2wasp ncdf2wasp.cpp)

find_library(HDF5_LIB hdf5)

target_link_libraries (ncdf2wasp common wasp ${HDF5_LIB})

OpenMPInstall (
	TARGETS ncdf2wasp
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
