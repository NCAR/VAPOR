//************************************************************************
//							  *
//	   Copyright (C)  2017					*
//	 University Corporation for Atmospheric Research		  *
//	   All Rights Reserved					*
//							  *
//************************************************************************/
//
//  File:	   TFWidget.cpp
//
//  Author:	 Scott Pearse
//	  National Center for Atmospheric Research
//	  PO 3000, Boulder, Colorado
//
//  Date:	   March 2017
//
//  Description:	Implements the TFWidget class.  This provides
//	  a widget that is inserted in the "Appearance" tab of various Renderer GUIs
//
#include <GL/glew.h>
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include <QFontDatabase>
#include <QStringList>
#include <qradiobutton.h>
#include <qcolordialog.h>
#include "TwoDSubtabs.h"
#include "RenderEventRouter.h"
#include "vapor/RenderParams.h"
#include "vapor/TwoDDataParams.h"
#include "TFWidget.h"
#include "ErrorReporter.h"

using namespace VAPoR;

string TFWidget::_nDimsTag = "ActiveDimension";

TFWidget::TFWidget(QWidget *parent) : QWidget(parent), Ui_TFWidgetGUI()
{
    setupUi(this);

    _somethingChanged = false;
    _autoUpdateHisto = false;
    _discreteColormap = false;

    _myRGB[0] = _myRGB[1] = _myRGB[2] = 1.f;

    _minCombo = new Combo(_minRangeEdit, _minRangeSlider);
    _maxCombo = new Combo(_maxRangeEdit, _maxRangeSlider);
    _rangeCombo = new RangeCombo(_minCombo, _maxCombo);

    _colorMapMinSliderEdit->SetLabel(QString::fromAscii("Min"));
    _colorMapMinSliderEdit->SetIntType(false);
    _colorMapMinSliderEdit->SetExtents(0.f, 1.f);

    _colorMapMaxSliderEdit->SetLabel(QString::fromAscii("Max"));
    _colorMapMaxSliderEdit->SetIntType(false);
    _colorMapMaxSliderEdit->SetExtents(0.f, 1.f);

    _cLevel = 0;
    _refLevel = 0;
    _timeStep = 0;
    for (int i = 0; i < 3; i++) {
        _minExt.push_back(0.f);
        _maxExt.push_back(0.f);
    }

    connectWidgets();
}

void TFWidget::collapseConstColorWidgets()
{
    useConstColorFrame->hide();
    constColorFrame->hide();
    adjustSize();
}

void TFWidget::showConstColorWidgets()
{
    useConstColorFrame->show();
    constColorFrame->show();
    adjustSize();
}

void TFWidget::hideWhitespaceFrame()
{
    whitespaceFrame->hide();
    adjustSize();
}

void TFWidget::showWhitespaceFrame()
{
    whitespaceFrame->show();
    adjustSize();
}

void TFWidget::Reinit(TFFlags flags)
{
    _flags = flags;
    if (_flags & CONSTANT)
        showConstColorWidgets();
    else
        collapseConstColorWidgets();

    if (_flags & COLORVAR_IS_IN_TF2) {
        if (_tabWidget->count() < 2) _tabWidget->insertTab(1, _colormapTFE, "Color Mapped VARIABLE");
    } else
        _tabWidget->removeTab(1);

    adjustSize();
}

TFWidget::~TFWidget()
{
    if (_minCombo) {
        delete _minCombo;
        _minCombo = NULL;
    }
    if (_maxCombo) {
        delete _maxCombo;
        _maxCombo = NULL;
    }
    if (_rangeCombo) {
        delete _rangeCombo;
        _rangeCombo = NULL;
    }
}

void TFWidget::enableTFWidget(bool state)
{
    _loadButton->setEnabled(state);
    _saveButton->setEnabled(state);
    _tfFrame->setEnabled(state);
    _minRangeEdit->setEnabled(state);
    _maxRangeEdit->setEnabled(state);
    _opacitySlider->setEnabled(state);
    _autoUpdateHistoCheckbox->setEnabled(state);
    _colorInterpCombo->setEnabled(state);
}

