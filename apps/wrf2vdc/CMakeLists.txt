add_executable (wrf2vdc wrf2vdc.cpp)

find_library(HDF5_LIB hdf5)

target_link_libraries (wrf2vdc common vdc ${HDF5_LIB})

OpenMPInstall (
	TARGETS wrf2vdc
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
