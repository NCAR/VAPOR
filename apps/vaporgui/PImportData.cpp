#include "DatasetTypeLookup.h"
#include "MainForm.h"
#include "PImportData.h"
#include "PFileSelector.h"
#include "PRadioButtons.h"
#include "PGroup.h"
#include "PButton.h"
#include "PLabel.h"
#include "VLabel.h"
#include "VHBoxWidget.h"
#include "vapor/ControlExecutive.h"
#include "vapor/GUIStateParams.h"
#include "vapor/FileUtils.h"

#include <QFileDialog>

PImportDataWidget::PImportDataWidget(VAPoR::ControlExec* ce, MainForm *mf) : PLineItem("", "Import Files", _hBox = new VHBoxWidget()), _ce(ce), _mf(mf) {
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(_hBox->layout());
    layout->addWidget(_fileLabel = new VLabel(""),3);
    layout->addWidget(_importButton = new PButton("Select\nFiles", [this](VAPoR::ParamsBase*){_importDataset();}),1);

    _fileLabel->MakeSelectable();
}

void PImportDataWidget::_importDataset() {
    std::vector<std::string> fileNames;
    std::string defaultPath = getParams()->GetValueString(GUIStateParams::ImportDataDirTag, FileUtils::HomeDir());
    QStringList qfileNames = QFileDialog::getOpenFileNames(this, "Select Filename Prefix", QString::fromStdString(defaultPath));
    for (const QString &qStr : qfileNames) fileNames.push_back(qStr.toStdString());    
    if (fileNames.empty()) return;

    std::string format = DatasetTypeShortName( getParams()->GetValueString(GUIStateParams::ImportDataTypeTag,"") );
    _mf->ImportDataset(fileNames, format);

    // Need return code from control exec here
    if (true) getParams()->SetValueStringVec(GUIStateParams::ImportDataFilesTag, "", fileNames);

    //emit dataImported();
}

void PImportDataWidget::updateGUI() const {
    std::vector<string> files = getParams()->GetValueStringVec(GUIStateParams::ImportDataFilesTag);
    int count = files.size();
    if (count > 0) {
        std::string dir = FileUtils::Dirname(files[0]);
        _fileLabel->SetText("Imported " + std::to_string(count) + " files\nFrom: " + dir);
    }
    else _fileLabel->SetText("");
    _importButton->Update(getParams());
}

PImportData::PImportData(VAPoR::ControlExec* ce, MainForm* mf) : PWidget("", _group = new PGroup()), _ce(ce), _mf(mf) {
    std::vector<std::string> types = GetDatasetTypeDescriptions();
    PRadioButtons* prb = new PRadioButtons(GUIStateParams::ImportDataTypeTag, types);
    _group->Add(prb);
    
    //_selector = new PFilesOpenSelector(GUIStateParams::ImportDataFilesTag, "Import Files" );
    //connect(_selector, &PFilesOpenSelector::filesSelected, this, &PImportData::importDataset);
    //_group->Add(_selector);

    _group->Add(new PImportDataWidget(ce, mf));
    //_captureButton = new PButton("Capture\nCurrent Frame", [this](VAPoR::ParamsBase*){_captureSingleImage();});)
}

void PImportData::importDataset() {
    std::cout << "Removed" << std::endl;
}

void PImportData::updateGUI() const {
    _group->Update(getParams());
    //_selector->Update(getParams());
}
