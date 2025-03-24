#include "PImportDataButton.h"
#include "MainForm.h"
#include "VHBoxWidget.h"
#include "VLabel.h"
#include "PButton.h"
#include "DatasetTypeLookup.h"
#include "vapor/ControlExecutive.h"
#include "vapor/GUIStateParams.h"
#include "vapor/FileUtils.h"

#include <QFileDialog>

PImportDataButton::PImportDataButton(VAPoR::ControlExec* ce, MainForm *mf) : PWidget("", _hBox = new VHBoxWidget()), _ce(ce), _mf(mf) {
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(_hBox->layout());
    layout->addWidget(_importButton = new PButton("Select File(s)", [this](VAPoR::ParamsBase*){_importDataset();}),1);
    layout->addWidget(_fileLabel = new VLabel(""),3);

    _fileLabel->MakeSelectable();
}

void PImportDataButton::_importDataset() {
    std::vector<std::string> fileNames;
    std::string defaultPath = getParams()->GetValueString(GUIStateParams::ImportDataDirTag, FileUtils::HomeDir());
    QStringList qfileNames = QFileDialog::getOpenFileNames(this, "Select Filename Prefix", QString::fromStdString(defaultPath));
    for (const QString &qStr : qfileNames) fileNames.push_back(qStr.toStdString());
    if (fileNames.empty()) return;

    //std::string format = DatasetTypeShortName( getParams()->GetValueString(GUIStateParams::ImportDataTypeTag,"") );
    std::string format = GetDatasets()[getParams()->GetValueLong(GUIStateParams::ImportDataTypeTag, 0)].first;
    _mf->ImportDataset(fileNames, format);

    // Need return code from control exec here
    if (true) getParams()->SetValueStringVec(GUIStateParams::ImportDataFilesTag, "", fileNames);
}

void PImportDataButton::updateGUI() const {
    std::vector<string> files = getParams()->GetValueStringVec(GUIStateParams::ImportDataFilesTag);
    int count = files.size();
    if (count > 0) {
        std::string dir = FileUtils::Dirname(files[0]);
        _fileLabel->SetText("Imported " + std::to_string(count) + " file(s)\nFrom: " + dir);
    }
    else _fileLabel->SetText("");
    _importButton->Update(getParams());
}
