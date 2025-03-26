#include "MainForm.h"
#include "PCaptureWidget.h"
#include "PTimeRangeSelector.h"
#include "PRadioButtons.h"
#include "PButton.h"
#include "PSection.h"
#include "PEnumDropdown.h"
#include "VComboBox.h"
#include "VLabel.h"
#include "VHBoxWidget.h"

#include "vapor/AnimationParams.h"
#include "vapor/ControlExecutive.h"
#include "vapor/STLUtils.h"
#include "vapor/FileUtils.h"

#include <QFileDialog>
#include <QHBoxLayout>

const std::string TiffStrings::FileFilter = "TIFF (*.tif)";
const std::string TiffStrings::FileSuffix = "tif";

const std::string PngStrings::FileFilter = "PNG (*.png)";
const std::string PngStrings::FileSuffix = "png";

PCaptureHBox::PCaptureHBox(VAPoR::ControlExec *ce, MainForm *mf)
    : PWidget("", _hBox = new VHBoxWidget()), _ce(ce), _mf(mf)
{
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    _typeCombo = new PEnumDropdownStandalone(AnimationParams::CaptureTypeTag, {"TIFF", "PNG"}, {0, 1});

    _fileLabel = new VLabel("");
    _fileLabel->MakeSelectable();

    _captureButton = new PButton("Capture\nCurrent Frame", [this](VAPoR::ParamsBase*){_captureSingleImage();});
    _captureButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, 0);

    _captureTimeSeriesButton = new PButton("Capture\nTime Series", [this](VAPoR::ParamsBase*){_captureTimeSeries();});
    _captureTimeSeriesButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, 1);

    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(_hBox->layout());
    layout->addWidget(_typeCombo,1);
    layout->addWidget(_fileLabel,3);
    layout->addWidget(_captureButton,1);
    layout->addWidget(_captureTimeSeriesButton,1);
}

void PCaptureHBox::_dropdownIndexChanged(int index) {
    int value;
    if (_enumMap.empty()) {
        value = index;
    } else {
        VAssert(index >= 0 && index < _enumMap.size());
        value = _enumMap[index];
    }
    setParamsLong(value);
}

void PCaptureHBox::updateGUI() const {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _typeCombo->Update(ap);

    // Format the label for the saved file
    std::string file, time;
    std::string dir = ap->GetValueString(AnimationParams::CaptureFileDirTag, "");
    if (ap->GetValueLong(AnimationParams::CaptureModeTag, 0) == 0) { // Capture signle frame
        file = ap->GetValueString(AnimationParams::CaptureFileNameTag, "");
        time = ap->GetValueString(AnimationParams::CaptureFileTimeTag, "");
    }
    else { // Capture timeseries
        file = ap->GetValueString(AnimationParams::CaptureTimeSeriesFileNameTag, "");
        if (file != "") {
            file = FileUtils::Basename(ap->GetValueString(AnimationParams::CaptureTimeSeriesFileNameTag, ""));
            size_t lastDot = file.find_last_of(".");
            file = file.substr(0, lastDot) + "####" + file.substr(lastDot);
            time = ap->GetValueString(AnimationParams::CaptureTimeSeriesTimeTag, "");
        }
    }
    if (!file.empty()) file = "Saved: " + file + "\nDir: " + dir + "\nOn: " + time;
    _fileLabel->SetText(file);

    _captureButton->Update(ap);
    _captureTimeSeriesButton->Update(ap);
}

void PCaptureHBox::_captureSingleImage() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    std::string defaultPath = ap->GetValueString(AnimationParams::CaptureFileDirTag, FileUtils::HomeDir());

    if (ap->GetValueLong(AnimationParams::CaptureTypeTag, 0) == 0)
        _mf->CaptureSingleImage(TiffStrings::FileFilter, "."+TiffStrings::FileSuffix);
    else 
        _mf->CaptureSingleImage(PngStrings::FileFilter, "."+PngStrings::FileSuffix);

    ap->SetValueString(AnimationParams::CaptureFileTimeTag, "Capture file time", STLUtils::GetUserTime());
}

void PCaptureHBox::_captureTimeSeries() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
   
    std::string filter, defaultSuffix;
    if (ap->GetValueLong(AnimationParams::CaptureTypeTag, 0) == 0) { // .tiff
         filter = TiffStrings::FileFilter;
         defaultSuffix =  TiffStrings::FileSuffix;
    }
    else {                                                          // .png
         filter = PngStrings::FileFilter;
         defaultSuffix =  PngStrings::FileSuffix;
    }

    std::string defaultPath = ap->GetValueString(AnimationParams::CaptureFileDirTag, FileUtils::HomeDir());
    std::string fileName = QFileDialog::getSaveFileName(this, "Select Filename Prefix", QString::fromStdString(defaultPath), QString::fromStdString(filter)).toStdString();
    if (fileName.empty()) return;

    ap->SetValueString(AnimationParams::CaptureFileDirTag, "Capture animation file directory", FileUtils::Dirname(fileName));
    ap->SetValueString(AnimationParams::CaptureTimeSeriesFileNameTag, "Capture animation file name", FileUtils::Basename(fileName));
    ap->SetValueString(AnimationParams::CaptureTimeSeriesTimeTag, "Capture file time", STLUtils::GetUserTime());

    bool rc = _mf->StartAnimCapture(fileName, defaultSuffix);  // User may cancel to prevent file overwrite, so capture return code
    std::cout << "rc " << rc << std::endl;
    if (rc) {
        _mf->SetTimeStep(ap->GetStartTimestep());
        _mf->AnimationPlayForward();
    }
}

PCaptureWidget::PCaptureWidget(VAPoR::ControlExec *ce, MainForm *mf)
    : PWidget("", _section = new PSection("Image(s)")), _ce(ce), _mf(mf)
{
    _section->Add(new PRadioButtons(AnimationParams::CaptureModeTag, {"Current frame", "Time series range"}));

    PTimeRangeSelector *t = new PTimeRangeSelector(_ce);
    t->EnableBasedOnParam(AnimationParams::CaptureModeTag, 1);
    _section->Add(t);

    _section->Add(new PCaptureHBox(ce, mf));
}

void PCaptureWidget::updateGUI() const {    
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _section->Update(ap);
}
