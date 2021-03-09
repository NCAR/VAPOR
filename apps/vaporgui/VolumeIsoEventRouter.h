#pragma once

#include "RenderEventRouterGUI.h"
#include "vapor/VolumeIsoRenderer.h"

//! \class VolumeIsoEventRouter
//! \ingroup Public_GUI
//! \brief Isosurface renderer GUI
//! \author Stas Jaroszynski

class VolumeIsoEventRouter : public RenderEventRouterGUI {
public:
    VolumeIsoEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::VolumeIsoRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return false; }
    bool          Supports3DVariables() const { return true; }

protected:
    string _getDescription() const;
    string _getSmallIconImagePath() const { return "IsoSurface_small.png"; }
    string _getIconImagePath() const { return "IsoSurface.png"; }
};
