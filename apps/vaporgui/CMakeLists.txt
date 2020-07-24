set (SRCS
	EventRouter.cpp
	RenderEventRouter.cpp
	RenderHolder.cpp
	ImageFrame.cpp
	GLWidget.cpp
	ErrorReporter.cpp
	Histo.cpp
	ColorbarWidget.cpp
	SettingsEventRouter.cpp
	VizSelectCombo.cpp
	AnnotationEventRouter.cpp
	AnimationEventRouter.cpp
	HelloEventRouter.cpp
	NavigationEventRouter.cpp
	TwoDDataEventRouter.cpp
	WireFrameEventRouter.cpp
    PythonVariables.cpp
	Statistics.cpp
	Plot.cpp
	helloSubtabs.cpp
	VariablesWidget.cpp
	GeometryWidget.cpp
	CopyRegionWidget.cpp
	RangeCombos.cpp
	BarbEventRouter.cpp
	BannerGUI.cpp
	AnimationParams.cpp
	GUIStateParams.cpp
	MouseModeParams.cpp
	SettingsParams.cpp
	main.cpp
	MainForm.cpp
    VizWin.cpp
	VizWinMgr.cpp
	TabManager.cpp
	TrackBall.cpp
	ImageEventRouter.cpp
	ContourEventRouter.cpp
    SliceEventRouter.cpp
	PlotParams.cpp
	StatisticsParams.cpp
	ContourSubtabs.cpp
    SliceSubtabs.cpp
	BarbSubtabs.cpp
	TransformTable.cpp
	VaporTable.cpp
	FidelityWidget.cpp
	QSliderEdit.cpp
	QRange.cpp
	QSinglePoint.cpp
	QIntValidatorWithFixup.cpp
	FileOperationChecker.cpp
	Combo.cpp
	windowsUtils.cpp
	Manip.cpp
    FlowEventRouter.cpp
    FlowSubtabs.cpp
	VolumeEventRouter.cpp
	VolumeIsoEventRouter.cpp
	VolumeIsoSubtabs.cpp
	VolumeSubtabs.cpp
	ParamsWidgets.cpp
	ParamsMenuItems.cpp
	ParamsMenuItems.h
	TFEditor.cpp
	TFEditor.h
	TFEditorIsoSurface.cpp
	TFEditorIsoSurface.h
	TFOpacityWidget.cpp
	TFOpacityWidget.h
	TFColorWidget.cpp
	TFColorWidget.h
	QRangeSlider.cpp
	QRangeSlider.h
	TFHistogramWidget.cpp
	TFHistogramWidget.h
	TFInfoWidget.cpp
	TFInfoWidget.h
	QColorWidget.cpp
	QColorWidget.h
	TFColorInfoWidget.cpp
	TFColorInfoWidget.h
	TFOpacityInfoWidget.cpp
	TFOpacityInfoWidget.h
	TFMapWidget.cpp
	TFMapWidget.h
	TFMapGroupWidget.cpp
	TFMapGroupWidget.h
	TFHistogramInfoWidget.cpp
	TFHistogramInfoWidget.h
	TFIsoValueWidget.cpp
	TFIsoValueWidget.h
	TFIsoValueInfoWidget.cpp
	TFIsoValueInfoWidget.h
	TFUtils.cpp
	TFUtils.h
	TFMappingRangeSelector.cpp
	TFMappingRangeSelector.h
	QRangeSliderTextCombo.cpp
	QRangeSliderTextCombo.h
	QPaintUtils.cpp
	QPaintUtils.h
	VSection.cpp
	VSection.h
	VSubGroup.cpp
	VSubGroup.h
    VFrame.h
	VFrame.cpp
	VLineItem.cpp
	VLineItem.h
    VContainer.cpp
    VContainer.h
    VComboBox.cpp
    VComboBox.h
    VIntSpinBox.cpp
    VIntSpinBox.h
    VCheckBox.cpp
    VCheckBox.h
    VLineEdit.cpp
    VLineEdit.h
    VSlider.cpp
    VSlider.h
    VSliderEdit.cpp
    VSliderEdit.h
    VPushButton.cpp
    VPushButton.h
    QPushButtonWithDoubleClick.h
    VFileSelector.cpp
    VFileSelector.h
    VGeometry2.cpp
    VGeometry2.h
	PWidget.cpp
	PWidget.h
	PWidgetHLI.h
	PGroup.cpp
	PGroup.h
	PSection.cpp
	PSection.h
	PCheckbox.cpp
	PCheckbox.h
	PCheckboxHLI.h
	PDoubleInput.cpp
	PDoubleInput.h
	PDoubleInputHLI.h
	PIntegerInput.cpp
	PIntegerInput.h
	PIntegerInputHLI.h
	PDisplay.cpp
	PDisplay.h
	PDisplayHLI.h
	PLineItem.cpp
	PLineItem.h
	PEnumDropdown.cpp
	PEnumDropdown.h
	PEnumDropdownHLI.h
	PStringDropdown.cpp
	PStringDropdown.h
	PStringDropdownHLI.h
	PColorSelector.cpp
	PColorSelector.h
	PFileSelector.cpp
	PFileSelector.h
	PFileSelectorHLI.h
	PDynamicMixin.cpp
	PDynamicMixin.h
	PSliderEdit.cpp
	PSliderEdit.h
	PSliderEditHLI.h
	PVariableSelector.cpp
	PVariableSelector.h
	PRegionSelector.cpp
	PRegionSelector.h
	ParamsWidgetDemo.cpp
	ParamsWidgetDemo.h
	PTFEditor.cpp
	PTFEditor.h
	ModelEventRouter.cpp
	ModelEventRouter.h
	ModelSubtabs.cpp
	ModelSubtabs.h
	TFEditorVolume.cpp
	TFEditorVolume.h
	VDoubleValidator.cpp
	VDoubleValidator.h
	hide_std_error_util.cpp
	hide_std_error_util.h

	# Need to include all files that request .ui files
	TwoDSubtabs.h
	WireFrameSubtabs.h
    FlowSubtabs.h
	Statistics.h
	Plot.h
    PythonVariables.h
    BarbSubtabs.h
    ColorbarWidget.h
    OrientationAngles.h
    TransformTable.h
	ImageSubtabs.h
	ContourSubtabs.h
    SliceSubtabs.h
    VaporTable.h 
	FidelityWidget.h
	QSliderEdit.h
	QRange.h
	QSinglePoint.h
	VolumeEventRouter.h
	VolumeIsoEventRouter.h
	VolumeSubtabs.h
	VolumeIsoSubtabs.h
	HelloEventRouter.h
    SliceEventRouter.h
	TwoDDataEventRouter.h

	MainForm.h
	VizWinMgr.h
	windowsUtils.h
	ErrorReporter.h
	Histo.h
	EventRouter.h
	GLWidget.h
	VizWin.h
	Core3_2_context.h
	TrackBall.h
    Manip.h 
    NavigationEventRouter.h
	AnimationEventRouter.h
	AnimationParams.h
	AnnotationEventRouter.h
	BannerGUI.h
	BarbEventRouter.h
	FlowEventRouter.h
	Combo.h
	ContourEventRouter.h
	CopyRegionWidget.h
	FileOperationChecker.h
	GUIStateParams.h
	GeometryWidget.h
	ImageEventRouter.h
	ImageEventRouter.h
	ImageFrame.h
	ImageFrame.h
	MouseModeParams.h
	PlotParams.h
    PythonVariablesParams.h
	QIntValidatorWithFixup.h
	RangeCombos.h
	RenderEventRouter.h
	RenderHolder.h
	SettingsEventRouter.h
	SettingsParams.h
	StatisticsParams.h
	TabManager.h
	VariablesWidget.h
	VizSelectCombo.h
	WireFrameEventRouter.h
	ParamsWidgets.h
	Flags.h
)

