#include "ImportTab.h"
#include "VLabel.h"
#include "VSection.h"
#include "PImportData.h"
#include "MainForm.h"
#include <vapor/GUIStateParams.h>
#include <vapor/ControlExecutive.h>
#include <QVBoxLayout>

using namespace VAPoR;

ImportTab::ImportTab(VAPoR::ControlExec *ce, MainForm *mf) : _ce(ce)
{
    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);

    l->addWidget(new VSectionGroup("Import Data", {_importer = new PImportData(_ce, mf)}));
    connect(_importer, &PImportData::dataImported, this, &ImportTab::DataImported);

    //l->addWidget(new PImportData2("Import Data2");

    VSectionGroup *sg = new VSectionGroup("Tips", {
        new VHyperlink(
            "How to convert Non-Compliant NetCDF Files?",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/dataFormatRequirements/netCDF/nonCompliantNetCDF.html",
            true
        ),
        new VHyperlink(
            "Download example Datasets",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/downloads/sampleData.html",
            true
        ),
        new VHyperlink(
            "Additional information on supported data formats",
            "https://vapordocumentationwebsite.readthedocs.io/en/latest/dataFormatRequirements.html",
            true
        ),
        new VHyperlink(
            "Get Help",
            "https://vapor.discourse.group/",
            true
        ),
    });
    sg->setEnabled(true);
    l->addWidget(sg);

    l->addStretch(1);
    setLayout(l);
}

void ImportTab::Update()
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    _importer->Update(guiParams);
}

void ImportTab::DataImported() { emit dataImported(); }
