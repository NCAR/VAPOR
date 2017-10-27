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
#include "vapor/ContourParams.h"
#include "VariablesWidget.h"
#include "ContourEventRouter.h"
#include "EventRouter.h"

using namespace VAPoR;

ContourEventRouter::ContourEventRouter(QWidget *parent, VizWinMgr *vizMgr, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, ContourParams::GetClassType())
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
}

// Destructor does nothing
ContourEventRouter::~ContourEventRouter()
{
    if (_variables) delete _variables;
#ifdef DEAD
    if (_image) delete _image;
#endif
    if (_geometry) delete _geometry;
    if (_appearance) delete _appearance;
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

    cout << "ContourEventRouter::_initializeTab() : override param defaults here" << endl;
    rParams->SetVariables3D(false);
}

void ContourEventRouter::_updateTab()
{
    // The variable tab updates itself:
    _variables->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _appearance->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _geometry->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());
}
