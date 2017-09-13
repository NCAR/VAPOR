set (SRCS
	EventRouter.cpp
	RenderEventRouter.cpp
	RenderHolder.cpp
	ImageFrame.cpp
	ControlPointEditor.cpp
	BoxSliderFrame.cpp
	DomainWidget.cpp
	GLWidget.cpp
	OpacityWidget.cpp
	ErrorReporter.cpp
	Histo.cpp
	ColorbarWidget.cpp
	StartupEventRouter.cpp
	AppSettingsEventRouter.cpp
	VizSelectCombo.cpp
	VizFeatureEventRouter.cpp
	AnimationEventRouter.cpp
	MappingFrame.cpp
	HelloEventRouter.cpp
	regioneventrouter.cpp
	ViewpointEventRouter.cpp
	TwoDDataEventRouter.cpp
	RangeController.cpp
	Statistics.cpp
	Plot.cpp
	SeedMe.cpp
	helloSubtabs.cpp
	TFWidget.cpp
	VariablesWidget.cpp
	GeometryWidget.cpp
	RangeCombos.cpp
	BarbEventRouter.cpp
	BannerGUI.cpp
	AnimationParams.cpp
	AppSettingsParams.cpp
	GUIStateParams.cpp
	MouseModeParams.cpp
	StartupParams.cpp
	main.cpp
	MainForm.cpp
	VizWin.cpp
	VizWinMgr.cpp
	TabManager.cpp
	MessageReporter.cpp
	TrackBall.cpp
	GLColorbarWidget.cpp
	ImageEventRouter.cpp
	ContourEventRouter.cpp
	PlotParams.cpp
	StatisticsParams.cpp
	MiscParams.cpp
	ContourSubtabs.cpp

	# Need to include all files that request .ui files
	TwoDSubtabs.h
	Statistics.h
	SeedMe.h
	Plot.h
	BarbSubtabs.h
	ColorbarWidget.h
	ImageSubtabs.h
	ContourSubtabs.h
)

set (UIS 
	animationTab.ui
	vizTab.ui
	regionTab.ui
	boxframe.ui
	startupTab.ui
	appSettingsTab.ui
	vizFeaturesTab.ui
	ControlPointEditorBase.ui
	barbAppearanceSubtab.ui
	newRendererDialog.ui
	renderselector.ui
	helloLayoutSubtab.ui
	helloAppearanceSubtab.ui
	TwoDAppearanceGUI.ui
	BarbAppearanceGUI.ui
	BarbVariablesGUI.ui
	BarbGeometryGUI.ui
	TwoDVariablesGUI.ui
	TwoDGeometryGUI.ui
	VariablesWidgetGUI.ui
	HelloVariablesGUI.ui
	statsWindow.ui
	errMsg.ui
	plotWindow.ui
	SeedMeGUI.ui
	firstTimeUser.ui
	TFWidgetGUI.ui
	GeometryWidgetGUI.ui
	ColorbarWidgetGUI.ui
	ImageAppearanceGUI.ui
	ImageGeometryGUI.ui
	ImageVariablesGUI.ui
	ContourAppearanceGUI.ui
	ContourGeometryGUI.ui
	ContourVariablesGUI.ui
)

find_package(Qt4 REQUIRED)
set (CMAKE_AUTOUIC ON) # This needs to appear before adding sources to work properly
set (CMAKE_AUTOMOC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
	set (MACOSX_BUNDLE_ICON_FILE VAPOR.icns)
	set (MACOSX_BUNDLE_ICON_FILE_PATH ${CMAKE_SOURCE_DIR}/share/images/VAPOR.icns)
	set_source_files_properties(${MACOSX_BUNDLE_ICON_FILE_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	add_executable (vaporgui MACOSX_BUNDLE ${MACOSX_BUNDLE_ICON_FILE_PATH} ${SRCS} ${UIS})
	set (INSTALL_BIN_DIR ".")
else()
	add_executable (vaporgui ${SRCS} ${UIS})
endif()

# target_link_libraries (vaporgui common vdc wasp render params m jpeg dl util GLEW tiff geotiff assimp Qt4::QtCore Qt4::QtGui Qt4::QtOpenGL)
target_link_libraries (vaporgui common vdc wasp render params jpeg ${GLEW} tiff geotiff assimp Qt4::QtCore Qt4::QtGui Qt4::QtOpenGL python${PYTHONVERSION})

if (UNIX AND NOT APPLE)
	target_link_libraries (vaporgui quadmath)
endif ()

if (APPLE)
	target_compile_options (vaporgui PRIVATE "-Wno-deprecated-declarations")
endif ()

add_definitions (-DPYTHONVERSION="${PYTHONVERSION}")
add_definitions (-DPYTHONDIR="${PYTHONDIR}")
# add_definitions (-DPARAMS_EXPORTS)

install (
	TARGETS vaporgui
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT VaporGUI
	)
