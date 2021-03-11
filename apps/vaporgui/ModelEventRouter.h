#pragma once

#include "RenderEventRouterGUI.h"
#include "vapor/ModelRenderer.h"

//! \class ModelEventRouter
//! \ingroup Public_GUI
//! \brief Model renderer GUI
//! \author Stas Jaroszynski

class ModelEventRouter : public RenderEventRouterGUI {
public:
    ModelEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::ModelRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return true; }
    bool          Supports3DVariables() const { return true; }

protected:
    string _getDescription() const;
    string _getSmallIconImagePath() const { return "Model_small.png"; }
    string _getIconImagePath() const { return "Model.png"; }
};