void TFWidget::loadTF()
{
    string varname = getCurrentVarName();
    if (varname.empty()) return;

    // Ignore TF's in session, for now.

    SettingsParams *sP;
    sP = (SettingsParams *)_paramsMgr->GetParams(SettingsParams::GetClassType());
    string path = sP->GetTFDir();

    fileLoadTF(varname, path.c_str(), true);

    Update(_dataMgr, _paramsMgr, _rParams);
}

void TFWidget::loadTF(string varname)
{
    SettingsParams *sP;
    sP = (SettingsParams *)_paramsMgr->GetParams(SettingsParams::GetClassType());

    string path = sP->GetTFDir();

    fileLoadTF(varname, path.c_str(), true);
}

void TFWidget::fileLoadTF(string varname, const char *startPath, bool savePath)
{
    QString s = QFileDialog::getOpenFileName(0, "Choose a transfer function file to open", startPath, "Vapor 3 Transfer Functions (*.tf3)");

    // Null string indicates nothing selected
    if (s.length() == 0)
        return;
    else {
        SettingsParams *sP;
        sP = (SettingsParams *)_paramsMgr->GetParams(SettingsParams::GetClassType());
        sP->SetTFDir(s.toStdString());
    }

    // Force name to end with .tf3
    if (!s.endsWith(".tf3")) { s += ".tf3"; }

    MapperFunction *tf = _rParams->GetMapperFunc(varname);
    assert(tf);

    vector<double> defaultRange;
    _dataMgr->GetDataRange(0, varname, 0, 0, defaultRange);

    int rc = tf->LoadFromFile(s.toStdString(), defaultRange);
    if (rc < 0) { MSG_ERR("Error loading transfer function"); }
}

void TFWidget::fileSaveTF()
{
    // Launch a file save dialog, open resulting file
    GUIStateParams *p;
    p = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());
    string path = p->GetCurrentTFPath();

    QString s = QFileDialog::getSaveFileName(0, "Choose a filename to save the transfer function", path.c_str(), "Vapor 3 Transfer Functions (*.tf3)");
    // Did the user cancel?
    if (s.length() == 0) return;
    // Force the name to end with .tf3
    if (!s.endsWith(".tf3")) { s += ".tf3"; }

    string varname = _rParams->GetVariableName();
    if (varname.empty()) return;

    MapperFunction *tf = _rParams->GetMapperFunc(varname);
    assert(tf);

    int rc = tf->SaveToFile(s.toStdString());
    if (rc < 0) {
        MSG_ERR("Failed to write output file");
        return;
    }
}

void TFWidget::getRange(float range[2], float values[2])
{
    range[0] = range[1] = 0.0;
    values[0] = values[1] = 0.0;
    string varName = getCurrentVarName();
    if (varName.empty() || varName == "Constant") return;

    size_t ts = _rParams->GetCurrentTimestep();
    int    ref = _rParams->GetRefinementLevel();
    int    cmp = _rParams->GetCompressionLevel();

    if (!_dataMgr->VariableExists(ts, varName, ref, cmp)) return;

    vector<double> rangev;
    int            rc = _dataMgr->GetDataRange(ts, varName, ref, cmp, rangev);
    if (rc < 0) {
        MSG_ERR("Error loading variable");
        return;
    }
    assert(rangev.size() == 2);

    range[0] = rangev[0];
    range[1] = rangev[1];

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    values[0] = tf->getMinMapValue();
    values[1] = tf->getMaxMapValue();
}

float TFWidget::getOpacity()
{
    string          varName = _rParams->GetVariableName();
    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    assert(tf);

    return tf->getOpacityScale();
}

