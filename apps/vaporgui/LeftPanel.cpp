#include "LeftPanel.h"
#include "RenderersPanel.h"
#include "ImportTab.h"
#include "AnnotationEventRouter.h"
#include "AnimationTab.h"
#include "ViewpointTab.h"
#include "ExportTab.h"
#include "vapor/ControlExecutive.h"
#include "vapor/GUIStateParams.h"

#include <QScrollArea>
#include <iostream>

LeftPanel::LeftPanel(ControlExec *ce) : _ce(ce)
{
    auto add = [this](auto &&w, auto &&t) constexpr {
        QScrollArea *scrollArea = new QScrollArea;
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setWidget(w);
        scrollArea->setWidgetResizable(true);
        //scrollArea->setEnabled(false);
        addTab(scrollArea, t);
        _uTabs.push_back(w);
    };
   
    _importTab = new ImportTab(ce); 
    connect(_importTab, &ImportTab::dataImported, this, &LeftPanel::_goToRendererTab);
    add(_importTab, "Import");
    add(new RenderersPanel(ce), "Render");
    add(new AnnotationEventRouter(ce), "Annotate");
    add(new AnimationTab(ce), "Animation");
    add(new ViewpointTab(ce), "Viewpoint");
    add(new ExportTab(ce), "Export");

    for (int i=1 ; i<count(); ++i) setTabEnabled(i, false);
    setTabEnabled(0,true);
    //setCurrentIndex(0);

    connect(this, &QTabWidget::currentChanged, this, &LeftPanel::tabChanged);
}

void LeftPanel::Update()
{
    bool noDatasetLoaded = _ce->GetParams<GUIStateParams>()->GetOpenDataSetNames().empty();
    if (noDatasetLoaded) return;
    for (int i = 0; i < count(); ++i) setTabEnabled(i, true);
    setTabEnabled(currentIndex(),true);
    _uTabs[currentIndex()]->Update();
}

void LeftPanel::UpdateImportMenu() {
    setEnabled(true);
    _importTab->setEnabled(true);
    _importTab->Update();
}

void LeftPanel::tabChanged(int i)
{
    _uTabs[i]->Update();
}

void LeftPanel::_goToRendererTab() { setCurrentIndex(1); }
