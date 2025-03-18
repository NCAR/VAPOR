#include "RendererInspector.h"
#include "RenderEventRouter.h"
#include <QDebug>
#include <vapor/GUIStateParams.h>
#include <vapor/ControlExecutive.h>

using namespace VAPoR;
using std::vector;
using std::string;
#include "QTabWidget"

RendererInspector::RendererInspector(VAPoR::ControlExec *ce)
: _ce(ce)
{
    vector<string> rendererNames = RenderEventRouterFactory::Instance()->GetFactoryNames();

    for (auto name : rendererNames) {
        RenderEventRouter *re = RenderEventRouterFactory::Instance()->CreateInstance(name, NULL, _ce);
        QWidget *w = dynamic_cast<QWidget*>(re);
        VAssert(w);
        VRouter::Add(w);
        _classToInspectorMap[name] = std::pair<RenderEventRouter*,QWidget*>(re, w);
    }
}

void RendererInspector::Update()
{
    ParamsMgr *paramsMgr = _ce->GetParamsMgr();
    GUIStateParams *guiParams = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    bool noDatasetLoaded = _ce->GetParams<GUIStateParams>()->GetOpenDataSetNames().empty();
    if (noDatasetLoaded) return;
    
    string currentViz = guiParams->GetActiveVizName();
    string renClass, renInst, _;
    guiParams->GetActiveRenderer(currentViz, renClass, renInst);

    if (renInst.empty() || _classToInspectorMap.count(renClass) == 0 || !paramsMgr->RenderParamsLookup(renInst, _, _, _)) {
        Show(nullptr);
        return;
    }

    Show(_classToInspectorMap[renClass].second);
    _classToInspectorMap[renClass].first->SetActive(renInst);
    _classToInspectorMap[renClass].first->Update();
}

std::vector<RenderEventRouter *> RendererInspector::GetRenderEventRouters() const
{
    vector<RenderEventRouter*> v;
    for (auto it = _classToInspectorMap.begin(); it != _classToInspectorMap.end(); ++it)
        v.push_back(it->second.first);
    return v;
}
