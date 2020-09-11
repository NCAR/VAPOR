#ifndef CONTOUREVENTROUTER_H
#define CONTOUREVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/ContourRenderer.h"
#include "vapor/ContourParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "ContourSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class GLContourImageWindow;

//!
//! \class ContourEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the Contour tab in the GUI
//! \author Scott Pearse
//! \version 3.0
//! \date  April 2016

//!	The ContourEventRouter class manages the Contour gui.  There are three sub-tabs,
//! for variables, geometry, and appearance.

class ContourEventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    ContourEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    virtual void hookUpTab() {}

    virtual bool Supports2DVariables() const { return true; }
    virtual bool Supports3DVariables() const { return false; }

    void GetWebHelp(vector<pair<string, string>> &help) const;

    // Get static string identifier for this router class
    //
    static string GetClassType() { return (VAPoR::ContourRenderer::GetClassType()); }

    string GetType() const { return GetClassType(); }

protected:
    virtual void   _initializeTab();
    virtual void   _updateTab();
    virtual string _getDescription() const;

    virtual string _getSmallIconImagePath() const { return ("Contours_small.png"); }
    virtual string _getIconImagePath() const { return ("Contours.png"); }

private:
    ContourEventRouter() {}

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events
    //! will always affect the combo boxes and other widgets in the tab, and
    //! it would be confusing if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    ContourVariablesSubtab * _variables;
    ContourGeometrySubtab *  _geometry;
    ContourAppearanceSubtab *_appearance;
    ContourAnnotationSubtab *_annotation;
};

#endif    // CONTOUREVENTROUTER_H
