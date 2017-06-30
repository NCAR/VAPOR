UI_FILES := animationTab vizTab regionTab  \
	boxframe startupTab appSettingsTab\
	vizFeaturesTab \
	ControlPointEditorBase \
	barbAppearanceSubtab newRendererDialog renderselector \
	helloLayoutSubtab helloAppearanceSubtab \
	TwoDAppearanceGUI \
	BarbAppearanceGUI BarbVariablesGUI BarbGeometryGUI \
	TwoDVariablesGUI TwoDGeometryGUI VariablesWidgetGUI HelloVariablesGUI \
	statsWindow errMsg plotWindow SeedMeGUI firstTimeUser TFWidgetGUI \
	GeometryWidgetGUI ColorbarWidgetGUI

ifeq ($(BUILD_FLOW),1)
UI_FILES += fieldAppearanceSubtab seedingSubtab 
endif
