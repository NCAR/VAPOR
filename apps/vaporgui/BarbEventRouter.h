#ifndef BARBEVENTROUTER_H
#define BARBEVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/BarbRenderer.h"
#include "vapor/BarbParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "BarbSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class GLBarbImageWindow;

//!
//! \class BarbEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the Barb tab in the GUI
//! \author Scott Pearse
//! \version 3.0
//! \date  April 2016

//!	The BarbEventRouter class manages the Barb gui.  There are three sub-tabs,
//! for variables, geometry, and appearance.

class BarbEventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    BarbEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    virtual void hookUpTab() {}

    void GetWebHelp(vector<pair<string, string>> &help) const;

    // Get static string identifier for this router class
    //
    static string GetClassType() { return (VAPoR::BarbRenderer::GetClassType()); }

    string GetType() const { return GetClassType(); }

protected:
    virtual void _updateTab();
    virtual void _initializeTab();

    virtual string _getDescription() const;

    virtual string _getSmallIconImagePath() const { return ("Barbs_small.png"); }
    virtual string _getIconImagePath() const { return ("Barbs.png"); }

private:
    BarbEventRouter() {}

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events
    //! will always affect the combo boxes and other widgets in the tab, and
    //! it would be confusing if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    BarbVariablesSubtab * _variables;
    BarbGeometrySubtab *  _geometry;
    BarbAppearanceSubtab *_appearance;
    BarbAnnotationSubtab *_annotation;
};

#endif    // BARBEVENTROUTER_H
