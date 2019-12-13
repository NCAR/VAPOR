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
#include "vapor/FlowParams.h"
#include "VariablesWidget.h"
#include "FlowEventRouter.h"
#include "EventRouter.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<FlowEventRouter> registrar(FlowEventRouter::GetClassType());

FlowEventRouter::FlowEventRouter(QWidget *parent, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, FlowParams::GetClassType())
{
    _variables = new FlowVariablesSubtab(this);
    QScrollArea *qsvar = new QScrollArea(this);
    qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _variables->adjustSize();
    qsvar->setWidget(_variables);
    qsvar->setWidgetResizable(true);
    addTab(qsvar, "Variables");

    _seeding = new FlowSeedingSubtab(this);
    QScrollArea *qsseed = new QScrollArea(this);
    qsseed->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsseed->setWidget(_seeding);
    qsseed->setWidgetResizable(true);
    _seedingTab = qsseed;
    addTab(qsseed, "Seeding");

    _appearance = new FlowAppearanceSubtab(this);
    QScrollArea *qsapp = new QScrollArea(this);
    qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsapp->setWidget(_appearance);
    qsapp->setWidgetResizable(true);
    addTab(qsapp, "Appearance");

    _geometry = new FlowGeometrySubtab(this);
    QScrollArea *qsgeo = new QScrollArea(this);
    qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsgeo->setWidget(_geometry);
    qsgeo->setWidgetResizable(true);
    addTab(qsgeo, "Geometry");

    _annotation = new FlowAnnotationSubtab(this);
    QScrollArea *qsAnnotation = new QScrollArea(this);
    qsAnnotation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsAnnotation->setWidget(_annotation);
    qsAnnotation->setWidgetResizable(true);
    addTab(qsAnnotation, "Annotations");
}

void FlowEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

#ifdef VAPOR_3_0
    help.push_back(make_pair("Flow Overview", "http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#FlowOverview"));

    help.push_back(make_pair("Renderer control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/renderer-instances"));

    help.push_back(make_pair("Data accuracy control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/refinement-and-lod-control"));

    help.push_back(make_pair("Flow geometry options", "http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#FlowGeometry"));

    help.push_back(make_pair("Flow Appearance settings", "http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#HelloAppearance"));
#endif
}

void FlowEventRouter::_updateTab()
{
    // The variable tab updates itself:
    //
    _variables->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _appearance->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());
    _seeding->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());
    _geometry->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());
    _annotation->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());

    // Sync selected tab with GUIStateParams
    GUIStateParams *gp = (GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    if (gp->IsFlowSeedTabActive()) {
        if (currentWidget() != _seedingTab) {
            blockSignals(true);
            setCurrentWidget(_seedingTab);
            blockSignals(false);
        }
    } else {
        if (currentWidget() == _seedingTab) {
            blockSignals(true);
            setCurrentIndex(1);
            blockSignals(false);
        }
    }
}

string FlowEventRouter::_getDescription() const { return ("Displays steady or unsteady flow trajectories through the user's domain.\n"); }
