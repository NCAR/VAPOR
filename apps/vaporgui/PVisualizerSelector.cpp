#include "PVisualizerSelector.h"
#include "VComboBox.h"
#include "vapor/ParamsMgr.h"
#include "vapor/GUIStateParams.h"
#ifndef NDEBUG
    #include "vapor/STLUtils.h"
#endif

PVisualizerSelector::PVisualizerSelector()
: PWidget("", _dropdown = new VComboBox)
{
    connect(_dropdown, &VComboBox::ValueChanged, this, &PVisualizerSelector::dropdownTextChanged);
}

void PVisualizerSelector::updateGUI() const
{
    GUIStateParams *gsp = (GUIStateParams*)getParamsMgr()->GetParams(GUIStateParams::GetClassType());

    auto options = getParamsMgr()->GetVisualizerNames();
    options.push_back("New Visualizer");
    _dropdown->SetOptions(options);
    _dropdown->SetValue(gsp->GetActiveVizName());

#ifndef NDEBUG
    if (!STLUtils::Contains(options, gsp->GetActiveVizName())) {
        options.push_back("ERR: \"" + gsp->GetActiveVizName() + "\"");
        _dropdown->SetOptions(options);
        _dropdown->SetValue("ERR: \"" + gsp->GetActiveVizName() + "\"");
    }
#endif
}

void PVisualizerSelector::dropdownTextChanged(std::string text)
{
    GUIStateParams *gsp = (GUIStateParams*)getParamsMgr()->GetParams(GUIStateParams::GetClassType());

    if (text == "New Visualizer") {
        getParamsMgr()->BeginSaveStateGroup("New Visualizer");
        text = getParamsMgr()->CreateVisualizerParamsInstance();
        gsp->SetActiveVizName(text);
        getParamsMgr()->EndSaveStateGroup();
    } else {
        gsp->SetActiveVizName(text);
    }
}
