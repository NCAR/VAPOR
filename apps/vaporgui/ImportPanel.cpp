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
    _importer = new PImportData(_ce);
    //_splitter->addWidget(_importer = new PImportData(_ce));
    _splitter->addWidget(_importer);
    _splitter->setEnabled(true);
    //_importer->setEnabled(true); nothing
    //setEnabled(true); nothing

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
    // _importer->setEnabled(true); nothing

    _renList->Update();

    if (!guiParams->GetActiveDataset().empty() && guiParams->GetActiveRendererInst().empty()) {
        _inspectorRouter->Show(_dataInspector);
        _dataInspector->Update();
    } else {
        _inspectorRouter->Show(_renInspector);
        _renInspector->Update();
    }
}

#include "PCheckbox.h"
TestPanel::TestPanel(VAPoR::ControlExec* ce) : VContainer(_splitter = new QSplitter(Qt::Orientation::Vertical)), _ce(ce) {
    _cb = new PCheckbox(GUIStateParams::trashTag, "trash");
    _splitter->addWidget(_cb);
}

void TestPanel::Update() {
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    bool onoff = guiParams->GetValueLong(GUIStateParams::trashTag, false);
    std::cout << "Updating TestPanel " << onoff << std::endl;
}
