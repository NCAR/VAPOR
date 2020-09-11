#pragma once

#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/ModelRenderer.h"
#include "vapor/ModelParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "ModelSubtabs.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
}

class GLModelImageWindow;

//!
//! \class ModelEventRouter
//! \ingroup Public_GUI
//! \brief An EventRouter subclass that handles the Model tab in the GUI
//! \author Scott Pearse
//! \version 3.0
//! \date  April 2016

//!	The ModelEventRouter class manages the Model gui.  There are three sub-tabs,
//! for variables, geometry, and appearance.

class ModelEventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    ModelEventRouter(QWidget *parent, VAPoR::ControlExec *ce);

    virtual void hookUpTab() {}

    void GetWebHelp(vector<pair<string, string>> &help) const;

    // Get static string identifier for this router class
    //
    static string GetClassType() { return (VAPoR::ModelRenderer::GetClassType()); }

    string GetType() const { return GetClassType(); }

    virtual bool Supports2DVariables() const { return true; }
    virtual bool Supports3DVariables() const { return true; }

protected:
    virtual void   _initializeTab();
    virtual void   _updateTab();
    virtual string _getDescription() const;

    virtual string _getSmallIconImagePath() const { return ("Model_small.png"); }
    virtual string _getIconImagePath() const { return ("Model.png"); }

private:
    ModelEventRouter() {}

    //! Override default wheel behavior on the tab.  This has the effect of
    //! ignoring wheel events over the tab.  This is because wheel events
    //! will always affect the combo boxes and other widgets in the tab, and
    //! it would be confusing if wheel events also scrolled the tab itself
    void wheelEvent(QWheelEvent *) {}

    ModelVariablesSubtab *_variables;
    ModelGeometrySubtab * _geometry;
};
