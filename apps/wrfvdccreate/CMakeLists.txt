add_executable (wrfvdccreate wrfvdccreate.cpp)

find_library(HDF5_LIB hdf5)

target_link_libraries (wrfvdccreate common vdc ${HDF5_LIB})

OpenMPInstall (
	TARGETS wrfvdccreate
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
