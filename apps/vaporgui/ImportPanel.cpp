#include "ImportPanel.h"
#include <QSplitter>
#include <vapor/GUIStateParams.h>
#include <vapor/ControlExecutive.h>
#include "RendererList.h"
#include "RendererInspector.h"
#include "DatasetInspector.h"
#include "PDatasetImportTypeSelector.h"
#include "VRouter.h"
#include "PImportData.h"

using namespace VAPoR;

ImportPanel::ImportPanel(VAPoR::ControlExec *ce)
: VContainer(_splitter = new QSplitter(Qt::Orientation::Vertical)),
  _ce(ce)
{
    std::vector<std::string> list = {"foo", "bar", "baz"};
    //_splitter->addWidget(_importer = new PDatasetImportTypeSelector(list));
    //_splitter->addWidget(_importer = new PImportData(list));
    _splitter->addWidget(_importer = new PImportDataSection);

    _splitter->addWidget(_renList = new RendererList(_ce));
    _splitter->addWidget(_inspectorRouter = new VRouter);
    _splitter->setChildrenCollapsible(false);

    _inspectorRouter->Add(_renInspector = new RendererInspector(_ce));
    _inspectorRouter->Add(_dataInspector = new DatasetInspector(_ce));
}

void ImportPanel::Update()
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());

    _importer->Update(guiParams);

    _renList->Update();

    if (!guiParams->GetActiveDataset().empty() && guiParams->GetActiveRendererInst().empty()) {
        _inspectorRouter->Show(_dataInspector);
        _dataInspector->Update();
    } else {
        _inspectorRouter->Show(_renInspector);
        _renInspector->Update();
    }
}
