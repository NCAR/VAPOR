#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#include <vapor/glutil.h>
#include <vapor/ImageParams.h>
#include <vapor/ImageRenderer.h>
#include <ImageEventRouter.h>
#include <EventRouter.h>
#include <qlineedit.h>
#include <QFileDialog>
#include <qscrollarea.h>

using namespace VAPoR;

ImageEventRouter::ImageEventRouter(QWidget *parent, ControlExec *ce)
    : QTabWidget(parent),
      RenderEventRouter(ce, ImageParams::GetClassType()) {
    _variables = new ImageVariablesSubtab(this);
    QScrollArea *qsvar = new QScrollArea(this);
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

ImageEventRouter::~ImageEventRouter() {
    if (_variables)
        delete _variables;
    if (_geometry)
        delete _geometry;
    if (_appearance)
        delete _appearance;
}

void ImageEventRouter::GetWebHelp(vector<pair<string, string>> &help) const {
    help.clear();
}

void ImageEventRouter::_updateTab() {
    _variables->Update(GetActiveDataMgr(),
                       _controlExec->GetParamsMgr(),
                       GetActiveParams());

    _appearance->Update(GetActiveDataMgr(),
                        _controlExec->GetParamsMgr(),
                        GetActiveParams());
    _geometry->Update(_controlExec->GetParamsMgr(),
                      GetActiveDataMgr(),
                      GetActiveParams());
}
