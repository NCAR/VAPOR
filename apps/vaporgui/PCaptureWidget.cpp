#include "CaptureUtils.h"
#include "CaptureController.h"
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
#include "vapor/NavigationUtils.h"

#include <QFileDialog>
#include <QHBoxLayout>

const std::string TiffStrings::FileFilter = "TIFF (*.tif)";
const std::string TiffStrings::FileSuffix = "tif";

const std::string PngStrings::FileFilter = "PNG (*.png)";
const std::string PngStrings::FileSuffix = "png";

PCaptureHBox::PCaptureHBox(VAPoR::ControlExec *ce, CaptureController *captureController)
    : PWidget("", _hBox = new VHBoxWidget()), _ce(ce), _captureController(captureController)
{
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    _typeCombo = new PEnumDropdownStandalone(AnimationParams::CaptureTypeTag, {"TIFF", "PNG"}, {0, 1});

    _fileLabel = new VLabel("");
    _fileLabel->MakeSelectable();

    //_captureButton = new PButton("Export", [this](VAPoR::ParamsBase*){CaptureUtils::CaptureSingleImage(_ce);});
    _captureButton = new PButton("Export", [this](VAPoR::ParamsBase*){_captureController->CaptureSingleImage();});
    _captureButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, AnimationParams::SingleImage);

    _captureTimeSeriesButton = new PButton("Export", [this](VAPoR::ParamsBase*){_captureTimeSeries();});
    _captureTimeSeriesButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, AnimationParams::TimeSeries);

    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(_hBox->layout());
    layout->addWidget(_typeCombo,1);
    layout->addWidget(_fileLabel,3);
    layout->addWidget(_captureButton,1);
    layout->addWidget(_captureTimeSeriesButton,1);
}

void PCaptureHBox::_dropdownIndexChanged(int index) {
    int value;
    if (_enumMap.empty()) value = index; 
    else value = _enumMap[index];
    setParamsLong(value);
}

void PCaptureHBox::updateGUI() const {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _typeCombo->Update(ap);

    // Format the label for the saved file
    std::string file, time;
    std::string dir = ap->GetValueString(AnimationParams::CaptureFileDirTag, "");
    if (ap->GetValueLong(AnimationParams::CaptureModeTag, AnimationParams::SingleImage) == AnimationParams::SingleImage) { // Capture signle frame
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

void PCaptureHBox::_captureTimeSeries() {
    // User may cancel to prevent file overwrite, so capture return code
    //bool rc = CaptureUtils::EnableAnimationCapture(_ce);
    bool rc = _captureController->EnableAnimationCapture();
    if (rc) {
        //AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    //    std::cout << "Setting to timestep " << ap->GetStartTimestep() << std::endl;
        //NavigationUtils::SetTimestep(_ce, ap->GetValueLong(AnimationParams::CaptureStartTag, ap->GetStartTimestep()));
        //ap->SetValueLong(AnimationParams::AnimationStartedTag, "Enable tag", true);
    }
}

PCaptureWidget::PCaptureWidget(VAPoR::ControlExec *ce, CaptureController *captureController)
    : PWidget("", _section = new PSection("Image(s)")), _ce(ce)
{
    _section->Add(new PRadioButtons(AnimationParams::CaptureModeTag, {"Current frame", "Time series range"}));

    PTimeRangeSelector *t = new PTimeRangeSelector(_ce);
    t->EnableBasedOnParam(AnimationParams::CaptureModeTag, 1);
    _section->Add(t);

    _section->Add(new PCaptureHBox(ce, captureController));
}

void PCaptureWidget::updateGUI() const {    
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _section->Update(ap);
}
