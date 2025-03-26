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

void LeftPanel::Update()
{
    GUIStateParams *gsp = _ce->GetParams<GUIStateParams>();
    // Enable the only import panel when no data is loaded
    bool noDatasetLoaded = gsp->GetOpenDataSetNames().empty();
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

    if (gsp->GetValueLong(GUIStateParams::DataJustLoadedTag, 0) != 0) {
        setCurrentIndex(1); // go to Renderer tab
        gsp->SetValueLong(GUIStateParams::DataJustLoadedTag, "Unsetting data just loaded tag", 0);
    }
}

void LeftPanel::tabChanged(int i)
{
    _uTabs[i]->Update();
}
