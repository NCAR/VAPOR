#ifdef WIN32
    #pragma warning(disable : 4100)
#endif

#include <vapor/glutil.h>
#include <vapor/ImageParams.h>
// #include <vapor/ImageRenderer.h>
#include <ImageEventRouter.h>
#include <EventRouter.h>
#include <qlineedit.h>
#include <QFileDialog>
#include <qscrollarea.h>

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<ImageEventRouter> registrar(ImageEventRouter::GetClassType());

ImageEventRouter::ImageEventRouter(QWidget *parent, ControlExec *ce) : QTabWidget(parent), RenderEventRouter(ce, ImageParams::GetClassType())
{
    sizePolicy().setVerticalPolicy(QSizePolicy::Maximum);

    _variables = new ImageVariablesSubtab(this);
    QScrollArea *qsvar = new QScrollArea(this);
    qsvar->sizePolicy().setVerticalPolicy(QSizePolicy::Maximum);
    qsvar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _variables->adjustSize();
    qsvar->setWidget(_variables);
    qsvar->setWidgetResizable(true);
    addTab(qsvar, "Variables");

    _appearance = new ImageAppearanceSubtab(this);
    QScrollArea *qsapp = new QScrollArea(this);
    qsapp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsapp->setWidget(_appearance);
    qsapp->setWidgetResizable(true);
    addTab(qsapp, "Appearance");

    _geometry = new ImageGeometrySubtab(this);
    QScrollArea *qsgeo = new QScrollArea(this);
    qsgeo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    qsgeo->setWidget(_geometry);
    qsgeo->setWidgetResizable(true);
    addTab(qsgeo, "Geometry");
}

void ImageEventRouter::GetWebHelp(vector<pair<string, string>> &help) const { help.clear(); }

void ImageEventRouter::_updateTab()
{
    _variables->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());

    _appearance->Update(GetActiveDataMgr(), _controlExec->GetParamsMgr(), GetActiveParams());
    _geometry->Update(_controlExec->GetParamsMgr(), GetActiveDataMgr(), GetActiveParams());
}

string ImageEventRouter::_getDescription() const
{
    return ("Displays a "
            "georeferenced image that is automatically reprojected and fit to the user's"
            "data, as long as the data contains georeference metadata.  The image "
            "renderer may be offset by a height variable to show bathymetry or mountainous"
            " terrain.\n\n ");
}