void TFWidget::updateColorInterpolation()
{
    MapperFunction *tf = getMainMapperFunction();

    TFInterpolator::type t = tf->getColorInterpType();
    _colorInterpCombo->blockSignals(true);
    _colorMapColorInterpCombo->blockSignals(true);
    if (t == TFInterpolator::diverging) {
        _colorInterpCombo->setCurrentIndex(0);
        _colorMapColorInterpCombo->setCurrentIndex(0);
        showWhitespaceFrame();
    } else if (t == TFInterpolator::discrete) {
        _colorInterpCombo->setCurrentIndex(1);
        _colorMapColorInterpCombo->setCurrentIndex(1);
        hideWhitespaceFrame();
    } else {
        _colorInterpCombo->setCurrentIndex(2);
        _colorMapColorInterpCombo->setCurrentIndex(2);
        hideWhitespaceFrame();
    }
    _colorInterpCombo->blockSignals(false);
    _colorMapColorInterpCombo->blockSignals(false);

    int useWhitespace = tf->getUseWhitespace();
    if (useWhitespace) {
        _whitespaceCheckbox->setCheckState(Qt::Checked);
        _colorMapWhitespaceCheckbox->setCheckState(Qt::Checked);
    } else {
        _whitespaceCheckbox->setCheckState(Qt::Unchecked);
        _colorMapWhitespaceCheckbox->setCheckState(Qt::Unchecked);
    }
}

void TFWidget::updateAutoUpdateHistoCheckbox()
{
    MapperFunction *tf = getMainMapperFunction();

    // Update the state of autoUpdateHisto according to params
    //
    _autoUpdateHistoCheckbox->blockSignals(true);
    _colorMapAutoUpdateHistoCheckbox->blockSignals(true);
    if (tf->GetAutoUpdateHisto()) {
        _autoUpdateHistoCheckbox->setCheckState(Qt::Checked);
        _colorMapAutoUpdateHistoCheckbox->setCheckState(Qt::Checked);
    } else {
        _autoUpdateHistoCheckbox->setCheckState(Qt::Unchecked);
        _colorMapAutoUpdateHistoCheckbox->setCheckState(Qt::Unchecked);
    }
    _autoUpdateHistoCheckbox->blockSignals(false);
    _colorMapAutoUpdateHistoCheckbox->blockSignals(false);
}

void TFWidget::updateSliders()
{
    // Update min/max transfer function sliders/lineEdits
    //
    float range[2], values[2];
    getRange(range, values);

    _rangeCombo->Update(range[0], range[1], values[0], values[1]);
    _opacitySlider->setValue(getOpacity() * 100);

    _minLabel->setText(QString::number(range[0]));
    _maxLabel->setText(QString::number(range[1]));
}

void TFWidget::updateMappingFrames()
{
    _mappingFrame->Update(_dataMgr, _paramsMgr, _rParams);
    _mappingFrame->fitToView();

    if (_flags & COLORVAR_IS_IN_TF2) {
        _colorMapMappingFrame->Update(_dataMgr, _paramsMgr, _rParams);
        _colorMapMappingFrame->fitToView();
    }
}

void TFWidget::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *rParams)
{
    assert(paramsMgr);
    assert(dataMgr);
    assert(rParams);

    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;
    _rParams = rParams;

    if (getCurrentVarName() == "") {
        setEnabled(false);
        return;
    } else {
        setEnabled(true);
    }

    updateAutoUpdateHistoCheckbox();
    updateMappingFrames();
    updateColorInterpolation();
    updateSliders();
    updateConstColorWidgets();

    string newName = getCurrentVarName();
    if (_varName != newName) {
        _varName = newName;
        refreshHistograms();
    }

    checkForExternalChangesToHisto();
}

void TFWidget::checkForCompressionChanges()
{
    int newCLevel = _rParams->GetCompressionLevel();
    if (_cLevel != newCLevel) {
        _cLevel = _rParams->GetCompressionLevel();
        _somethingChanged = true;
    }

    int newRefLevel = _rParams->GetRefinementLevel();
    if (_refLevel != newRefLevel) {
        _refLevel = newRefLevel;
        _somethingChanged = true;
    }
}

void TFWidget::checkForBoxChanges()
{
    std::vector<double> minExt, maxExt;
    VAPoR::Box *        box = _rParams->GetBox();
    box->GetExtents(minExt, maxExt);
    for (int i = 0; i < minExt.size(); i++) {
        if (minExt[i] != _minExt[i]) {
            _somethingChanged = true;
            _minExt[i] = minExt[i];
        }
        if (maxExt[i] != _maxExt[i]) {
            _somethingChanged = true;
            _maxExt[i] = maxExt[i];
        }
    }
}

