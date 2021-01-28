#pragma once

#include <vapor/PreferencesRenderer.h>
#include <RenderEventRouter.h>

//! \class PreferencesEventRouter
//! \ingroup Public_GUI
//! \brief Preferences Renderer GUI
//! \author Scott Pearse

class PreferencesEventRouter : public RenderEventRouterGUI {
public:
    PreferencesEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static std::string GetClassType() { return VAPoR::PreferencesRenderer::GetClassType(); }
    std::string        GetType() const { return GetClassType(); }
    bool               Supports2DVariables() const { return true; }
    bool               Supports3DVariables() const { return true; }

protected:
    string _getDescription() const;
    string _getSmallIconPreferencesPath() const { return "Preferences_small.png"; }
    string _getIconPreferencesPath() const { return "Preferences.png"; }
};
