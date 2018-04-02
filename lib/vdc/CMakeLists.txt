add_library (vdc SHARED
	BlkMemMgr.cpp
	Grid.cpp
	StructuredGrid.cpp
	RegularGrid.cpp
	StretchedGrid.cpp
	LayeredGrid.cpp
	CurvilinearGrid.cpp
	UnstructuredGrid.cpp
	UnstructuredGrid2D.cpp
	NetCDFSimple.cpp
	NetCDFCollection.cpp
	NetCDFCFCollection.cpp
	UDUnitsClass.cpp
	Proj4API.cpp
	DC.cpp
	DCWRF.cpp
	DCCF.cpp
	DCMPAS.cpp
	VDC.cpp
	VDCNetCDF.cpp
	DataMgr.cpp
	DataMgrUtils.cpp
	GeoUtil.cpp
	vizutil.cpp
	KDTreeRG.cpp
	kdtree.c
	VDC_c.cpp
	DerivedVar.cpp
	DerivedVarMgr.cpp
	DCUtils.cpp
	glutil.cpp
)

target_link_libraries (vdc PUBLIC common wasp udunits2 proj ${GLEW} ${OPENGL_LIBRARIES})

if (UNIX AND NOT APPLE)
	target_link_libraries (common GLU)
endif ()

add_definitions (-DVDF_EXPORTS)

install (
	TARGETS vdc
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)