set (UIS 
	LeftPanel.ui
	animationTab.ui
	NavigationTab.ui
	boxframe.ui
	startupTab.ui
	AnnotationGUI.ui
	SettingsGUI.ui
	appSettingsTab.ui
	NewRendererDialog.ui
	errMsg.ui
	helloLayoutSubtab.ui
	helloAppearanceSubtab.ui
	BarbAppearanceGUI.ui
	BarbVariablesGUI.ui
	BarbGeometryGUI.ui
	BarbAnnotationGUI.ui
	ImageAppearanceGUI.ui
	ImageGeometryGUI.ui
	ImageVariablesGUI.ui
	TransformTableGUI.ui
	TwoDVariablesGUI.ui
	TwoDAppearanceGUI.ui
	TwoDGeometryGUI.ui
	TwoDAnnotationGUI.ui
	ContourVariablesGUI.ui
	ContourAppearanceGUI.ui
	ContourGeometryGUI.ui
	ContourAnnotationGUI.ui
	WireFrameVariablesGUI.ui
	WireFrameAppearanceGUI.ui
	WireFrameGeometryGUI.ui
	WireFrameAnnotationGUI.ui
    SliceVariablesGUI.ui
    SliceAppearanceGUI.ui
    SliceGeometryGUI.ui
    SliceAnnotationGUI.ui
	VariablesWidgetGUI.ui
	statsWindow.ui
	plotWindow.ui
	firstTimeUser.ui
    PythonVariablesGUI.ui
	GeometryWidgetGUI.ui
	ColorbarWidgetGUI.ui
	FidelityWidgetGUI.ui
	StretchFactorTableGUI.ui
	OrientationAnglesGUI.ui
	CopyRegionWidgetGUI.ui
	VolumeAnnotationGUI.ui
	VolumeAppearanceGUI.ui
	VolumeGeometryGUI.ui
	VolumeVariablesGUI.ui
	VolumeIsoAnnotationGUI.ui
	VolumeIsoAppearanceGUI.ui
	VolumeIsoGeometryGUI.ui
	VolumeIsoVariablesGUI.ui
	ModelGeometryGUI.ui
	ModelVariablesGUI.ui
	
	QSliderEdit.ui
	QRange.ui
	QSinglePoint.ui
)

