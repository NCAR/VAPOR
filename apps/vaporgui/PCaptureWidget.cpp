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

//PCaptureLabel::PCaptureLabel() : PWidget("", _label = new VLabel("")) {}
PCaptureLabel::PCaptureLabel() : PWidget("", _label = new QLabel("")) {
    _label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void PCaptureLabel::updateGUI() const {
    AnimationParams *ap = dynamic_cast<AnimationParams*>(getParams());
    std::string capturePath = ap->GetValueString(AnimationParams::CaptureFilePathTag,"");
    std::string captureFileName = ap->GetValueString(AnimationParams::CaptureFileNameTag,"");
    std::string captureFileType = ap->GetValueString(AnimationParams::CaptureFileTypeTag,"");
    //_label->SetText(capturePath + " " + captureFileName + " " + captureFileType);
    _label->setText(QString::fromStdString(captureFileName));
    std::cout << capturePath + " : " + captureFileName + " : " + captureFileType << std::endl;
}

PCaptureToolbar::PCaptureToolbar(VAPoR::ControlExec *ce, MainForm *mf)
    : PWidget("", _hBox = new VHBoxWidget()), _ce(ce), _mf(mf)
{
    _hBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    //_pGroup = new PHGroup;
    //_hBox->layout()->addWidget(_pGroup);
    //_pGroup->Add(new PStringDropdown(AnimationParams::CaptureFileTypeTag, {CaptureFileTypes::TIFF, CaptureFileTypes::PNG}," "));
    //_pGroup->Add(new PCaptureLabel());
    //_pGroup->Add(new PButton("Capture", [this](VAPoR::ParamsBase*){_captureSingleImage();}));

    _typeCombo = new VComboBox({CaptureFileTypes::TIFF, CaptureFileTypes::PNG});
    VComboBox* typeCombo2 = new VComboBox({CaptureFileTypes::TIFF, CaptureFileTypes::PNG});
    _fileLabel = new VLabel("");
    //_fileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    //_fileLabel->setMaximumWidth(QWIDGETSIZE_MAX);
    _captureButton = new PButton("Capture", [this](VAPoR::ParamsBase*){_captureSingleImage();});

    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(_hBox->layout());
    layout->addWidget(_typeCombo,1);
    //layout->addItem((QLayoutItem*)new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->addWidget(_fileLabel,3);
    layout->addWidget(_captureButton,1);
}

void PCaptureToolbar::updateGUI() const {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    //_pGroup->Update(ap);

    std::string type = ap->GetValueString(AnimationParams::CaptureFileTypeTag, CaptureFileTypes::TIFF);
    _typeCombo->SetValue(type);
    std::string file = ap->GetValueString(AnimationParams::CaptureFileNameTag, "");
    _fileLabel->SetText(file);
    _captureButton->Update(ap);
}

void PCaptureToolbar::_captureSingleImage() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    string fileType = ap->GetValueString(AnimationParams::CaptureFileTypeTag, "");
    if (fileType == "PNG") _mf->captureSingleImage("PNG (*.png)", ".png");
    else _mf->captureSingleImage("TIFF (*.tif *.tiff)", ".tiff");
}

//PCaptureWidget::PCaptureWidget(VAPoR::ControlExec *ce)
PCaptureWidget::PCaptureWidget(VAPoR::ControlExec *ce, MainForm *mf)
    : PWidget("", _section = new PSection("Image(s)")), _ce(ce), _mf(mf)
{
    
    _section->Add(new PRadioButtons(AnimationParams::CaptureModeTag, {CaptureModes::CURRENT, CaptureModes::RANGE}));

    PTimeRangeSelector *t = new PTimeRangeSelector(_ce);
    // PWidget is setting _enableBasedOnParamsValue to 0, so we need to set it to a non-zero value.
    // The default value is not being set and should be garbage.  Why/how is it being initialized to 0?
    t->EnableBasedOnParam(AnimationParams::CaptureModeTag, -1);
    //t->EnableBasedOnParam(AnimationParams::CaptureModeTag, "Range");
    t->EnableBasedOnParam(AnimationParams::CaptureModeTag, CaptureModes::RANGE);
    _section->Add(t);

    
    //_section->Add(new PStringDropdown(AnimationParams::CaptureFileTypeTag, {CaptureFileTypes::TIFF, CaptureFileTypes::PNG}, "Capture file type"));

    //PButton *b = new PButton("Capture", [this](VAPoR::ParamsBase*){_captureSingleImage();});
    // DisableUndo causes segfault
    // b->DisableUndo();
    // Need to set ShowBasedOnParam for a numeric value.  See previous comment.
    //b->ShowBasedOnParam(AnimationParams::CaptureModeTag, -1);
    //b->ShowBasedOnParam(AnimationParams::CaptureModeTag, CaptureModes::CURRENT);
    //_section->Add(b);
    //_section->Add(new PLineItem("","",selectedFile,b));

    _section->Add(new PCaptureToolbar(ce, mf));

    //PLineItem *line = new PLineItem("","",new PStringInput(AnimationParams::b);
    //_section->layout()->addWidget(_currentFrameButton);
    //_section->layout()->addWidget(_rangeButton);
}

void PCaptureWidget::updateGUI() const {    
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _section->Update(ap);
}

void PCaptureWidget::_captureSingleImage() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    string fileType = ap->GetValueString(AnimationParams::CaptureFileTypeTag, "");
    if (fileType == "PNG") _mf->captureSingleImage("PNG (*.png)", ".png");
    else _mf->captureSingleImage("TIFF (*.tif *.tiff)", ".tiff");
}

void PCaptureWidget::_captureTimeseries() {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    string fileType = ap->GetValueString(AnimationParams::CaptureFileTypeTag, "");
    if (fileType == CaptureFileTypes::TIFF) _mf->captureTiffSequence();
    else _mf->capturePngSequence();
    _mf->_animationController->AnimationPlayForward();
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
