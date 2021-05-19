#pragma once

#include "RenderEventRouterGUI.h"
#include "vapor/ParticleRenderer.h"

//! \class ParticleEventRouter
//! \ingroup Public_GUI
//! \brief Particle renderer GUI
//! \author Stas Jaroszynski

class ParticleEventRouter : public RenderEventRouterGUI {
public:
    ParticleEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static string GetClassType() { return VAPoR::ParticleRenderer::GetClassType(); }
    string        GetType() const { return GetClassType(); }
    bool          Supports2DVariables() const { return true; }
    bool          Supports3DVariables() const { return true; }

protected:
    virtual string _getDescription() const;
    virtual string _getSmallIconImagePath() const { return "Particles_small.png"; }
    virtual string _getIconImagePath() const { return "Particles.png"; }
};
