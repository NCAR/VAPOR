#include "LeftPanel.h"
#include "RenderersPanel.h"
#include "ImportPanel.h"
#include "AnnotationEventRouter.h"
#include "AnimationTab.h"
#include "ViewpointTab.h"
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
        addTab(scrollArea, t);
        _uTabs.push_back(w);
    };
   
    _importPanel = new ImportPanel(ce); 
    add(_importPanel, "Import");
    //add(new ImportPanel(ce), "Import");
    //RenderersPanel* rp = new RenderersPanel(ce);
    //rp->setEnabled(false);
    //add(rp, "Renderers2");
    add(new RenderersPanel(ce), "Renderers");
    add(new AnnotationEventRouter(ce), "Annotation");
    add(new AnimationTab(ce), "Animation");
    add(new ViewpointTab(ce), "Viewpoint");

    connect(this, &QTabWidget::currentChanged, this, &LeftPanel::tabChanged);
}

void LeftPanel::Update()
{
    bool noDatasetLoaded = _ce->GetParams<GUIStateParams>()->GetOpenDataSetNames().empty();
    if (noDatasetLoaded && currentIndex()!=0) {
        return;
    }
    std::cout << "Updating " << currentIndex() << std::endl;
    _uTabs[currentIndex()]->Update();
}

void LeftPanel::UpdateImportMenu() {
    //_uTabs[0]->Update();
    setEnabled(true);
    _importPanel->setEnabled(true);
    _importPanel->Update();
}

void LeftPanel::tabChanged(int i)
{
    bool noDatasetLoaded = _ce->GetParams<GUIStateParams>()->GetOpenDataSetNames().empty();
    if (noDatasetLoaded && currentIndex()!=0) {
        return;
    }
    std::cout << "Updating " << currentIndex() << std::endl;
    _uTabs[i]->Update();
}
