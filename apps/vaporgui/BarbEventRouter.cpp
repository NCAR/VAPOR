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
#include <vapor/TransferFunction.h>
#include "VizWin.h"
#include "vapor/BarbParams.h"
#include "VariablesWidget.h"
#include "BarbEventRouter.h"
#include "EventRouter.h"

using namespace VAPoR;

BarbEventRouter::BarbEventRouter(QWidget *parent, VizWinMgr *vizMgr, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, BarbParams::GetClassType())
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
}

// Destructor does nothing
BarbEventRouter::~BarbEventRouter()
{
    if (_variables) delete _variables;
#ifdef DEAD
    if (_image) delete _image;
#endif
    if (_geometry) delete _geometry;
    if (_appearance) delete _appearance;
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

void BarbEventRouter::_updateTab()
{
    // The variable tab updates itself:
    _variables->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _appearance->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _geometry->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());
}
