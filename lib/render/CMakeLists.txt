set (SRC
	BarbRenderer.cpp
	ControlExecutive.cpp
	VolumeRenderer.cpp
	VolumeIsoRenderer.cpp
	HelloRenderer.cpp
	Renderer.cpp
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
	RayCaster.cpp
	FlowRenderer.cpp
	ControlExecutive.cpp
	ContourRenderer.cpp
    SliceRenderer.cpp
	MyPython.cpp
	MatrixManager.cpp
	Shader.cpp
	LegacyGL.cpp
	ShaderManager.cpp
	GLManager.cpp
	FontManager.cpp
	Font.cpp
	TextLabel.cpp
	PyEngine.cpp
	CalcEngineMgr.cpp
	GeoTIFWriter.cpp
	ImageWriter.cpp
	JPGWriter.cpp
	PNGWriter.cpp
	TIFWriter.cpp
	Proj4StringParser.cpp
	glutil.cpp
	VolumeAlgorithm.cpp
	VolumeRegular.cpp
	# VolumeResampled.cpp
	# VolumeTest.cpp
	# VolumeTest2.cpp
	VolumeCellTraversal.cpp
	Texture.cpp
	Framebuffer.cpp
	ModelRenderer.cpp
)

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/BarbRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/ControlExecutive.h
	${PROJECT_SOURCE_DIR}/include/vapor/HelloRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/VolumeRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/VolumeIsoRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/Renderer.h
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
	${PROJECT_SOURCE_DIR}/include/vapor/AnnotationRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/ControlExecutive.h
	${PROJECT_SOURCE_DIR}/include/vapor/ContourRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/MyPython.h
	${PROJECT_SOURCE_DIR}/include/vapor/MatrixManager.h
	${PROJECT_SOURCE_DIR}/include/vapor/Shader.h
	${PROJECT_SOURCE_DIR}/include/vapor/LegacyGL.h
	${PROJECT_SOURCE_DIR}/include/vapor/ShaderManager.h
	${PROJECT_SOURCE_DIR}/include/vapor/GLManager.h
	${PROJECT_SOURCE_DIR}/include/vapor/FontManager.h
	${PROJECT_SOURCE_DIR}/include/vapor/Font.h
	${PROJECT_SOURCE_DIR}/include/vapor/IResourceManager.h
	${PROJECT_SOURCE_DIR}/include/vapor/TextLabel.h
	${PROJECT_SOURCE_DIR}/include/vapor/RayCaster.h
	${PROJECT_SOURCE_DIR}/include/vapor/FlowRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/PyEngine.h
	${PROJECT_SOURCE_DIR}/include/vapor/CalcEngineMgr.h
	${PROJECT_SOURCE_DIR}/include/vapor/GeoTIFWriter.h
	${PROJECT_SOURCE_DIR}/include/vapor/ImageWriter.h
	${PROJECT_SOURCE_DIR}/include/vapor/JPGWriter.h
	${PROJECT_SOURCE_DIR}/include/vapor/PNGWriter.h
	${PROJECT_SOURCE_DIR}/include/vapor/TIFWriter.h
	${PROJECT_SOURCE_DIR}/include/vapor/jpegapi.h
	${PROJECT_SOURCE_DIR}/include/vapor/Proj4StringParser.h
	${PROJECT_SOURCE_DIR}/include/vapor/SliceRenderer.h
	${PROJECT_SOURCE_DIR}/include/vapor/glutil.h
	${PROJECT_SOURCE_DIR}/include/vapor/VolumeAlgorithm.h
	${PROJECT_SOURCE_DIR}/include/vapor/VolumeRegular.h
	# ${PROJECT_SOURCE_DIR}/include/vapor/VolumeResampled.h
	# ${PROJECT_SOURCE_DIR}/include/vapor/VolumeTest.h
	# ${PROJECT_SOURCE_DIR}/include/vapor/VolumeTest2.h
	${PROJECT_SOURCE_DIR}/include/vapor/VolumeCellTraversal.h
	${PROJECT_SOURCE_DIR}/include/vapor/Texture.h
	${PROJECT_SOURCE_DIR}/include/vapor/Framebuffer.h
	${PROJECT_SOURCE_DIR}/include/vapor/ModelRenderer.h
)

add_library (render SHARED ${SRC} ${HEADERS})

target_link_libraries (render PUBLIC common vdc params flow ${FTGL} ${FREETYPE} ${GEOTIFF} ${JPEG} ${TIFF} python${PYTHONVERSION} ${GLEW} ${OPENGL_LIBRARIES} ${ASSIMP})

if (UNIX AND NOT APPLE)
	target_link_libraries (render PUBLIC GLU)
endif ()


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
