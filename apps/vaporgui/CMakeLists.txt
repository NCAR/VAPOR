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
	# Histo.cpp
	ColorbarWidget.cpp
	StartupEventRouter.cpp
	AppSettingsEventRouter.cpp
	VizSelectCombo.cpp
	VizFeatureEventRouter.cpp
	ColorbarSettings.cpp
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
	ColorMappedVariable.cpp
	BannerGUI.cpp
	# AppSettingsParams.cpp
	# GUIStateParams.cpp
	# MouseModeParams.cpp
	# StartupParams.cpp
	main.cpp
	MainForm.cpp
	VizWin.cpp
	VizWinMgr.cpp
	TabManager.cpp
	MessageReporter.cpp
	# TrackBall.cpp

	# Need to include all files that request .ui files
	TwoDSubtabs.h
	Statistics.h
	SeedMe.h
	Plot.h
	BarbSubtabs.h
)

set (UIS 
	BarbAppearanceGUI.ui
	BarbGeometryGUI.ui
	BarbVariablesGUI.ui
	ColorMappedVariableGUI.ui
	ColorbarWidgetGUI.ui
	ControlPointEditorBase.ui
	GeometryWidgetGUI.ui
	HelloVariablesGUI.ui
	SeedMeGUI.ui
	TFWidgetGUI.ui
	TwoDAppearanceGUI.ui
	TwoDGeometryGUI.ui
	TwoDVariablesGUI.ui
	VariablesWidgetGUI.ui
	animationTab.ui
	appSettingsTab.ui
	barbAppearanceSubtab.ui
	boxframe.ui
	colorbarframe.ui
	errMsg.ui
	firstTimeUser.ui
	helloAppearanceSubtab.ui
	helloLayoutSubtab.ui
	newRendererDialog.ui
	plotWindow.ui
	regionTab.ui
	renderselector.ui
	startupTab.ui
	statsWindow.ui
	vizFeaturesTab.ui
	vizTab.ui
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
else()
	add_executable (vaporgui ${SRCS} ${UIS})
endif()

# target_link_libraries (vaporgui common vdc wasp render params m jpeg dl util GLEW tiff geotiff assimp Qt4::QtCore Qt4::QtGui Qt4::QtOpenGL)
target_link_libraries (vaporgui common vdc wasp render params jpeg ${GLEW} tiff geotiff assimp Qt4::QtCore Qt4::QtGui Qt4::QtOpenGL)

if (UNIX AND NOT APPLE)
	target_link_libraries (vaporgui quadmath)
endif ()

if (APPLE)
	target_compile_options (vaporgui PRIVATE "-Wno-deprecated-declarations")
endif ()

add_definitions (-DPYTHONVERSION="${PYTHONVERSION}")
add_definitions (-DPYTHONDIR="${PYTHONDIR}")
# add_definitions (-DPARAMS_EXPORTS)
