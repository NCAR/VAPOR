set (SRC
	BarbRenderer.cpp
	ControlExecutive.cpp
	HelloRenderer.cpp
	Renderer.cpp
	ShaderMgr.cpp
	ShaderProgram.cpp
	TwoDDataRenderer.cpp
	TwoDRenderer.cpp
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
	ControlExecutive.cpp
	ContourRenderer.cpp
	Manip.cpp
	MyPython.cpp
)

set (HEAD
	../../include/vapor/BarbRenderer.h
	../../include/vapor/ControlExecutive.h
	../../include/vapor/HelloRenderer.h
	../../include/vapor/Renderer.h
	../../include/vapor/ShaderMgr.h
	../../include/vapor/ShaderProgram.h
	../../include/vapor/TwoDDataRenderer.h
	../../include/vapor/TwoDRenderer.h
	../../include/vapor/GeoTile.h
	../../include/vapor/GeoTileMercator.h
	../../include/vapor/GeoTileEquirectangular.h
	../../include/vapor/GeoImage.h
	../../include/vapor/GeoImageTMS.h
	../../include/vapor/GeoImageGeoTiff.h
	../../include/vapor/ImageRenderer.h
	../../include/vapor/Visualizer.h
	../../include/vapor/AnnotationsRenderer.h
	../../include/vapor/textRenderer.h
	../../include/vapor/ControlExecutive.h
	../../include/vapor/ContourRenderer.h
	../../include/vapor/MyPython.h
)

add_library (render SHARED ${SRC} ${HEAD})

target_link_libraries (render PUBLIC common vdc params ftgl freetype geotiff jpeg tiff python${PYTHONVERSION})

if (NOT WIN32)
	target_compile_options(render PRIVATE "-Wno-deprecated-declarations")
endif ()

add_definitions (-DRENDER_EXPORTS)

install (
	TARGETS render
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)
