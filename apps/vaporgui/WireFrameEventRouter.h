#pragma once

#include "RenderEventRouterGUI.h"
#include "vapor/WireFrameRenderer.h"

//! \class WireFrameEventRouter
//! \ingroup Public_GUI
//! \brief WireFrame Renderer GUI
//! \author Stas Jaroszynski

class WireFrameEventRouter : public RenderEventRouterGUI {
public:
    WireFrameEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::WireFrameRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return true; }
    bool          Supports3DVariables() const { return true; }

protected:
    string _getDescription() const;
    string _getSmallIconImagePath() const { return "WireFrame_small.png"; }
    string _getIconImagePath() const { return "WireFrame.png"; }
};
