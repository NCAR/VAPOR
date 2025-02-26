#include "PCaptureWidget.h"
#include "MainForm.h"
#include "PRadioButtons.h"
#include "PButton.h"
#include "PTimeRangeSelector.h"
#include "VSection.h"
#include "PSection.h"
#include "QLabel.h"
#include "VComboBox.h"
#include "VLabel.h"
#include "VHBoxWidget.h"
#include "VPushButton.h"
#include "VLineItem.h"
#include "PStringDropdown.h"
//#include "PCaptureLabel.h"

#include "vapor/GUIStateParams.h"
#include "vapor/AnimationParams.h"
#include "vapor/ViewpointParams.h"
#include "vapor/ControlExecutive.h"
#include "vapor/NavigationUtils.h"

#include <QHBoxLayout>
#include <QLabel>

typedef VAPoR::ViewpointParams VP;

const std::string CaptureModes::CURRENT = "Current frame";
const std::string CaptureModes::RANGE   = "Timeseries range";
const std::string CaptureModes::ALL     = "All renderings";

const std::string CaptureFileTypes::TIFF = "TIFF";
const std::string CaptureFileTypes::PNG  = "PNG";

PCaptureLabel::PCaptureLabel() : PWidget("", _label = new QLabel("")) {
    _label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void PCaptureLabel::updateGUI() const {
    AnimationParams *ap = dynamic_cast<AnimationParams*>(getParams());
    std::string captureFileText = "Saved: " + ap->GetValueString(AnimationParams::CaptureFileNameTag,"");
    std::cout << "text " << captureFileText << std::endl;
    _label->setText(QString::fromStdString(captureFileText));
}

PCaptureToolbar::PCaptureToolbar(VAPoR::ControlExec *ce, MainForm *mf)
    : PWidget("", _hBox = new VHBoxWidget()), _ce(ce), _mf(mf)
{
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    _typeCombo = new VComboBox({CaptureFileTypes::TIFF, CaptureFileTypes::PNG});
    _fileLabel = new VLabel("");

    _captureButton = new PButton("Capture\nCurrent Frame", [this](VAPoR::ParamsBase*){_captureSingleImage();});
    // PWidget is setting _enableBasedOnParamsValue to 0, so we need to set it to a non-zero value.
    // The default value is not being set and should be garbage.  Why/how is it being initialized to 0?
    _captureButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, -1);
    _captureButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, CaptureModes::CURRENT);

    _captureTimeseriesButton = new PButton("Capture\nTimeseries", [this](VAPoR::ParamsBase*){_captureTimeseries();});
    // Need to set ShowBasedOnParam for a numeric value.  See previous comment.
    _captureTimeseriesButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, -1);
    _captureTimeseriesButton->ShowBasedOnParam(AnimationParams::CaptureModeTag, CaptureModes::RANGE);
    

    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(_hBox->layout());
    layout->addWidget(_typeCombo,1);
    layout->addWidget(_fileLabel,3);
    layout->addWidget(_captureButton,1);
    layout->addWidget(_captureTimeseriesButton,1);
}

void PCaptureToolbar::updateGUI() const {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _typeCombo->SetValue(ap->GetValueString(AnimationParams::CaptureFileTypeTag, CaptureFileTypes::TIFF));
    _fileLabel->SetText("Saved: " + ap->GetValueString(AnimationParams::CaptureFileNameTag, ""));
    _captureButton->Update(ap);
    _captureTimeseriesButton->Update(ap);
}

void PCaptureToolbar::_captureSingleImage() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    string fileType = ap->GetValueString(AnimationParams::CaptureFileTypeTag, "");
    if (fileType == CaptureFileTypes::PNG) _mf->captureSingleImage("PNG (*.png)", ".png");
    else _mf->captureSingleImage("TIFF (*.tif *.tiff)", ".tiff");
}

void PCaptureToolbar::_captureTimeseries() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    string fileType = ap->GetValueString(AnimationParams::CaptureFileTypeTag, "");


    std::cout << "fileType " << fileType << std::endl;
    if (fileType == CaptureFileTypes::TIFF) _mf->captureTiffSequence();
    else _mf->capturePngSequence();
    _mf->_animationController->AnimationPlayForward();
    // AnimationPlayForward returns before the animation is done playing,
    // so we cannot call endAnimCapture() here.
    //_mf->endAnimCapture();

    // Manually setting the timestep in this way does not capture the image, nor does it apply
    // the various changes to MainForm during AnimationPlayForward.
    //auto start = ap->GetValueLong(AnimationParams::_startTimestepTag, 0);
    //auto end = ap->GetValueLong(AnimationParams::_endTimestepTag, 1);
    //for (auto i=start; i<end; i++) _mf->_animationController->SetTimeStep(i);

    // The only other option I can think of to end the animation capture sequence is to
    // call MainForm::endAnimCapture() in MainForm::_setAnimationOnOff().  Otherwise the
    // user needs to manually click "End image capture" to end it.
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

    _section->Add(new PCaptureToolbar(ce, mf));
}

void PCaptureWidget::updateGUI() const {    
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _section->Update(ap);
}

// private
//_mf->stopAnimCapture(string vizName);
//_mf->endAnimCapture(string vizName);
//_mf->setAnimationOnOFF(bool onOff);

// slots
//_mf->captureTiffSequence();
//_mf->capturePngSequence();
//_mf->selectAnimCaptureOutput(string filter, string defaultSuffix);
//startAnimCapture(string baseFile, string defaultSuffix = "tiff")
//captureSingleImage(stirng filter, string defaultSuffix);
//_setAnimationOnOff // Enable/disable VCR controllers
