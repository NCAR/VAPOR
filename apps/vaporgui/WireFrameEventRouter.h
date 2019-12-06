#ifndef WIREFRAMEEVENTROUTER_H
#define WIREFRAMEEVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/WireFrameRenderer.h"
#include "vapor/WireFrameParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "WireFrameSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class GLWireFrameImageWindow;

//!
//! \class WireFrameEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the WireFrame tab in the GUI
//! \author John Clyne
//! \version 3.0
//! \date  June 2018

//!	The WireFrameEventRouter class manages the WireFrame gui.
//! There are three sub-tabs,
//! for variables, geometry, and appearance.

class WireFrameEventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    WireFrameEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    void GetWebHelp(vector<pair<string, string>> &help) const;

    //
    static string GetClassType() { return (VAPoR::WireFrameRenderer::GetClassType()); }
    string        GetType() const { return GetClassType(); }

    virtual DimFlags GetDimFlags() const { return _variables->_variablesWidget->GetDimFlags(); }

protected:
    void           _updateTab();
    virtual string _getDescription() const;

    virtual string _getSmallIconImagePath() const { return ("WireFrame_small.png"); }
    virtual string _getIconImagePath() const { return ("WireFrame.png"); }

private:
    WireFrameEventRouter() {}

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events will always
    //! affect the combo boxes and other widgets in the tab, and it would be confusing
    //! if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    //! VariablesWidget is used as Variables tab
    WireFrameVariablesSubtab * _variables;
    WireFrameGeometrySubtab *  _geometry;
    GLWireFrameImageWindow *   _glWireFrameImageWindow;
    WireFrameAppearanceSubtab *_appearance;
    WireFrameAnnotationSubtab *_annotation;
};

#endif    // WIREFRAMEEVENTROUTER_H
