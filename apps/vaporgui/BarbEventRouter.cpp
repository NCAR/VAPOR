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
#include "vapor/BarbParams.h"
#include "VariablesWidget.h"
#include "BarbEventRouter.h"
#include "EventRouter.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<BarbEventRouter> registrar(BarbEventRouter::GetClassType());

BarbEventRouter::BarbEventRouter(QWidget *parent, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, BarbParams::GetClassType())
{
    _variables = new BarbVariablesSubtab(this);
    QScrollArea *qsvar = new QScrollArea(this);
    qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _variables->adjustSize();
    qsvar->setWidget(_variables);
    qsvar->setWidgetResizable(true);
    addTab(qsvar, "Variables");

    _appearance = new BarbAppearanceSubtab(this);
    QScrollArea *qsapp = new QScrollArea(this);
    qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsapp->setWidget(_appearance);
    qsapp->setWidgetResizable(true);
    addTab(qsapp, "Appearance");

    _geometry = new BarbGeometrySubtab(this);
    QScrollArea *qsgeo = new QScrollArea(this);
    qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsgeo->setWidget(_geometry);
    qsgeo->setWidgetResizable(true);
    addTab(qsgeo, "Geometry");

    _annotation = new BarbAnnotationSubtab(this);
    QScrollArea *qsAnnotation = new QScrollArea(this);
    qsAnnotation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsAnnotation->setWidget(_annotation);
    qsAnnotation->setWidgetResizable(true);
    addTab(qsAnnotation, "Annotation");
}

void BarbEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Barb Overview", "http://www.vapor.ucar.edu/docs/vapor-gui-help/BarbOverview"));
    help.push_back(make_pair("Renderer control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/renderer-instances"));
    help.push_back(make_pair("Data accuracy control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/refinement-and-lod-control"));
    help.push_back(make_pair("Barb geometry options", "http://www.vapor.ucar.edu/docs/vapor-gui-help/BarbGeometry"));
    help.push_back(make_pair("Barb Appearance settings", "<>"
                                                         "http://www.vapor.ucar.edu/docs/vapor-gui-help/BarbAppearance"));
}

void BarbEventRouter::_initializeTab()
{
    _updateTab();
    BarbParams *rParams = (BarbParams *)GetActiveParams();
    DataMgr *   dataMgr = GetActiveDataMgr();

    _variables->Initialize(rParams, dataMgr);
}

void BarbEventRouter::_updateTab()
{
    // The variable tab updates itself:
    _variables->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _appearance->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _geometry->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());

    _annotation->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());
}

string BarbEventRouter::_getDescription() const
{
    return ("Displays an "
            "array of arrows with the users domain, with custom dimensions that are "
            "defined by the user in the X, Y, and Z axes.  The arrows represent a vector "
            "whos direction is determined by up to three user-defined variables.\n\nBarbs "
            "can have a constant color applied to them, or they may be colored according "
            "to an additional user-defined variable.\n\n");
}
