#pragma once

#include <QObject>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/VolumeIsoRenderer.h"
#include "vapor/VolumeIsoParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "VolumeIsoSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class GLVolumeImageWindow;

//!
//! \class VolumeIsoEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the Volume tab in the GUI
//! \author John Clyne
//! \version 3.0
//! \date  June 2018

//!	The VolumeIsoEventRouter class manages the Volume gui.
//! There are three sub-tabs,
//! for variables, geometry, and appearance.

class VolumeIsoEventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    VolumeIsoEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    void GetWebHelp(vector<pair<string, string>> &help) const;

    //
    static string GetClassType() { return (VAPoR::VolumeIsoRenderer::GetClassType()); }
    string        GetType() const { return GetClassType(); }

protected:
    void           _updateTab();
    virtual string _getDescription() const;

    virtual string _getSmallIconImagePath() const { return ("IsoSurface_small.png"); }
    virtual string _getIconImagePath() const { return ("IsoSurface.png"); }

private:
    VolumeIsoEventRouter() {}

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events will always
    //! affect the combo boxes and other widgets in the tab, and it would be confusing
    //! if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    //! VariablesWidget is used as Variables tab
    VolumeIsoVariablesSubtab * _variables;
    VolumeIsoGeometrySubtab *  _geometry;
    GLVolumeImageWindow *      _glVolumeImageWindow;
    VolumeIsoAppearanceSubtab *_appearance;
    VolumeIsoAnnotationSubtab *_annotation;
};
