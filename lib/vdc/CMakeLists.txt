add_library (vdc SHARED
	BlkMemMgr.cpp
	CurvilinearGrid.cpp
	DC.cpp
	DCCF.cpp
	DCWRF.cpp
	DataMgr.cpp
	GeoUtil.cpp
	KDTreeRG.cpp
	LayeredGrid.cpp
	NetCDFCFCollection.cpp
	NetCDFCollection.cpp
	NetCDFSimple.cpp
	Proj4API.cpp
	RegularGrid.cpp
	StructuredGrid.cpp
	UDUnitsClass.cpp
	VDC.cpp
	VDCNetCDF.cpp
	VDC_c.cpp
	kdtree.c
	vizutil.cpp
)

target_link_libraries (vdc PUBLIC common wasp udunits2 proj)

add_definitions (-DVDF_EXPORTS)
