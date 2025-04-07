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
    add(_importTab, "Import");
    add(new RenderersPanel(ce), "Render");
    add(new AnnotationEventRouter(ce), "Annotate");
    add(new ExportTab(ce, mf), "Export");

    for (int i=1 ; i<count(); ++i) setTabEnabled(i, false);
    setTabEnabled(0,true);

    connect(this, &QTabWidget::currentChanged, this, &LeftPanel::tabChanged);
}

void LeftPanel::setEnabled(bool widgetsEnabled) {
    if (!widgetsEnabled) {
        // Only enable the Import tab, which should always be enabled
        //setEnabled(true);
        _importTab->setEnabled(true);
        setCurrentIndex(0);
        for (int tabNum=1; tabNum < count(); tabNum++) setTabEnabled(tabNum, false);
    }
    else {
        // Otherwise enable all tabs
        for (int i = 0; i < count(); ++i) setTabEnabled(i, true);
    }
}

void LeftPanel::Update()
{
    if (_uTabs.empty()) return;
    _uTabs[currentIndex()]->Update();
}

void LeftPanel::tabChanged(int i)
{
    _uTabs[i]->Update();
}

void LeftPanel::GoToRendererTab() {
    setCurrentIndex(1);
}
