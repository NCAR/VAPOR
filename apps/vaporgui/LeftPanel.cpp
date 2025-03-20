#include "LeftPanel.h"
#include "MainForm.h"
#include "RenderersPanel.h"
#include "ImportTab.h"
#include "ExportTab.h"
#include "vapor/ControlExecutive.h"
#include "AnnotationEventRouter.h"
#include "vapor/GUIStateParams.h"

#include <QScrollArea>

LeftPanel::LeftPanel(ControlExec *ce, MainForm *mf) : _ce(ce)
{
    auto add = [this](auto &&w, auto &&t) constexpr {
        QScrollArea *scrollArea = new QScrollArea;
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setWidget(w);
        scrollArea->setWidgetResizable(true);
        addTab(scrollArea, t);
        _uTabs.push_back(w);
    };
   
    _importTab = new ImportTab(_ce, mf); 
    connect(_importTab, &ImportTab::dataImported, this, &LeftPanel::GoToRendererTab);
    add(_importTab, "Import");
    add(new RenderersPanel(ce), "Render");
    add(new AnnotationEventRouter(ce), "Annotate");
    add(new ExportTab(ce, mf), "Export");

    for (int i=1 ; i<count(); ++i) setTabEnabled(i, false);
    setTabEnabled(0,true);

    connect(this, &QTabWidget::currentChanged, this, &LeftPanel::tabChanged);
}

void LeftPanel::Update()
{
    // Enable the only import panel when no data is loaded
    bool noDatasetLoaded = _ce->GetParams<GUIStateParams>()->GetOpenDataSetNames().empty();
    if (noDatasetLoaded) { 
        setEnabled(true);
        _importTab->setEnabled(true);
        _importTab->Update();
        setCurrentIndex(0);
        for (int tabNum=1; tabNum < count(); tabNum++) setTabEnabled(tabNum, false);
        return;
    }

    // Otherwise enable and update all tabs
    for (int i = 0; i < count(); ++i) setTabEnabled(i, true);
    setTabEnabled(currentIndex(),true);
    _uTabs[currentIndex()]->Update();
}

void LeftPanel::tabChanged(int i)
{
    _uTabs[i]->Update();
}

void LeftPanel::GoToRendererTab() { 
    setCurrentIndex(1); 
}
