#pragma once

#include <vapor/ImageRenderer.h>
#include <RenderEventRouterGUI.h>

//! \class ImageEventRouter
//! \ingroup Public_GUI
//! \brief Image Renderer GUI
//! \author Stas Jaroszynski

class ImageEventRouter : public RenderEventRouterGUI {
public:
    ImageEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    static std::string GetClassType() { return VAPoR::ImageRenderer::GetClassType(); }
    std::string        GetType() const { return GetClassType(); }
    bool               Supports2DVariables() const { return true; }
    bool               Supports3DVariables() const { return true; }

protected:
    string _getDescription() const;
    string _getSmallIconImagePath() const { return "Image_small.png"; }
    string _getIconImagePath() const { return "Image.png"; }
};
