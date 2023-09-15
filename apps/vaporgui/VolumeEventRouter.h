#pragma once

#include "vapor/VolumeRenderer.h"
#include "RenderEventRouterGUI.h"

//! \class VolumeEventRouter
//! \ingroup Public_GUI
//! \brief Volume renderer GUI
//! \author Stas Jaroszynski

class VolumeEventRouter : public RenderEventRouterGUI {
public:
    VolumeEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::VolumeRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return false; }
    bool          Supports3DVariables() const { return true; }

protected:
    string _getDescription() const;
    string _getSmallIconImagePath() const { return "DVR_small.png"; }
    string _getIconImagePath() const { return "DVR.png"; }
};
