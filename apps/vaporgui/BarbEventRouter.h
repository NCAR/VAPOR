#pragma once

#include "RenderEventRouterGUI.h"
#include "vapor/BarbRenderer.h"

//! \class BarbEventRouter
//! \ingroup Public_GUI
//! \brief Barb renderer GUI
//! \author Stas Jaroszynski

class BarbEventRouter : public RenderEventRouterGUI {
public:
    BarbEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::BarbRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return true; }
    bool          Supports3DVariables() const { return true; }

protected:
    virtual string _getDescription() const;
    virtual string _getSmallIconImagePath() const { return "Barbs_small.png"; }
    virtual string _getIconImagePath() const { return "Barbs.png"; }
};
