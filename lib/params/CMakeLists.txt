add_library (params SHARED
	XmlNode.cpp
	ParamsBase.cpp
	ColorMap.cpp
	OpacityMap.cpp
	MapperFunction.cpp
	TransferFunction.cpp
	Box.cpp
	ColorbarPbase.cpp
	RenderParams.cpp
	TwoDDataParams.cpp
	BarbParams.cpp
	Viewpoint.cpp
	Transform.cpp
	ViewpointParams.cpp
	regionparams.cpp
	ParamsMgr.cpp
	DataStatus.cpp
	VizFeatureParams.cpp
	HelloParams.cpp
	ImageParams.cpp
	TFInterpolator.cpp
	ContourParams.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	find_library (AGL AGL)
	target_link_libraries (params PUBLIC ${AGL})
endif()

target_link_libraries (params PUBLIC vdc expat)

add_definitions (-DPARAMS_EXPORTS)

install (
	TARGETS params
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)
