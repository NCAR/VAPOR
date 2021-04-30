set (SRCS
	EventRouter.cpp
	RenderEventRouter.cpp
	RenderHolder.cpp
	GLWidget.cpp
	ErrorReporter.cpp
	Histo.cpp
	VizSelectCombo.cpp
	AnnotationEventRouter.cpp
	AnimationEventRouter.cpp
	NavigationEventRouter.cpp
	TwoDDataEventRouter.cpp
	TwoDDataEventRouter.h
	WireFrameEventRouter.cpp
    PythonVariables.cpp
	Statistics.cpp
	Plot.cpp
	CopyRegionWidget.cpp
	RangeCombos.cpp
	BarbEventRouter.cpp
	BarbEventRouter.h
	BannerGUI.cpp
	AnimationParams.cpp
	GUIStateParams.cpp
	MouseModeParams.cpp
	SettingsParams.cpp
	main.cpp
	MainForm.cpp
	MainForm_isOpenGLContextActive.cpp
    VizWin.cpp
	VizWinMgr.cpp
	TabManager.cpp
	TrackBall.cpp
	ImageEventRouter.cpp
	ImageEventRouter.h
	ContourEventRouter.cpp
	ContourEventRouter.h
    SliceEventRouter.cpp
    SliceEventRouter.h
	PlotParams.cpp
	StatisticsParams.cpp
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
	VolumeEventRouter.cpp
	VolumeEventRouter.h
	VolumeIsoEventRouter.cpp
	VolumeIsoEventRouter.h
	ParamsMenuItems.cpp
	ParamsMenuItems.h
	TFEditor.cpp
	TFEditor.h
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
    VFrame.h
	VFrame.cpp
	VLineItem.cpp
	VLineItem.h
	VHBoxWidget.cpp
	VHBoxWidget.h
    VComboBox.cpp
    VComboBox.h
    VIntSpinBox.cpp
    VIntSpinBox.h
    VCheckBox.cpp
    VCheckBox.h
    VLineEdit_Deprecated.cpp
    VLineEdit_Deprecated.h
    VActions.h
    VActions.cpp
    VSlider.cpp
    VSlider.h
    VSliderEdit.cpp
    VSliderEdit.h
    VSliderEditInterface.cpp
    VSliderEditInterface.h
    VStringLineEdit.h
    VStringLineEdit.cpp
    VIntLineEdit.h
    VIntLineEdit.cpp
    VDoubleLineEdit.h
    VDoubleLineEdit.cpp
    VDoubleSliderEdit.cpp
    VDoubleSliderEdit.h
    VIntSliderEdit.cpp
    VIntSliderEdit.h
    VNumericLineEdit.cpp
    VNumericLineEdit.h
    VIntRangeMenu.cpp
    VIntRangeMenu.h
    VDoubleRangeMenu.cpp
    VDoubleRangeMenu.h
    VNumericFormatMenu.cpp
    VNumericFormatMenu.h
    VPushButton.cpp
    VPushButton.h
    QPushButtonWithDoubleClick.h
    VFileSelector.cpp
    VFileSelector.h
	V3DInput.cpp
	V3DInput.h
	VGroup.cpp
	VGroup.h
	Updateable.cpp
	Updateable.h
	POrientationSelector.cpp
	POrientationSelector.h
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
	PLabel.cpp
	PLabel.h
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
	PRegionSelector.cpp
	PRegionSelector.h
	ParamsWidgetDemo.cpp
	ParamsWidgetDemo.h
	PTFEditor.cpp
	PTFEditor.h
    PFidelitySection.cpp
    PFidelitySection.h
    PTMSLODInput.h
	ModelEventRouter.cpp
	ModelEventRouter.h
	VDoubleValidator.cpp
	VDoubleValidator.h
	hide_std_error_util.cpp
	hide_std_error_util.h
	VContainer.cpp
	VContainer.h
	UWidget.cpp
	UWidget.h
	AbstractWidgetGroup.cpp
	AbstractWidgetGroup.h
	PTransformWidget.cpp
	PTransformWidget.h
	PCopyRegionWidget.cpp
	PCopyRegionWidget.h
	PSliceSampleLocationSelector.cpp
	PSliceSampleLocationSelector.h
	PAnnotationColorbarWidget.cpp
	PAnnotationColorbarWidget.h
	PStringInput.cpp
	PStringInput.h
	PWidgets.h
	PWidgetsFwd.h
	PGeometrySubtab.cpp
	PGeometrySubtab.h
	PDimensionSelector.cpp
	PDimensionSelector.h
	PVariableSelector.cpp
	PVariableSelector.h
	PShowIf.cpp
	PShowIf.h
	PWidgetWrapper.cpp
	PWidgetWrapper.h
	PButton.cpp
	PButton.h
	PFlowRakeRegionSelector.cpp
	PFlowRakeRegionSelector.h
	PMultiVarSelector.cpp
	PMultiVarSelector.h
	AppSettingsMenu.cpp
	AppSettingsMenu.h
    PConstantColorWidget.cpp
    PConstantColorWidget.h
    PCornerSelector.cpp
    PCornerSelector.h
    RenderEventRouterGUI.cpp
    RenderEventRouterGUI.h
    BookmarkParams.cpp
    BookmarkParams.h
    CheckForUpdate.cpp
    CheckForUpdate.h
    ParticleEventRouter.cpp
    ParticleEventRouter.h

	# Need to include all files that request .ui files
	Statistics.h
	Plot.h
    PythonVariables.h
    TransformTable.h
    VaporTable.h 
	FidelityWidget.h
	QSliderEdit.h
	QRange.h
	QSinglePoint.h

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
	FlowEventRouter.h
	Combo.h
	CopyRegionWidget.h
	FileOperationChecker.h
	GUIStateParams.h
	MouseModeParams.h
	PlotParams.h
    PythonVariablesParams.h
	QIntValidatorWithFixup.h
	RangeCombos.h
	RenderEventRouter.h
	RenderHolder.h
	SettingsParams.h
	StatisticsParams.h
	TabManager.h
	VizSelectCombo.h
	WireFrameEventRouter.h
	Flags.h
)

set (UIS 
	LeftPanel.ui
	animationTab.ui
	NavigationTab.ui
	AnnotationGUI.ui
	NewRendererDialog.ui
	TransformTableGUI.ui
	statsWindow.ui
	plotWindow.ui
    PythonVariablesGUI.ui
	FidelityWidgetGUI.ui
	errMsg.ui
	
	QSliderEdit.ui
	QRange.ui
	QSinglePoint.ui
)

source_group (UIs FILES ${UIS})

find_package(Qt5 REQUIRED COMPONENTS Core OpenGL Widgets Gui DBus Network)

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
        ${TIFF_LIB} geotiff ${ASSIMP_LIB} Qt5::Widgets Qt5::OpenGL Qt5::Core Qt5::Gui Qt5::DBus Qt5::Network
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
