#include "DatasetTypeLookup.h"
#include "vapor/ParamsBase.h"

namespace {
    const std::vector<std::pair<std::string, std::string>> datasets =
    {
        { "wrf"   , "WRF-ARW" },
        { "cf"    , "NetCDF-CF" },
        { "bov"   , "Brick of Values (BOV)" },
        { "dcp"   , "Data Collection Particles (DCP)" },
        { "mpas"  , "MPAS" },
        { "ugrid" , "Unstructured Grid (UGRID)" },
        { "vdc"   , "VDC" }
    };
}

const std::vector<std::pair<std::string, std::string>>& GetDatasets() {
    return datasets;
}

std::vector<std::string> GetDatasetTypeDescriptions() {
    std::vector<std::string> descriptions;
    for (const auto& pair : datasets) descriptions.push_back(pair.second);
    return descriptions;
}

std::string DatasetTypeDescriptiveName(const std::string& type) {
    auto it = std::find_if(datasets.begin(), datasets.end(), [&type](const auto& pair) {
        return pair.first==type;
    });
    return (it != datasets.end()) ? it->second : "No description for given data type " + type ;
}

std::string DatasetTypeShortName(const std::string& descriptiveName) {
    auto it = std::find_if(datasets.begin(), datasets.end(), [&descriptiveName](const auto& pair) {
        return pair.second==descriptiveName;
    });
    return (it != datasets.end()) ? it->first : "No shortName for given description " + descriptiveName;
}

//void MainForm::selectAnimCatureOutput(string filter, string defaultSuffix)
//void MainForm::startAnimCapture(string baseFile, string defaultSuffix)
/*void MainForm::startAnimCapture(
    string baseFile, 
    string defaultSuffix, 
    ControlExec *ce, 
    GUIStateParams *guiparams,
    bool &animationCapture,
    std::string &capturingAnimationVizName,
    QAction *captureEndImageAction,
    QMenu *imageSequenceMenu,
    QMenu *singleImageMenu
) {
    QString   fileName = QString::fromStdString(baseFile);
    QFileInfo fileInfo = QFileInfo(fileName);

    QString suffix = fileInfo.suffix();
    if (suffix != "" || suffix != "jpg" || suffix != "jpeg" || suffix != "tif" || suffix != "tiff" || suffix != "png") {}
    if (suffix == "") {
        suffix = QString::fromStdString(defaultSuffix);
        fileName += QString::fromStdString(defaultSuffix);
    } else {
        fileName = fileInfo.absolutePath() + "/" + fileInfo.baseName();
    }

    QString fileBaseName = fileInfo.baseName();

    int posn;
    for (posn = fileBaseName.length() - 1; posn >= 0; posn--) {
        if (!fileBaseName.at(posn).isDigit()) break;
    }
    int startFileNum = 0;

    unsigned int lastDigitPos = posn + 1;
    if (lastDigitPos < fileBaseName.length()) {
        startFileNum = fileBaseName.right(fileBaseName.length() - lastDigitPos).toInt();
        fileBaseName.truncate(lastDigitPos);
    }

    QString filePath = fileInfo.absolutePath() + "/" + fileBaseName;
    // Insert up to 4 zeros
    QString zeroes;
    if (startFileNum == 0)
        zeroes = "000";
    else {
        switch ((int)log10((float)startFileNum)) {
        case (0): zeroes = "000"; break;
        case (1): zeroes = "00"; break;
        case (2): zeroes = "0"; break;
        default: zeroes = ""; break;
        }
    }
    filePath += zeroes;
    filePath += QString::number(startFileNum);
    filePath += ".";
    filePath += suffix;
    string fpath = filePath.toStdString();

    // Check if the numbered file exists.
    QFileInfo check_file(filePath);
    if (check_file.exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Are you sure?");
        QString msg = "The following numbered file exists.\n ";
        msg += filePath;
        msg += "\n";
        msg += "Do you want to continue? You can choose \"No\" to go back and change the file name.";
        msgBox.setText(msg);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) { return; }
    }

    // Turn on "image capture mode" in the current active visualizer
    _animationCapture = true;
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    _controlExec->EnableAnimationCapture(vizName, true, fpath);
    _capturingAnimationVizName = vizName;

    _captureEndImageAction->setEnabled(true);
    _imageSequenceMenu->setEnabled(false);
    _singleImageMenu->setEnabled(false);
}*/

//void MainForm::endAnimCapture()
