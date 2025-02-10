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
#include "PGroup.h"
#include "PSection.h"

using namespace VAPoR;

ImportPanel::ImportPanel(VAPoR::ControlExec *ce) : _ce(ce)
//: VContainer(_splitter = new QSplitter(Qt::Orientation::Vertical)), _ce(ce)
{
    //_section = new PSection("Import Data", {_importer = new PImportData(_ce)});
    //_section->AlwaysEnable();
    _section = new VSectionGroup("Import Data", {_importer = new PImportData(_ce)});
    //_section->layout()->add(_importer = new PImportData(_ce));

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    layout()->addWidget(_section);
    l->addStretch();

    //_importer = new PImportData(_ce);
    //_splitter->addWidget(_importer);
    //_splitter->setEnabled(true);

    //_splitter->addWidget(_renList = new RendererList(_ce));
    //_splitter->addWidget(_inspectorRouter = new VRouter);
    //_splitter->setChildrenCollapsible(false);

    //_inspectorRouter->Add(_renInspector = new RendererInspector(_ce));
    //_inspectorRouter->Add(_dataInspector = new DatasetInspector(_ce));
}

void ImportPanel::Update()
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    _importer->Update(guiParams);
    
    //GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());

    //_importer->Update(guiParams);

    //_renList->Update();

    //if (!guiParams->GetActiveDataset().empty() && guiParams->GetActiveRendererInst().empty()) {
    //    _inspectorRouter->Show(_dataInspector);
    //    _dataInspector->Update();
    //} else {
    //    _inspectorRouter->Show(_renInspector);
    //    _renInspector->Update();
    //}
}
