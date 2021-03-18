#pragma once

#include "RenderEventRouterGUI.h"
#include "vapor/ContourRenderer.h"

class PDoubleSliderEdit;

//! \class ContourEventRouter
//! \ingroup Public_GUI
//! \brief Contour renderer GUI
//! \author Stas Jaroszynski

class ContourEventRouter : public RenderEventRouterGUI {
public:
    ContourEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    bool          Supports2DVariables() const { return true; }
    bool          Supports3DVariables() const { return false; }
    static string GetClassType() { return VAPoR::ContourRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }

protected:
    virtual void   _updateTab();
    virtual string _getDescription() const;
    string         _getSmallIconImagePath() const { return "Contours_small.png"; }
    string         _getIconImagePath() const { return "Contours.png"; }

private:
    PDoubleSliderEdit *_spacingSlider;
    PDoubleSliderEdit *_minValueSlider;
};
