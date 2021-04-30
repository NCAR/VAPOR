set (SRC
	BlkMemMgr.cpp
	Grid.cpp
	ConstantGrid.cpp
	StructuredGrid.cpp
	RegularGrid.cpp
	StretchedGrid.cpp
	LayeredGrid.cpp
	CurvilinearGrid.cpp
	UnstructuredGrid.cpp
	UnstructuredGrid2D.cpp
	UnstructuredGrid3D.cpp
	UnstructuredGridLayered.cpp
	NetCDFSimple.cpp
	NetCDFCollection.cpp
	NetCDFCFCollection.cpp
	UDUnitsClass.cpp
	Proj4API.cpp
	DC.cpp
	DCWRF.cpp
	DCCF.cpp
	DCMPAS.cpp
	DCP.cpp
    DCMelanie.cpp
	VDC.cpp
	VDCNetCDF.cpp
	DerivedVar.cpp
    DerivedParticleDensity.cpp
	DerivedVarMgr.cpp
	DataMgr.cpp
	GridHelper.cpp
	DataMgrUtils.cpp
	GeoUtil.cpp
	vizutil.cpp
	KDTreeRG.cpp
	kdtree.c
	VDC_c.cpp
	DCUtils.cpp
)

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/BlkMemMgr.h
	${PROJECT_SOURCE_DIR}/include/vapor/Grid.h
	${PROJECT_SOURCE_DIR}/include/vapor/GridHelper.h
	${PROJECT_SOURCE_DIR}/include/vapor/ConstantGrid.h
	${PROJECT_SOURCE_DIR}/include/vapor/StructuredGrid.h
	${PROJECT_SOURCE_DIR}/include/vapor/RegularGrid.h
	${PROJECT_SOURCE_DIR}/include/vapor/StretchedGrid.h
	${PROJECT_SOURCE_DIR}/include/vapor/LayeredGrid.h
	${PROJECT_SOURCE_DIR}/include/vapor/CurvilinearGrid.h
	${PROJECT_SOURCE_DIR}/include/vapor/UnstructuredGrid.h
	${PROJECT_SOURCE_DIR}/include/vapor/UnstructuredGrid2D.h
	${PROJECT_SOURCE_DIR}/include/vapor/UnstructuredGrid3D.h
	${PROJECT_SOURCE_DIR}/include/vapor/UnstructuredGridLayered.h
	${PROJECT_SOURCE_DIR}/include/vapor/NetCDFSimple.h
	${PROJECT_SOURCE_DIR}/include/vapor/NetCDFCollection.h
	${PROJECT_SOURCE_DIR}/include/vapor/NetCDFCFCollection.h
	${PROJECT_SOURCE_DIR}/include/vapor/UDUnitsClass.h
	${PROJECT_SOURCE_DIR}/include/vapor/Proj4API.h
	${PROJECT_SOURCE_DIR}/include/vapor/DC.h
	${PROJECT_SOURCE_DIR}/include/vapor/DCWRF.h
	${PROJECT_SOURCE_DIR}/include/vapor/DCCF.h
	${PROJECT_SOURCE_DIR}/include/vapor/DCMPAS.h
	${PROJECT_SOURCE_DIR}/include/vapor/DCP.h
	${PROJECT_SOURCE_DIR}/include/vapor/DCMelanie.h
	${PROJECT_SOURCE_DIR}/include/vapor/VDC.h
	${PROJECT_SOURCE_DIR}/include/vapor/VDCNetCDF.h
	${PROJECT_SOURCE_DIR}/include/vapor/DataMgr.h
	${PROJECT_SOURCE_DIR}/include/vapor/DataMgrUtils.h
	${PROJECT_SOURCE_DIR}/include/vapor/GeoUtil.h
	${PROJECT_SOURCE_DIR}/include/vapor/vizutil.h
	${PROJECT_SOURCE_DIR}/include/vapor/KDTreeRG.h
	${PROJECT_SOURCE_DIR}/include/vapor/VDC_c.h
	${PROJECT_SOURCE_DIR}/include/vapor/DerivedVar.h
	${PROJECT_SOURCE_DIR}/include/vapor/DerivedParticleDensity.h
	${PROJECT_SOURCE_DIR}/include/vapor/DerivedVarMgr.h
	${PROJECT_SOURCE_DIR}/include/vapor/DCUtils.h
	${PROJECT_SOURCE_DIR}/include/vapor/QuadTreeRectangle.hpp
)

add_library (vdc SHARED ${SRC} ${HEADERS})

target_link_libraries (vdc PUBLIC common wasp ${UDUNITS2} ${PROJ})

add_definitions (-DVDF_EXPORTS)

install (
	TARGETS vdc
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)

install (
	FILES ${HEADERS}
	DESTINATION ${INSTALL_INCLUDE_DIR}
	COMPONENT Libraries
	)
