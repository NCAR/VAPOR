set (SRC
	BarbRenderer.cpp
	ControlExecutive.cpp
	HelloRenderer.cpp
	Renderer.cpp
	ShaderMgr.cpp
	ShaderProgram.cpp
	TwoDDataRenderer.cpp
	TwoDRenderer.cpp
	WireFrameRenderer.cpp
	GeoTile.cpp
	GeoTileMercator.cpp
	GeoTileEquirectangular.cpp
	GeoImage.cpp
	GeoImageTMS.cpp
	GeoImageGeoTiff.cpp
	ImageRenderer.cpp
	Visualizer.cpp
	AnnotationRenderer.cpp
	jfilewrite.cpp
	textRenderer.cpp
	RayCaster.cpp
	DVRenderer.cpp
	IsoSurfaceRenderer.cpp
	ControlExecutive.cpp
	ContourRenderer.cpp
    SliceRenderer.cpp
	MyPython.cpp
)

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/BarbRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/ControlExecutive.h
	${PROJECT_SOURCE_DIR}/include/vapor/HelloRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/Renderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/ShaderMgr.h
	${PROJECT_SOURCE_DIR}/include/vapor/ShaderProgram.h
	${PROJECT_SOURCE_DIR}/include/vapor/TwoDDataRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/TwoDRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/WireFrameRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/GeoTile.h
	${PROJECT_SOURCE_DIR}/include/vapor/GeoTileMercator.h
	${PROJECT_SOURCE_DIR}/include/vapor/GeoTileEquirectangular.h
	${PROJECT_SOURCE_DIR}/include/vapor/GeoImage.h
	${PROJECT_SOURCE_DIR}/include/vapor/GeoImageTMS.h
	${PROJECT_SOURCE_DIR}/include/vapor/GeoImageGeoTiff.h
	${PROJECT_SOURCE_DIR}/include/vapor/ImageRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/Visualizer.h
	${PROJECT_SOURCE_DIR}/include/vapor/AnnotationsRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/textRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/ControlExecutive.h
	${PROJECT_SOURCE_DIR}/include/vapor/ContourRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/MyPython.h
)

add_library (render SHARED ${SRC} ${HEADERS})

target_link_libraries (render PUBLIC common vdc params ftgl freetype geotiff jpeg tiff python${PYTHONVERSION})

add_definitions (-DRENDER_EXPORTS)

install (
	TARGETS render
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)

install (
	FILES ${HEADERS}
	DESTINATION ${INSTALL_INCLUDE_DIR}
	COMPONENT Libraries
	)
