#ifndef FLOWEVENTROUTER_H
#define FLOWEVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include <vapor/FlowRenderer.h>
#include "GL/glew.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "FlowSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

//!
//! \class FlowEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the Flow tab in the GUI
//! \author Scott Pearse
//! \version 3.0
//! \date  April 2016

//!	The FlowEventRouter class manages the Flow gui.  There are three sub-tabs,
//! for variables, geometry, and appearance.

class FlowEventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    FlowEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    ~FlowEventRouter();

    void GetWebHelp(vector<pair<string, string>> &help) const;

    //
    static string GetClassType() { return (VAPoR::FlowRenderer::GetClassType()); }
    string        GetType() const { return GetClassType(); }

protected:
    void           _updateTab();
    virtual string _getDescription() const;

    virtual string _getSmallIconImagePath() const { return ("Flow_small.png"); }
    virtual string _getIconImagePath() const { return ("Flow.png"); }

private:
    FlowEventRouter() {}

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events will always
    //! affect the combo boxes and other widgets in the tab, and it would be confusing
    //! if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    //! VariablesWidget is used as Variables tab
    FlowVariablesSubtab * _variables;
    FlowGeometrySubtab *  _geometry;
    FlowAppearanceSubtab *_appearance;
    FlowSeedingSubtab *   _seeding;
    FlowAnnotationSubtab *_annotation;
};

#endif    // FLOWEVENTROUTER_H
