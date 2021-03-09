#pragma once

#include "RenderEventRouterGUI.h"
#include "vapor/TwoDDataRenderer.h"

//! \class TwoDDataEventRouter
//! \ingroup Public_GUI
//! \brief TwoDData Renderer GUI
//! \author Stas Jaroszynski

class TwoDDataEventRouter : public RenderEventRouterGUI {
public:
    TwoDDataEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::TwoDDataRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return true; }
    bool          Supports3DVariables() const { return false; }

protected:
    string _getDescription() const;
    string _getSmallIconImagePath() const { return "TwoDData_small.png"; }
    string _getIconImagePath() const { return "TwoDData.png"; }
};
