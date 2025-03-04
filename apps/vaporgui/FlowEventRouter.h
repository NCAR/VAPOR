#pragma once

#include "RenderEventRouterGUI.h"
#include <vapor/FlowRenderer.h>

class PIntegerSliderEdit;
class PDoubleSliderEdit;

//! \class FlowEventRouter
//! \ingroup Public_GUI
//! \brief Flow renderer GUI
//! \author Stas Jaroszynski

class FlowEventRouter : public RenderEventRouterGUI {
    QWidget *           _seedingTab;
    PIntegerSliderEdit *_pathlineLengthSlider;
    PIntegerSliderEdit *_pathlineInjectionSlider;
    PDoubleSliderEdit * _xRakeCenterSlider;
    PDoubleSliderEdit * _yRakeCenterSlider;
    PDoubleSliderEdit * _zRakeCenterSlider;

public:
    static const std::string SeedingTabName;
    static const std::string IntegrationTabName;

    FlowEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::FlowRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return true; }
    bool          Supports3DVariables() const { return true; }
    void          Update();

protected:
    string _getDescription() const;
    string _getSmallIconImagePath() const { return "Flow_small.png"; }
    string _getIconImagePath() const { return "Flow.png"; }
};
