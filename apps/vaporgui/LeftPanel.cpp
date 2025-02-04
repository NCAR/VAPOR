#include "LeftPanel.h"
#include "RenderersPanel.h"
#include "ImportPanel.h"
#include "AnnotationEventRouter.h"
#include "AnimationTab.h"
#include "ViewpointTab.h"

#include <QScrollArea>
#include <iostream>

LeftPanel::LeftPanel(ControlExec *ce)
{
    auto add = [this](auto &&w, auto &&t) constexpr {
        QScrollArea *scrollArea = new QScrollArea;
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setWidget(w);
        scrollArea->setWidgetResizable(true);
        scrollArea->setEnabled(true);
        addTab(scrollArea, t);
        _uTabs.push_back(w);
    };
    _importPanel = new ImportPanel(ce);
    add(_importPanel, "Import2");
    _testPanel = new TestPanel(ce);
    add(_testPanel, "testPanel");
    add(new RenderersPanel(ce), "Renderers");
    add(new AnnotationEventRouter(ce), "Annotation");
    add(new AnimationTab(ce), "Animation");
    add(new ViewpointTab(ce), "Viewpoint");

    connect(this, &QTabWidget::currentChanged, this, &LeftPanel::tabChanged);
    setEnabled(true);
    _importPanel->setEnabled(true);
    _testPanel->setEnabled(true);
    std::cout << _testPanel->isEnabled() << " " << _importPanel->isEnabled() << " " << isEnabled() << std::endl;
}

void LeftPanel::Update()
{
    std::cout << "Update1 " << _testPanel->isEnabled() << " " << _importPanel->isEnabled() << " " << isEnabled() << std::endl;
    _uTabs[currentIndex()]->Update();
    std::cout << "Update2 " << _testPanel->isEnabled() << " " << _importPanel->isEnabled() << " " << isEnabled() << std::endl;
}

void LeftPanel::tabChanged(int i)
{
    _uTabs[i]->Update();
}

void LeftPanel::UpdateImportPanel() {
    std::cout << "Update3 " << _testPanel->isEnabled() << " " << _importPanel->isEnabled() << " " << isEnabled() << std::endl;
    _importPanel->Update();
    std::cout << "Update4 " << _testPanel->isEnabled() << " " << _importPanel->isEnabled() << " " << isEnabled() << std::endl;
}
