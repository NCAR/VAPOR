set (SRC
	XmlNode.cpp
	ParamsBase.cpp
	ColorMap.cpp
	OpacityMap.cpp
	MapperFunction.cpp
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
	AnnotationParams.cpp
	HelloParams.cpp
	ImageParams.cpp
	TFInterpolator.cpp
	ContourParams.cpp
	RayCasterParams.cpp
	AxisAnnotation.cpp
	WireFrameParams.cpp
    SliceParams.cpp
	DatasetsParams.cpp
	FlowParams.cpp
	VolumeParams.cpp
	VolumeIsoParams.cpp
	ModelParams.cpp
	ParticleParams.cpp
)

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/XmlNode.h
	${PROJECT_SOURCE_DIR}/include/vapor/ParamsBase.h
	${PROJECT_SOURCE_DIR}/include/vapor/ColorMap.h
	${PROJECT_SOURCE_DIR}/include/vapor/OpacityMap.h
	${PROJECT_SOURCE_DIR}/include/vapor/MapperFunction.h
	${PROJECT_SOURCE_DIR}/include/vapor/Box.h
	${PROJECT_SOURCE_DIR}/include/vapor/ColorbarPbase.h
	${PROJECT_SOURCE_DIR}/include/vapor/RenderParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/TwoDDataParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/BarbParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/Viewpoint.h
	${PROJECT_SOURCE_DIR}/include/vapor/ViewpointParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/regionparams.h
	${PROJECT_SOURCE_DIR}/include/vapor/ParamsMgr.h
	${PROJECT_SOURCE_DIR}/include/vapor/DataStatus.h
	${PROJECT_SOURCE_DIR}/include/vapor/AnnotationParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/HelloParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/ImageParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/TFInterpolator.h
	${PROJECT_SOURCE_DIR}/include/vapor/ContourParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/AxisAnnotation.h
	${PROJECT_SOURCE_DIR}/include/vapor/WireFrameParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/DatasetsParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/SliceParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/RayCasterParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/FlowParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/VolumeParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/VolumeIsoParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/ModelParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/ParticleParams.h
	${PROJECT_SOURCE_DIR}/include/vapor/Transform.h
)

add_library (params SHARED ${SRC} ${HEADERS})

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

install (
	FILES ${HEADERS}
	DESTINATION ${INSTALL_INCLUDE_DIR}
	COMPONENT Libraries
	)
