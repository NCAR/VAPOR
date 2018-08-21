#ifndef DVREVENTROUTER_H
#define DVREVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/DVRenderer.h"
#include "vapor/DVRParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "DVRSubtabs.h"

QT_USE_NAMESPACE

//! The DVREventRouter class manages the DVR gui.  There are three sub-tabs,
//! for variables, geometry, and appearance.

class DVREventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    DVREventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    ~DVREventRouter();

    void GetWebHelp(vector<pair<string, string>> &help) const;

    static string GetClassType() { return (VAPoR::DVRenderer::GetClassType()); }
    string        GetType() const { return GetClassType(); }

protected:
    void           _updateTab();
    virtual string _getDescription() const;
    virtual string _getSmallIconImagePath() const { return ("DVR_small.png"); }
    virtual string _getIconImagePath() const { return ("DVR.png"); }

private:
    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events will always
    //! affect the combo boxes and other widgets in the tab, and it would be confusing
    //! if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    //! VariablesWidget is used as Variables tab
    DVRVariablesSubtab * _variables;
    DVRGeometrySubtab *  _geometry;
    DVRAppearanceSubtab *_appearance;
};

#endif    // DVREVENTROUTER_H
