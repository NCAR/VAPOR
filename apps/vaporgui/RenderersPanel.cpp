#include "RenderersPanel.h"
#include <QSplitter>
#include <vapor/GUIStateParams.h>
#include <vapor/ControlExecutive.h>
#include "RendererList.h"
#include "RendererInspector.h"
#include "DatasetInspector.h"
#include "VRouter.h"

using namespace VAPoR;

RenderersPanel::RenderersPanel(VAPoR::ControlExec *ce)
: VContainer(_splitter = new QSplitter(Qt::Orientation::Vertical)),
  _ce(ce)
{
    _splitter->addWidget(_renList = new RendererList(_ce));
    _splitter->addWidget(_inspectorRouter = new VRouter);
    _splitter->setChildrenCollapsible(false);
    _inspectorRouter->Add(_renInspector = new RendererInspector(_ce));
    _inspectorRouter->Add(_dataInspector = new DatasetInspector(_ce));
}

void RenderersPanel::Update()
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());

    _renList->Update();

    if (!guiParams->GetActiveDataset().empty() && guiParams->GetActiveRendererInst().empty()) {
        _inspectorRouter->Show(_dataInspector);
        _dataInspector->Update();
    } else {
        _inspectorRouter->Show(_renInspector);
        _renInspector->Update();
    }
}
