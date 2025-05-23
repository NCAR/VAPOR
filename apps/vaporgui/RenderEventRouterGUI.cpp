#include "RenderEventRouterGUI.h"
#include "UWidget.h"
#include <QScrollArea>
#include "VContainer.h"
#include <vapor/ControlExecutive.h>

const std::string RenderEventRouterGUI::VariablesTabName = "Variables";
const std::string RenderEventRouterGUI::AppearanceTabName = "Appearance";
const std::string RenderEventRouterGUI::GeometryTabName = "Geometry";
const std::string RenderEventRouterGUI::ColorbarTabName = "Colorbar";

RenderEventRouterGUI::RenderEventRouterGUI(VAPoR::ControlExec *ce, string paramsType)
: RenderEventRouter(ce)
{
    connect(this, &QTabWidget::currentChanged, this, &RenderEventRouterGUI::tabChanged);
}


QWidget *RenderEventRouterGUI::AddSubtab(string title, UWidget *subtab)
{
    VContainer *container = new VContainer(subtab);
    container->AddBottomStretch();
    container->SetPadding(0, 6, 0, 0);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);

    addTab(scrollArea, QString::fromStdString(title));
    _subtabs.push_back(subtab);

    return scrollArea;
}

void RenderEventRouterGUI::Update()
{
    auto *params = GetActiveParams();
    auto *paramsMgr = _controlExec->GetParamsMgr();
    auto *dataMgr = GetActiveDataMgr();
    if (!(params && paramsMgr && dataMgr)) return;

    auto       gp = getGUIStateParams();
    bool       hasTab = false;
    const auto activeTab = QString::fromStdString(gp->ActiveTab());
    for (int i = 0; i <= count(); i++) {
        if (activeTab == tabText(i)) {
            hasTab = true;
            setTab(i);
            break;
        }
    }
    if (!hasTab) { setTab(0); }

    for (ParamsUpdatable *subtab : _subtabs) subtab->Update(params, paramsMgr, dataMgr);
}

void RenderEventRouterGUI::setTab(int i)
{
    blockSignals(true);
    setCurrentIndex(i);
    blockSignals(false);
}

void RenderEventRouterGUI::tabChanged(int i)
{
    getGUIStateParams()->SetActiveTab(tabText(i).toStdString());
}

GUIStateParams *RenderEventRouterGUI::getGUIStateParams() const { return (GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()); }
