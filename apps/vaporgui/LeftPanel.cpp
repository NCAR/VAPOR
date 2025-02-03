#include "LeftPanel.h"
#include "RenderersPanel.h"
#include "ImportPanel.h"
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
    add(new ImportPanel(ce), "Import");
    add(new RenderersPanel(ce), "Renderers");
    add(new AnnotationEventRouter(ce), "Annotation");
    add(new AnimationTab(ce), "Animation");
    add(new ViewpointTab(ce), "Viewpoint");

    connect(this, &QTabWidget::currentChanged, this, &LeftPanel::tabChanged);
}

void LeftPanel::Update()
{
    _uTabs[currentIndex()]->Update();
}

void LeftPanel::tabChanged(int i)
{
    _uTabs[i]->Update();
}
