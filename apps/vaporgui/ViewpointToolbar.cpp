#include "ViewpointToolbar.h"
#include <QComboBox>
#include "VizWinMgr.h"
#include "PVisualizerSelector.h"
#include <vapor/GUIStateParams.h>
#include <vapor/NavigationUtils.h>

#include "images/tiles.xpm"
#include "images/home.xpm"
#include "images/sethome.xpm"
#include "images/eye.xpm"

ViewpointToolbar::ViewpointToolbar(QWidget *parent, VAPoR::ControlExec *ce, VizWinMgr *vwm)
: QToolBar(parent), _ce(ce), _vizWinMgr(vwm)
{

    _visualizerSelector = new PVisualizerSelector();
    addWidget(_visualizerSelector);

    addAction(QPixmap(tiles), "Tile Windows", [this](){_vizWinMgr->FitSpace();});
    addAction(QPixmap(home), "Go to Home View", [this](){NavigationUtils::UseHomeViewpoint(_ce);});
    addAction(QPixmap(sethome), "Set Home View", [this](){NavigationUtils::SetHomeViewpoint(_ce);});
    addAction(QPixmap(eye), "View All", [this](){NavigationUtils::ViewAll(_ce);});

    auto alignViewCombo = new QComboBox(this);
    alignViewCombo->addItems({"Align View", "Nearest axis", "+ X", "+ Y", "+ Z", "- X", "- Y", "- Z (Default)"});
    alignViewCombo->setToolTip("Rotate view to an axis-aligned viewpoint, centered on current rotation center.");
    QObject::connect(alignViewCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int i){
        NavigationUtils::AlignView(_ce, i);
        sender()->blockSignals(true);
        ((QComboBox *)sender())->setCurrentIndex(0);
        sender()->blockSignals(false);
    });
    addWidget(alignViewCombo);
}


void ViewpointToolbar::Update()
{
    _visualizerSelector->Update(_ce->GetParams<GUIStateParams>(), _ce->GetParamsMgr());
}
