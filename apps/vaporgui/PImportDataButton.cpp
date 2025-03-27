#include "PImportDataButton.h"
#include "MainForm.h"
#include "VHBoxWidget.h"
#include "VLabel.h"
#include "DatasetTypeLookup.h"
#include "vapor/ControlExecutive.h"
#include "vapor/GUIStateParams.h"
#include "vapor/FileUtils.h"

#include <QFileDialog>
#include <QPushButton>

PImportDataButton::PImportDataButton(VAPoR::ControlExec* ce, MainForm *mf) : PWidget("", _hBox = new VHBoxWidget()), _ce(ce), _mf(mf) {
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(_hBox->layout());

    QPushButton *button = new QPushButton("Select File(s)", this);
    connect(button, &QPushButton::clicked, this, &PImportDataButton::_importDataset);
    layout->addWidget(button, 1);
    layout->addWidget(_fileLabel = new VLabel(""),3);

    _fileLabel->MakeSelectable();
}

void PImportDataButton::_importDataset() {
    GUIStateParams* gsp = dynamic_cast<GUIStateParams *>(getParams());
    std::vector<std::string> dataSetNames = gsp->GetOpenDataSetNames();
    std::string defaultPath = dataSetNames.size() ? gsp->GetOpenDataSetPaths(dataSetNames.back())[0] : FileUtils::HomeDir();
    
    QStringList qfileNames = QFileDialog::getOpenFileNames(this, "Select Filename Prefix", QString::fromStdString(defaultPath));
    std::vector<std::string> fileNames;
    for (const QString &qStr : qfileNames) fileNames.push_back(qStr.toStdString());
    if (fileNames.empty()) return;

    std::string format = GetDatasets()[getParams()->GetValueLong(GUIStateParams::ImportDataTypeTag, 0)].first;
    int rc = _mf->ImportDataset(fileNames, format);

    // Needs to be done here (or somewhere adjacent) instead of in MainForm when refactoring complete
    if (rc == 0) {
    //    getParams()->SetValueString(GUIStateParams::ImportDataDirTag, "", FileUtils::Dirname(fileNames[0]));
    //    getParams()->SetValueStringVec(GUIStateParams::ImportDataFilesTag, "", fileNames);
    //    getParams()->SetValueLong(GUIStateParams::DataJustLoadedTag, "Data has just been loaded", 1);
    }
}

void PImportDataButton::updateGUI() const {
    GUIStateParams* gsp = dynamic_cast<GUIStateParams *>(getParams());

    std::vector<std::string> dataSetNames = gsp->GetOpenDataSetNames();
    if (!dataSetNames.size()) return;

    std::vector<std::string> paths = gsp->GetOpenDataSetPaths(dataSetNames.back());
    int count = paths.size();

    if (count > 0) {
        std::string dir = FileUtils::Basename(paths[0]);
        _fileLabel->SetText("Imported: " + std::to_string(count) + " file(s)\nFrom: " + dir);
    }
    else _fileLabel->SetText("");
}
