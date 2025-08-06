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

#include <regex>
#include <QFileDialog>
#include <QHBoxLayout>

PCaptureHBox::PCaptureHBox(VAPoR::ControlExec *ce, CaptureController *captureController)
    : PWidget("", _hBox = new VHBoxWidget()), _ce(ce), _captureController(captureController)
{
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    _typeCombo = new PEnumDropdownStandalone(AnimationParams::CaptureTypeTag, {"TIFF", "PNG"}, {0, 1});

    _fileLabel = new VLabel("");
    _fileLabel->MakeSelectable();

    _captureButton = new PButton("Export", [this](VAPoR::ParamsBase*){_captureController->CaptureSingleImage();});
    _captureButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, AnimationParams::SingleImage);

    _captureTimeSeriesButton = new PButton("Export", [this](VAPoR::ParamsBase*){_captureController->EnableAnimationCapture();});
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
            size_t pos = file.rfind("0000");
            if (pos != std::string::npos) file.replace(pos, 4, "####"); // Replace last instance of 0000 with #### for file label
            time = ap->GetValueString(AnimationParams::CaptureTimeSeriesTimeTag, "");
        }
    }
    if (!file.empty()) file = "Saved: " + file + "\nDir: " + dir + "\nOn: " + time;
    _fileLabel->SetText(file);

    _captureButton->Update(ap);
    _captureTimeSeriesButton->Update(ap);
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
