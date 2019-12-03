#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <vapor/glutil.h>
#include <QLineEdit>
#include <QScrollArea>
#include <vector>
#include <string>
#include "vapor/VolumeParams.h"
#include "vapor/VolumeRenderer.h"
#include "VariablesWidget.h"
#include "VolumeEventRouter.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<VolumeEventRouter> registrar(VolumeEventRouter::GetClassType());

VolumeEventRouter::VolumeEventRouter(QWidget *parent, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, VolumeParams::GetClassType())
{
    _variables = new VolumeVariablesSubtab(this);
    QScrollArea *qsvar = new QScrollArea(this);
    qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _variables->adjustSize();
    qsvar->setWidget(_variables);
    qsvar->setWidgetResizable(true);
    addTab(qsvar, "Variables");

    _appearance = new VolumeAppearanceSubtab(this);
    QScrollArea *qsapp = new QScrollArea(this);
    qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsapp->setWidget(_appearance);
    qsapp->setWidgetResizable(true);
    addTab(qsapp, "Appearance");

    _geometry = new VolumeGeometrySubtab(this);
    QScrollArea *qsgeo = new QScrollArea(this);
    qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsgeo->setWidget(_geometry);
    qsgeo->setWidgetResizable(true);
    addTab(qsgeo, "Geometry");

    _annotation = new VolumeAnnotationSubtab(this);
    QScrollArea *qsAnnotation = new QScrollArea(this);
    qsAnnotation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsAnnotation->setWidget(_annotation);
    qsAnnotation->setWidgetResizable(true);
    addTab(qsAnnotation, "Annotation");
}

void VolumeEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Volume Overview", "http://www.vapor.ucar.edu/docs/vapor-gui-help/Volume#VolumeOverview"));

    help.push_back(make_pair("Renderer control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/renderer-instances"));

    help.push_back(make_pair("Data accuracy control", "http://www.vapor.ucar.edu/docs/vapor-how-guide/refinement-and-lod-control"));

    help.push_back(make_pair("Volume geometry options", "http://www.vapor.ucar.edu/docs/vapor-gui-help/twoD#TwoDGeometry"));

    help.push_back(make_pair("Volume Appearance settings", "http://www.vapor.ucar.edu/docs/vapor-gui-help/Volume#VolumeAppearance"));
}

void VolumeEventRouter::_updateTab()
{
    // The variable tab updates itself:
    //
    _variables->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _appearance->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _geometry->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());

    _annotation->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());
}

string VolumeEventRouter::_getDescription() const
{
    return ("Displays "
            "the user's 3D data variables within a volume described by the source data "
            "file, according to color and opacity settings defined by the user.\n\n"
            "These 3D variables may be offset by a height variable.\n\n");
}
