#ifndef ISOSURFACEEVENTROUTER_H
#define ISOSURFACEEVENTROUTER_H

#include <qobject.h>
#include <vapor/MyBase.h>
#include "GL/glew.h"
#include "vapor/IsoSurfaceRenderer.h"
#include "vapor/IsoSurfaceParams.h"
#include "RenderEventRouter.h"
#include "VariablesWidget.h"
#include "IsoSurfaceSubtabs.h"

QT_USE_NAMESPACE

class IsoSurfaceEventRouter : public QTabWidget, public RenderEventRouter {
    Q_OBJECT

public:
    IsoSurfaceEventRouter(QWidget *parent, VAPoR::ControlExec *ce);
    ~IsoSurfaceEventRouter();

    void GetWebHelp(vector<pair<string, string>> &help) const;

    static string GetClassType() { return (VAPoR::IsoSurfaceRenderer::GetClassType()); }
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
    IsoSurfaceVariablesSubtab * _variables;
    IsoSurfaceGeometrySubtab *  _geometry;
    IsoSurfaceAppearanceSubtab *_appearance;
    IsoSurfaceAnnotationSubtab *_annotation;
};

#endif
