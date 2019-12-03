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
#include <vapor/MapperFunction.h>
#include "vapor/ContourParams.h"
#include "VariablesWidget.h"
#include "ContourEventRouter.h"
#include "EventRouter.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<ContourEventRouter> registrar(ContourEventRouter::GetClassType());

ContourEventRouter::ContourEventRouter(QWidget *parent, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, ContourParams::GetClassType())
{
    _variables = new ContourVariablesSubtab(this);
    QScrollArea *qsvar = new QScrollArea(this);
    qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _variables->adjustSize();
    qsvar->setWidget(_variables);
    qsvar->setWidgetResizable(true);
    addTab(qsvar, "Variables");

    _appearance = new ContourAppearanceSubtab(this);
    QScrollArea *qsapp = new QScrollArea(this);
    qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsapp->setWidget(_appearance);
    qsapp->setWidgetResizable(true);
    addTab(qsapp, "Appearance");

    _geometry = new ContourGeometrySubtab(this);
    QScrollArea *qsgeo = new QScrollArea(this);
    qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsgeo->setWidget(_geometry);
    qsgeo->setWidgetResizable(true);
    addTab(qsgeo, "Geometry");

    _annotation = new ContourAnnotationSubtab(this);
    QScrollArea *qsAnnotation = new QScrollArea(this);
    qsAnnotation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsAnnotation->setWidget(_annotation);
    qsAnnotation->setWidgetResizable(true);
    addTab(qsAnnotation, "Annotation");
}

void ContourEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Contour Overview", "http://www.vapor.ucar.edu/docs/vapor-gui-help/ContourOverview"));
    help.push_back(make_pair("Renderer control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/renderer-instances"));
    help.push_back(make_pair("Data accuracy control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/refinement-and-lod-control"));
    help.push_back(make_pair("Contour geometry options", "http://www.vapor.ucar.edu/docs/vapor-gui-help/ContourGeometry"));
    help.push_back(make_pair("Contour Appearance settings", "<>"
                                                            "http://www.vapor.ucar.edu/docs/vapor-gui-help/ContourAppearance"));
}

void ContourEventRouter::_initializeTab()
{
    _updateTab();
    ContourParams *rParams = (ContourParams *)GetActiveParams();

    _appearance->Initialize(rParams);

    rParams->SetVariables3D(false);
}

void ContourEventRouter::_updateTab()
{
    // The variable tab updates itself:
    _variables->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _appearance->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _geometry->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());

    _annotation->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());
}

string ContourEventRouter::_getDescription() const
{
    return ("Displays "
            "a series of user defined contours along a two dimensional plane within the "
            "user's domain.\n\nContours may hae constant coloration, or may be colored "
            "according to a secondary variable.\n\nContours may be displaced by a height "
            "variable.\n\n ");
}
