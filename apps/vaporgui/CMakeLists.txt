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
	NavigationEventRouter.cpp
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
	TrackBall.cpp
	GLColorbarWidget.cpp
	ImageEventRouter.cpp
	ContourEventRouter.cpp
	PlotParams.cpp
	StatisticsParams.cpp
	MiscParams.cpp
	ContourSubtabs.cpp
	BarbSubtabs.cpp
	TransformTable.cpp
	VaporTable.cpp
	FidelityWidget.cpp

	# Need to include all files that request .ui files
	TwoDSubtabs.h
	Statistics.h
	SeedMe.h
	Plot.h
	BarbSubtabs.h
	ColorbarWidget.h
	OrientationAngles.h
	TransformTable.h
	ImageSubtabs.h
	ContourSubtabs.h
	VaporTable.h
	FidelityWidget.h
)

set (UIS 
	animationTab.ui
	NavigationTab.ui
	regionTab.ui
	boxframe.ui
	startupTab.ui
	appSettingsTab.ui
	vizFeaturesTab.ui
	ControlPointEditorBase.ui
	newRendererDialog.ui
	NewRendererDialog.ui
	renderselector.ui
	LeftPanel.ui
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
	StretchFactorTableGUI.ui
	OrientationAnglesGUI.ui
	GeometryWidgetGUI.ui
	ColorbarWidgetGUI.ui
	ImageAppearanceGUI.ui
	ImageGeometryGUI.ui
	ImageVariablesGUI.ui
	ContourAppearanceGUI.ui
	ContourGeometryGUI.ui
	ContourVariablesGUI.ui
	FidelityWidgetGUI.ui
)

find_package(Qt4 REQUIRED)
set (CMAKE_AUTOUIC ON) # This needs to appear before adding sources to work properly
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
	set (MACOSX_BUNDLE_ICON_FILE VAPOR.icns)
	set (MACOSX_BUNDLE_ICON_FILE_PATH ${CMAKE_SOURCE_DIR}/share/images/VAPOR.icns)
	set_source_files_properties(${MACOSX_BUNDLE_ICON_FILE_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	add_executable (vapor MACOSX_BUNDLE ${MACOSX_BUNDLE_ICON_FILE_PATH} ${SRCS} ${UIS})
	set (INSTALL_BIN_DIR ".")
elseif (WIN32)
	set (WINDOWS_ICON_PATH "${CMAKE_SOURCE_DIR}/Images/vapor-win-icon.ico")
	configure_file (vapor.rc.in ${CMAKE_CURRENT_BINARY_DIR}/vaporgui.rc)
	add_executable (vapor ${SRCS} ${UIS} ${CMAKE_CURRENT_BINARY_DIR}/vaporgui.rc)
else ()
	add_executable (vapor ${SRCS} ${UIS})
endif()

# target_link_libraries (vapor common vdc wasp render params m jpeg dl util GLEW tiff geotiff assimp Qt4::QtCore Qt4::QtGui Qt4::QtOpenGL)
target_link_libraries (vapor common vdc wasp render params jpeg ${GLEW} tiff geotiff assimp Qt4::QtCore Qt4::QtGui Qt4::QtOpenGL python${PYTHONVERSION})

if (UNIX AND NOT APPLE)
	target_link_libraries (vapor quadmath)
endif ()

if (APPLE)
	target_compile_options (vapor PRIVATE "-Wno-deprecated-declarations")
endif ()

add_definitions (-DPYTHONVERSION="${PYTHONVERSION}")
add_definitions (-DPYTHONDIR="${PYTHONDIR}")
# add_definitions (-DPARAMS_EXPORTS)

install (
	TARGETS vapor
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT vaporgui
	)
