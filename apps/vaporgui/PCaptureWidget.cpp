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

const std::string CaptureModes::CURRENT = "Current frame";
const std::string CaptureModes::RANGE   = "Time series range";

const std::string TiffStrings::CaptureFileType = "TIFF";
const std::string TiffStrings::FileFilter = "TIFF (*.tif)";
const std::string TiffStrings::FileSuffix = "tif";

const std::string PngStrings::CaptureFileType = "PNG";
const std::string PngStrings::FileFilter = "PNG (*.png)";
const std::string PngStrings::FileSuffix = "png";

PCaptureHBox::PCaptureHBox(VAPoR::ControlExec *ce, MainForm *mf)
    : PWidget("", _hBox = new VHBoxWidget()), _ce(ce), _mf(mf)
{
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    _typeCombo = new VComboBox({TiffStrings::CaptureFileType, PngStrings::CaptureFileType});
    connect(_typeCombo, &VComboBox::IndexChanged, this, &PCaptureHBox::_dropdownIndexChanged);

    _fileLabel = new VLabel("");
    _fileLabel->MakeSelectable();

    _captureButton = new PButton("Capture\nCurrent Frame", [this](VAPoR::ParamsBase*){_captureSingleImage();});
    // PWidget is setting _enableBasedOnParamsValue to 0, so we need to set it to a non-zero value.
    // The default value is not being set and should be garbage.  Why/how is it being initialized to 0?
    _captureButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, -1);
    _captureButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, CaptureModes::CURRENT);

    _captureTimeSeriesButton = new PButton("Capture\nTime Series", [this](VAPoR::ParamsBase*){_captureTimeSeries();});
    // Need to set ShowBasedOnParam for a numeric value.  See previous comment.
    _captureTimeSeriesButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, -1);
    _captureTimeSeriesButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, CaptureModes::RANGE);
    

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
    _typeCombo->SetValue(ap->GetValueString(AnimationParams::CaptureTypeTag, TiffStrings::CaptureFileType));

    // Format the label for the saved file
    std::string dir, file, time;
    if (ap->GetValueString(AnimationParams::CaptureModeTag, "") == CaptureModes::CURRENT) {
        dir = ap->GetValueString(AnimationParams::CaptureFileDirTag, "");
        file = ap->GetValueString(AnimationParams::CaptureFileNameTag, "");
        time = ap->GetValueString(AnimationParams::CaptureFileTimeTag, "");
    }
    else {
        dir = ap->GetValueString(AnimationParams::CaptureTimeSeriesFileDirTag, "");
        file = ap->GetValueString(AnimationParams::CaptureTimeSeriesFileNameTag, "");
        size_t lastSlash = file.find_last_of("/\\");
        size_t lastDot = file.find_last_of(".");

        // Indicates that multiple animation files are or have been captured
        bool valid = lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash);
        if (valid) file = file.substr(0, lastDot) + "_####" + file.substr(lastDot);

        time = ap->GetValueString(AnimationParams::CaptureTimeSeriesTimeTag, "");
    }
    if (!file.empty()) file = "Saved: " + file + "\nDir: " + dir + "\nOn: " + time;
    _fileLabel->SetText(file);

    _captureButton->Update(ap);
    _captureTimeSeriesButton->Update(ap);
}

void PCaptureHBox::_captureSingleImage() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    string fileType = ap->GetValueString(AnimationParams::CaptureTypeTag, "");
    if (fileType == PngStrings::FileFilter) _mf->CaptureSingleImage(PngStrings::FileFilter, ".png");
    else _mf->CaptureSingleImage(TiffStrings::FileFilter, ".tiff");

    ap->SetValueString(AnimationParams::CaptureFileTimeTag, "Capture file time", STLUtils::GetUserTime());
}

void PCaptureHBox::_captureTimeSeries() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
   
    std::string filter, defaultSuffix;
    if (ap->GetValueString(AnimationParams::CaptureTypeTag, "") == TiffStrings::CaptureFileType) {
         filter = TiffStrings::FileFilter;
         defaultSuffix =  TiffStrings::FileSuffix;
    }
    else {
         filter = PngStrings::FileFilter;
         defaultSuffix =  PngStrings::FileSuffix;
    }
    std::cout << filter << " " << defaultSuffix << " " << ap->GetValueString(AnimationParams::CaptureTypeTag, "") << std::endl;
    std::string defaultPath = ap->GetValueString(AnimationParams::CaptureTimeSeriesFileDirTag, FileUtils::HomeDir());
    std::string fileName = QFileDialog::getSaveFileName(this, "Select Filename Prefix", QString::fromStdString(defaultPath), QString::fromStdString(filter)).toStdString();
    if (fileName.empty()) return;

    ap->SetValueString(AnimationParams::CaptureTimeSeriesFileDirTag, "Capture animation file directory", FileUtils::Dirname(fileName));
    ap->SetValueString(AnimationParams::CaptureTimeSeriesFileNameTag, "Capture animation file name", fileName);
    ap->SetValueString(AnimationParams::CaptureTimeSeriesTimeTag, "Capture file time", STLUtils::GetUserTime());

    _mf->SetTimeStep(ap->GetStartTimestep());
    bool rc = _mf->StartAnimCapture(fileName, defaultSuffix);  // User may cancel to prevent file overwrite, so capture return code
    if (rc) _mf->AnimationPlayForward();
}

PCaptureWidget::PCaptureWidget(VAPoR::ControlExec *ce, MainForm *mf)
    : PWidget("", _section = new PSection("Image(s)")), _ce(ce), _mf(mf)
{
    
    _section->Add(new PRadioButtons(AnimationParams::CaptureModeTag, {CaptureModes::CURRENT, CaptureModes::RANGE}));

    PTimeRangeSelector *t = new PTimeRangeSelector(_ce);
    // Need to set ShowBasedOnParam for a numeric value.  See previous comment.
    t->EnableBasedOnParam(AnimationParams::CaptureModeTag, -1);
    t->EnableBasedOnParam(AnimationParams::CaptureModeTag, CaptureModes::RANGE);
    _section->Add(t);

    _section->Add(new PCaptureHBox(ce, mf));
}

void PCaptureWidget::updateGUI() const {    
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _section->Update(ap);
}