void TFWidget::checkForMapperRangeChanges()
{
    double          min = _minCombo->GetValue();
    double          max = _maxCombo->GetValue();
    MapperFunction *tf = getMainMapperFunction();
    double          newMin = tf->getMinMapValue();
    double          newMax = tf->getMaxMapValue();
    if (min != newMin) _somethingChanged = true;
    if (max != newMax) _somethingChanged = true;
}

void TFWidget::checkForTimestepChanges()
{
    int newTimestep = _rParams->GetCurrentTimestep();
    if (_timeStep != newTimestep) {
        _timeStep = newTimestep;
        _somethingChanged = true;
    }
}

void TFWidget::checkForExternalChangesToHisto()
{
    checkForCompressionChanges();
    checkForBoxChanges();
    checkForMapperRangeChanges();
    checkForTimestepChanges();

    if (_somethingChanged) {
        if (autoUpdateHisto())
            refreshHistograms();
        else
            _updateHistoButton->setEnabled(true);
    }
    _somethingChanged = false;
}

void TFWidget::updateConstColorWidgets()
{
    float rgb[3];
    _rParams->GetConstantColor(rgb);
    QColor   color(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255);
    QPalette palette(_colorDisplay->palette());
    palette.setColor(QPalette::Base, color);
    _colorDisplay->setPalette(palette);

    _useConstColorCheckbox->blockSignals(true);
    bool useSingleColor = _rParams->UseSingleColor();
    if (useSingleColor)
        _useConstColorCheckbox->setCheckState(Qt::Checked);
    else
        _useConstColorCheckbox->setCheckState(Qt::Unchecked);
    _useConstColorCheckbox->blockSignals(false);

    string varName;
    if (_flags & COLORVAR_IS_IN_TF1) {
        varName = _rParams->GetColorMapVariableName();
        // If we are using a single color instead of a
        // color mapped variable, disable the transfer function
        //
        if (varName == "") {
            enableTFWidget(false);
        }

        // Otherwise enable it and continue on to updating the
        // min/max sliders in the transfer function
        //
        else {
            enableTFWidget(true);
        }
    }
}

void TFWidget::connectWidgets()
{
    connect(_rangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_updateHistoButton, SIGNAL(pressed()), this, SLOT(updateHisto()));
    connect(_autoUpdateHistoCheckbox, SIGNAL(stateChanged(int)), this, SLOT(autoUpdateHistoChecked(int)));
    connect(_colorInterpCombo, SIGNAL(activated(int)), this, SLOT(colorInterpChanged(int)));
    connect(_whitespaceCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setUseWhitespace(int)));
    connect(_loadButton, SIGNAL(pressed()), this, SLOT(loadTF()));
    connect(_saveButton, SIGNAL(pressed()), this, SLOT(fileSaveTF()));
    connect(_mappingFrame, SIGNAL(updateParams()), this, SLOT(setRange()));
    connect(_mappingFrame, SIGNAL(endChange()), this, SLOT(emitTFChange()));
    connect(_opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));
    connect(_colorSelectButton, SIGNAL(pressed()), this, SLOT(setSingleColor()));
    connect(_useConstColorCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setUsingSingleColor(int)));

    // Connections for our ColorMapVariable transfer function
    //
    connect(_colorMapOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));
    connect(_colorMapUpdateHistoButton, SIGNAL(pressed()), this, SLOT(updateHisto()));
    connect(_colorMapAutoUpdateHistoCheckbox, SIGNAL(stateChanged(int)), this, SLOT(autoUpdateHistoChecked(int)));
    connect(_colorMapColorInterpCombo, SIGNAL(activated(int)), this, SLOT(colorInterpChanged(int)));
    connect(_colorMapWhitespaceCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setUseWhitespace(int)));
    connect(_colorMapMinSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(setColorMapMinRange(double)));
    connect(_colorMapMaxSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(setColorMapMaxRange(double)));
}

void TFWidget::emitTFChange() { emit emitChange(); }

void TFWidget::opacitySliderChanged(int value)
{
    string          varName = _rParams->GetVariableName();
    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    assert(tf);
    tf->setOpacityScale(value / 100.f);
    emit emitChange();
}