source_group (UIs FILES ${UIS})

find_package(Qt5 REQUIRED COMPONENTS Core OpenGL Widgets Gui DBus )

set (CMAKE_AUTOUIC ON) # This needs to appear before adding sources to work properly
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (ASSIMP_LIB assimp)
set (PYTHON_LIB python${PYTHONVERSION}m)
set (TIFF_LIB tiff)

if (APPLE)
	set (SRCS ${SRCS} core_profile_attributes.mm)
	set (MACOSX_BUNDLE_GUI_IDENTIFIER Vapor3)
	set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}})
	set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_RC})
	set (MACOSX_BUNDLE_ICON_FILE VAPOR.icns)
	set (MACOSX_BUNDLE_ICON_FILE_PATH ${CMAKE_SOURCE_DIR}/share/images/VAPOR.icns)
	set_source_files_properties(${MACOSX_BUNDLE_ICON_FILE_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	add_executable (vapor MACOSX_BUNDLE ${MACOSX_BUNDLE_ICON_FILE_PATH} ${SRCS} ${UIS})
	set (INSTALL_BIN_DIR ".")
elseif (WIN32)
    set (ASSIMP_LIB assimp-vc140-mt)
    set (PYTHON_LIB python${PYTHONVERSION})
    set (TIFF_LIB libtiff)

	set (WINDOWS_ICON_PATH "${CMAKE_SOURCE_DIR}/share/images/vapor-win-icon.ico")
	configure_file (vapor.rc.in ${CMAKE_CURRENT_BINARY_DIR}/vaporgui.rc)
	add_executable (vapor WIN32 ${SRCS} ${UIS} ${CMAKE_CURRENT_BINARY_DIR}/vaporgui.rc)
else ()
	add_executable (vapor ${SRCS} ${UIS})
endif()

target_link_libraries (
        vapor common vdc wasp render params jpeg ${GLEW} 
        ${TIFF_LIB} geotiff ${ASSIMP_LIB} Qt5::Widgets Qt5::OpenGL Qt5::Core Qt5::Gui Qt5::DBus 
        ${PYTHON_LIB}
)

if (APPLE)
	find_library (CORESERVICES CoreServices)
	find_library (FOUNDATION Foundation)
	find_library (COCOA Cocoa)
	# target_link_libraries (vapor "-framework CoreServices")
	target_link_libraries (vapor ${CORESERVICES} ${FOUNDATION} ${COCOA})
elseif (UNIX AND NOT APPLE)
	target_link_libraries (vapor quadmath)
endif ()

install (
	TARGETS vapor
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT vaporgui
	)
