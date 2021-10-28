#pragma once

#include "RenderEventRouterGUI.h"
#include <vapor/SliceRenderer.h>

//!
//! \class SliceEventRouter
//! \ingroup Public_GUI
//! \brief Defines the Slice Renderer GUI
//! \author Stas Jaroszynski

class SliceEventRouter : public RenderEventRouterGUI {
public:
    SliceEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::SliceRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return false; }
    bool          Supports3DVariables() const { return true; }

protected:
    string _getDescription() const;
    string _getSmallIconImagePath() const { return "Slice_small.png"; }
    string _getIconImagePath() const { return "Slice.png"; }
};