void TFWidget::setRange()
{
    float min = _mappingFrame->getMinEditBound();
    float max = _mappingFrame->getMaxEditBound();
    setRange(min, max);
    emit emitChange();
}

void TFWidget::setRange(double min, double max)
{
    _somethingChanged = true;

    MapperFunction *tf = getMainMapperFunction();

    tf->setMinMapValue(min);
    tf->setMaxMapValue(max);

    updateHisto();
    emit emitChange();
}

void TFWidget::refreshHistograms()
{
    bool force = true;
    // MapperFunction *mf = getMainMapperFunction();
    //	_mappingFrame->updateMapperFunction(mf);
    _mappingFrame->RefreshHistogram(force);

    if (_flags & COLORVAR_IS_IN_TF2) {
        // mf = getColorMapperFunction();
        //		_colorMapMappingFrame->updateMapperFunction(mf);
        _colorMapMappingFrame->RefreshHistogram(force);
    }

    updateMappingFrames();
    _updateHistoButton->setEnabled(false);
}

void TFWidget::updateHisto()
{
    bool buttonRequest = sender() == _updateHistoButton ? true : false;
    if (autoUpdateHisto() || buttonRequest) {
        refreshHistograms();
    } else {
        _mappingFrame->fitToView();
    }
}

void TFWidget::autoUpdateHistoChecked(int state)
{
    bool bstate;
    if (state == 0)
        bstate = false;
    else
        bstate = true;

    MapperFunction *tf = getMainMapperFunction();
    tf->SetAutoUpdateHisto(bstate);

    updateHisto();
}

void TFWidget::setSingleColor()
{
    QPalette palette(_colorDisplay->palette());
    QColor   color = QColorDialog::getColor(palette.color(QPalette::Base), this);
    if (!color.isValid()) return;

    palette.setColor(QPalette::Base, color);
    _colorDisplay->setPalette(palette);

    qreal rgb[3];
    color.getRgbF(&rgb[0], &rgb[1], &rgb[2]);
    float myRGB[3];
    myRGB[0] = rgb[0];
    myRGB[1] = rgb[1];
    myRGB[2] = rgb[2];

    _rParams->SetConstantColor(myRGB);
}

void TFWidget::setUsingSingleColor(int state)
{
    if (state > 0) {
        _rParams->SetUseSingleColor(true);
    } else {
        _rParams->SetUseSingleColor(false);
    }
}

void TFWidget::colorInterpChanged(int index)
{
    MapperFunction *tf = getMainMapperFunction();

    if (index == 0) {
        tf->setColorInterpType(TFInterpolator::diverging);
    } else if (index == 1) {
        tf->setColorInterpType(TFInterpolator::discrete);
    } else if (index == 2) {
        tf->setColorInterpType(TFInterpolator::linear);
    }
}

void TFWidget::setUseWhitespace(int state)
{
    MapperFunction *tf = getMainMapperFunction();
    tf->setUseWhitespace(state);
}

void TFWidget::setColorMapMinRange(double min)
{
    MapperFunction *mf = getColorMapperFunction();
    mf->setMaxMapValue(min);
}

void TFWidget::setColorMapMaxRange(double max)
{
    MapperFunction *mf = getColorMapperFunction();
    mf->setMaxMapValue(max);
}

MapperFunction *TFWidget::getColorMapperFunction()
{
    string          varname = _rParams->GetColorMapVariableName();
    MapperFunction *tf = _rParams->GetMapperFunc(varname);
    assert(tf);
    return tf;
}

bool TFWidget::autoUpdateHisto()
{
    MapperFunction *tf = getMainMapperFunction();
    if (tf->GetAutoUpdateHisto())
        return true;
    else
        return false;
}

string TFWidget::getCurrentVarName()
{
    string varname = "";
    if (_flags & COLORVAR_IS_IN_TF1) {
        varname = _rParams->GetColorMapVariableName();
    } else {
        varname = _rParams->GetVariableName();
    }
    return varname;
}

MapperFunction *TFWidget::getMainMapperFunction()
{
    string          varname = getCurrentVarName();
    MapperFunction *tf = _rParams->GetMapperFunc(varname);
    assert(tf);
    return tf;
}
