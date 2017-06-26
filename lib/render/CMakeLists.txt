add_library (render SHARED
	BarbRenderer.cpp
	ControlExecutive.cpp
	HelloRenderer.cpp
	Renderer.cpp
	ShaderMgr.cpp
	ShaderProgram.cpp
	TwoDDataRenderer.cpp
	TwoDRenderer.cpp
	Visualizer.cpp
	VizFeatureRenderer.cpp
	jfilewrite.cpp
)

target_link_libraries (render PUBLIC common vdc params ftgl freetype geotiff jpeg tiff)

target_compile_options(render PRIVATE "-Wno-deprecated-declarations")
