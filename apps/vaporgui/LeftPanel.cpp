#include "LeftPanel.h"
#include "RenderersPanel.h"
#include "AnnotationEventRouter.h"
#include "AnimationTab.h"
#include "ViewpointTab.h"

#include <QScrollArea>

LeftPanel::LeftPanel(ControlExec *ce)
{
    auto add = [this](auto &&w, auto &&t) constexpr {
        QScrollArea *scrollArea = new QScrollArea;
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setWidget(w);
        scrollArea->setWidgetResizable(true);
        addTab(scrollArea, t);
        _uTabs.push_back(w);
    };
    add(new RenderersPanel(ce), "Renderers");
    add(new AnnotationEventRouter(ce), "Annotation");
    add(new AnimationTab(ce), "Animation");
    add(new ViewpointTab(ce), "Viewpoint");

    connect(this, &QTabWidget::currentChanged, this, &LeftPanel::tabChanged);
}

void LeftPanel::setEnabled(bool widgetsEnabled) {
    if (!widgetsEnabled) {
        // Only enable the Import tab, which should always be enabled
        setEnabled(true);
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
