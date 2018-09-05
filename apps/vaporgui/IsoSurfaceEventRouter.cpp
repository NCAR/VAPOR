#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <vapor/glutil.h>
#include <qlineedit.h>
#include <qscrollarea.h>
#include <qcolordialog.h>
#include <QFileDialog>
#include <vector>
#include <string>
#include "vapor/IsoSurfaceParams.h"
#include "VariablesWidget.h"
#include "IsoSurfaceEventRouter.h"
#include "EventRouter.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<IsoSurfaceEventRouter> registrar(IsoSurfaceEventRouter::GetClassType());

IsoSurfaceEventRouter::IsoSurfaceEventRouter(QWidget *parent, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, IsoSurfaceParams::GetClassType())
{
    _variables = new IsoSurfaceVariablesSubtab(this);
    QScrollArea *qsvar = new QScrollArea(this);
    qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _variables->adjustSize();
    qsvar->setWidget(_variables);
    qsvar->setWidgetResizable(true);
    addTab(qsvar, "Variables");

    _appearance = new IsoSurfaceAppearanceSubtab(this);
    QScrollArea *qsapp = new QScrollArea(this);
    qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsapp->setWidget(_appearance);
    qsapp->setWidgetResizable(true);
    addTab(qsapp, "Appearance");

    _geometry = new IsoSurfaceGeometrySubtab(this);
    QScrollArea *qsgeo = new QScrollArea(this);
    qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsgeo->setWidget(_geometry);
    qsgeo->setWidgetResizable(true);
    addTab(qsgeo, "Geometry");

    _annotation = new IsoSurfaceAnnotationSubtab(this);
    QScrollArea *qsannotation = new QScrollArea(this);
    qsannotation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsannotation->setWidget(_annotation);
    qsannotation->setWidgetResizable(true);
    addTab(qsannotation, "Annotation");
}

IsoSurfaceEventRouter::~IsoSurfaceEventRouter()
{
    if (_variables) delete _variables;
    if (_geometry) delete _geometry;
    if (_appearance) delete _appearance;
    if (_annotation) delete _annotation;
}

void IsoSurfaceEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("IsoSurface Overview", "https://www.vapor.ucar.edu/docs/vapor-gui-general-guide/volume-rendering-dvr"));

    help.push_back(make_pair("Renderer control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/renderer-instances"));

    help.push_back(make_pair("Data accuracy control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/refinement-and-lod-control"));
}

void IsoSurfaceEventRouter::_updateTab()
{
    _variables->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _appearance->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _geometry->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());

    _annotation->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());
}

string IsoSurfaceEventRouter::_getDescription() const
{
    return ("Displays "
            "the user's 3D data variables within a volume described by the source data "
            "file, according to color and opacity settings defined by the user.\n\n"
            "These 3D variables may be offset by a height variable.\n\n");
}
