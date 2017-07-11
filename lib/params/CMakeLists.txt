add_library (params SHARED
	AnimationParams.cpp
	BarbParams.cpp
	Box.cpp
	ColorMap.cpp
	ColorbarPbase.cpp
	DataStatus.cpp
	HelloParams.cpp
	MapperFunction.cpp
	OpacityMap.cpp
	ParamsBase.cpp
	ParamsMgr.cpp
	RenderParams.cpp
	TFInterpolator.cpp
	TransferFunction.cpp
	TwoDDataParams.cpp
	Viewpoint.cpp
	ViewpointParams.cpp
	VizFeatureParams.cpp
	XmlNode.cpp
	regionparams.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	find_library (AGL AGL)
	target_link_libraries (params PUBLIC ${AGL})
endif()

find_package (expat REQUIRED)
target_link_libraries (params PUBLIC vdc ${EXPAT_LIBRARIES})
