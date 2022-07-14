add_executable (cf2vdc cf2vdc.cpp)

find_library(HDF5_LIB hdf5)

target_link_libraries (cf2vdc common vdc ${HDF5_LIB})

OpenMPInstall (
	TARGETS cf2vdc
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
