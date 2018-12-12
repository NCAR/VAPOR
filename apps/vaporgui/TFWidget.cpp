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

    _initialized = false;
    _externalChangeHappened = false;
    _mainHistoRangeChanged = false;
    _secondaryHistoRangeChanged = false;
    _mainHistoNeedsRefresh = false;
    _secondaryHistoNeedsRefresh = false;
    _discreteColormap = false;
    _mainVarName = "";
    _secondaryVarName = "";

    _myRGB[0] = _myRGB[1] = _myRGB[2] = 1.f;

    _minCombo = new Combo(_minRangeEdit, _minRangeSlider);
    _maxCombo = new Combo(_maxRangeEdit, _maxRangeSlider);
    _rangeCombo = new RangeCombo(_minCombo, _maxCombo);

    _secondaryMinSliderEdit->SetLabel(QString::fromAscii("Min"));
    _secondaryMinSliderEdit->SetIntType(false);
    _secondaryMinSliderEdit->SetExtents(0.f, 1.f);

    _secondaryMaxSliderEdit->SetLabel(QString::fromAscii("Max"));
    _secondaryMaxSliderEdit->SetIntType(false);
    _secondaryMaxSliderEdit->SetExtents(0.f, 1.f);

    _cLevel = 0;
    _refLevel = 0;
    _timeStep = 0;
    for (int i = 0; i < 3; i++) {
        _minExt.push_back(0.f);
        _maxExt.push_back(0.f);
    }

    _varRange.resize(2);
    std::fill(_varRange.begin(), _varRange.end(), 0.f);

    connectWidgets();
}

void TFWidget::configureConstantColorControls()
{
    if (_flags & CONSTANT_COLOR) {
        _useConstColorFrame->show();
        _constColorFrame->show();
    } else {
        _useConstColorFrame->hide();
        _constColorFrame->hide();
    }

    adjustSize();
}

void TFWidget::configureSecondaryTransferFunction()
{
    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
        if (_tabWidget->count() < 2) _tabWidget->insertTab(1, _secondaryTFE, "Color Mapped VARIABLE");

        _mappingFrame->setColorMapping(false);
        _whitespaceFrame->hide();
        _colorInterpolationFrame->hide();
        _loadSaveFrame->hide();
    } else {
        _tabWidget->removeTab(1);
        _whitespaceFrame->show();
        _colorInterpolationFrame->show();
        _useConstColorFrame->show();
        _constColorFrame->show();
        _loadSaveFrame->show();
    }
}

void TFWidget::Reinit(TFFlags flags)
{
    _flags = flags;

    if (_flags & ISOLINES)
        _mappingFrame->setIsolineSliders(true);
    else
        _mappingFrame->setIsolineSliders(false);

    if (_flags & SAMPLING) _mappingFrame->SetIsSampling(true);

    configureSecondaryTransferFunction();
    configureConstantColorControls();

    _tabWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

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

void TFWidget::loadTF()
{
    string varname = _rParams->GetColorMapVariableName();
    if (varname.empty()) return;

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

    Update(_dataMgr, _paramsMgr, _rParams);
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

    bool mainTF = true;
    if (_flags & COLORMAP_VAR_IS_IN_TF2) mainTF = false;
    string varname = getTFVariableName(mainTF);
    if (varname.empty()) return;

    MapperFunction *tf = _rParams->GetMapperFunc(varname);
    assert(tf);

    int rc = tf->SaveToFile(s.toStdString());
    if (rc < 0) {
        MSG_ERR("Failed to write output file");
        return;
    }
}

void TFWidget::getVariableRange(float range[2], float values[2], bool secondaryVariable = false)
{
    range[0] = range[1] = 0.0;
    values[0] = values[1] = 0.0;
    string varName;
    if (secondaryVariable)
        varName = _rParams->GetColorMapVariableName();
    else {
        bool mainTF = true;
        varName = getTFVariableName(mainTF);
    }
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
    cout << "_dataMgr->GetDataRange()   " << endl;
    cout << "   " << rangev[0] << endl;
    cout << "   " << rangev[1] << endl;

    assert(rangev.size() == 2);

    range[0] = rangev[0];
    range[1] = rangev[1];

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    values[0] = tf->getMinMapValue();
    values[1] = tf->getMaxMapValue();
}

float TFWidget::getOpacity()
{
    bool mainTF = true;
    if (_flags & COLORMAP_VAR_IS_IN_TF2) mainTF = false;
    string varName = getTFVariableName(mainTF);

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    assert(tf);

    return tf->getOpacityScale();
}

void TFWidget::updateColorInterpolation()
{
    MapperFunction *mf;
    QComboBox *     colorInterpCombo;
    QFrame *        whitespaceFrame;
    QCheckBox *     whitespaceCheckbox;

    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
        mf = getSecondaryMapperFunction();
        colorInterpCombo = _secondaryVarInterpCombo;
        whitespaceFrame = _secondaryWhitespaceFrame;
        whitespaceCheckbox = _secondaryWhitespaceCheckbox;
    } else {
        mf = getMainMapperFunction();
        colorInterpCombo = _colorInterpCombo;
        whitespaceFrame = _whitespaceFrame;
        whitespaceCheckbox = _whitespaceCheckbox;
    }

    TFInterpolator::type t = mf->getColorInterpType();
    colorInterpCombo->blockSignals(true);
    if (t == TFInterpolator::diverging) {
        colorInterpCombo->setCurrentIndex(0);
        whitespaceFrame->show();
    } else if (t == TFInterpolator::discrete) {
        colorInterpCombo->setCurrentIndex(1);
        whitespaceFrame->hide();
    } else {
        colorInterpCombo->setCurrentIndex(2);
        whitespaceFrame->hide();
    }
    colorInterpCombo->blockSignals(false);

    int useWhitespace = mf->getUseWhitespace();
    if (useWhitespace) {
        whitespaceCheckbox->setCheckState(Qt::Checked);
    } else {
        whitespaceCheckbox->setCheckState(Qt::Unchecked);
    }

    adjustSize();
}

void TFWidget::updateMainAutoUpdateHistoCheckboxes()
{
    MapperFunction *mf = getMainMapperFunction();
    _autoUpdateMainHistoCheckbox->blockSignals(true);
    if (mf->GetAutoUpdateHisto())
        _autoUpdateMainHistoCheckbox->setCheckState(Qt::Checked);
    else
        _autoUpdateMainHistoCheckbox->setCheckState(Qt::Unchecked);
    _autoUpdateMainHistoCheckbox->blockSignals(false);
}

void TFWidget::updateSecondaryAutoUpdateHistoCheckbox()
{
    MapperFunction *mf = getSecondaryMapperFunction();
    _autoUpdateSecondaryHistoCheckbox->blockSignals(true);
    if (mf->GetAutoUpdateHisto())
        _autoUpdateSecondaryHistoCheckbox->setCheckState(Qt::Checked);
    else
        _autoUpdateSecondaryHistoCheckbox->setCheckState(Qt::Unchecked);
    _autoUpdateSecondaryHistoCheckbox->blockSignals(false);
}

void TFWidget::updateMainSliders()
{
    float range[2], values[2];
    getVariableRange(range, values);

    _rangeCombo->Update(range[0], range[1], values[0], values[1]);
    _opacitySlider->setValue(getOpacity() * 100);

    _minLabel->setText(QString::number(range[0]));
    _maxLabel->setText(QString::number(range[1]));
}

void TFWidget::updateSecondarySliders()
{
    float range[2], values[2];
    getVariableRange(range, values, true);
    _secondaryMinSliderEdit->SetExtents(range[0], range[1]);
    _secondaryMinSliderEdit->SetValue(values[0]);
    _secondaryMaxSliderEdit->SetExtents(range[0], range[1]);
    _secondaryMaxSliderEdit->SetValue(values[1]);

    _secondaryMinLabel->setText(QString::number(range[0]));
    _secondaryMaxLabel->setText(QString::number(range[1]));
}

void TFWidget::updateMainMappingFrame()
{
    bool buttonPress = sender() == _updateMainHistoButton ? true : false;

    bool histogramRecalculated = _mappingFrame->Update(_dataMgr, _paramsMgr, _rParams, buttonPress);

    if (histogramRecalculated) {
        _updateMainHistoButton->setEnabled(false);
        _externalChangeHappened = false;
    } else {
        checkForCompressionChanges();
        checkForBoxChanges();
        checkForMainMapperRangeChanges();
        checkForTimestepChanges();
        if (_externalChangeHappened || _mainHistoRangeChanged) {
            cout << "set to true" << endl;
            _updateMainHistoButton->setEnabled(true);
        }
    }
}

void TFWidget::updateSecondaryMappingFrame()
{
    bool buttonPress = sender() == _updateSecondaryHistoButton ? true : false;

    Histo *secondaryHisto = _secondaryMappingFrame->GetHistogram();
    if (secondaryHisto == nullptr) {
        Histo *mainHisto = _mappingFrame->GetHistogram();
        bool   mainTF = true;
        string varName = getTFVariableName(mainTF);
        _secondaryMappingFrame->CopyHistogram(_paramsMgr, varName, mainHisto);
    }

    bool histogramRecalculated = _secondaryMappingFrame->Update(_dataMgr, _paramsMgr, _rParams, buttonPress);

    MapperFunction *mainMF = getMainMapperFunction();
    MapperFunction *secondaryMF = getSecondaryMapperFunction();
    if (mainMF == secondaryMF) _mappingFrame->fitViewToDataRange();

    if (histogramRecalculated) {
        _updateSecondaryHistoButton->setEnabled(false);
        _externalChangeHappened = false;
    } else {
        checkForCompressionChanges();
        checkForBoxChanges();
        checkForSecondaryMapperRangeChanges();
        checkForTimestepChanges();
        if (_externalChangeHappened || _secondaryHistoRangeChanged) _updateSecondaryHistoButton->setEnabled(true);
    }
}

void TFWidget::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *rParams, bool internalUpdate)
{
    assert(paramsMgr);
    assert(dataMgr);
    assert(rParams);

    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;
    _rParams = rParams;

    bool   mainTF = true;
    string varname = getTFVariableName(mainTF);
    if (varname == "") {
        setEnabled(false);
        return;
    } else {
        setEnabled(true);
    }

    updateMainMappingFrame();    // set mapper func to that of current variable, refresh _rParams etc
    updateSecondaryMappingFrame();

    updateQtWidgets();
}

void TFWidget::updateQtWidgets()
{
    enableUpdateButtonsIfNeeded();
    updateColorInterpolation();
    updateConstColor();
    updateMainAutoUpdateHistoCheckboxes();
    updateSecondaryAutoUpdateHistoCheckbox();
    updateMainSliders();
    updateSecondarySliders();
}

bool TFWidget::mainVariableChanged()
{
    bool   mainTF = true;
    string newName = getTFVariableName(mainTF);
    if (_mainVarName != newName) {
        _mainVarName = newName;
        return true;
    } else
        return false;
}

bool TFWidget::secondaryVariableChanged()
{
    bool   mainTF = false;
    string newName = getTFVariableName(mainTF);
    if (_secondaryVarName != newName) {
        _secondaryVarName = newName;
        return true;
    } else
        return false;
}

void TFWidget::refreshMainHisto()
{
    return;
    _mappingFrame->RefreshHistogram();

    refreshSecondaryDuplicateHistogram();

    Update(_dataMgr, _paramsMgr, _rParams, true);
    _updateMainHistoButton->setEnabled(false);
    _mainHistoNeedsRefresh = false;
}

void TFWidget::refreshSecondaryDuplicateHistogram()
{
    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
        MapperFunction *mainMF = getMainMapperFunction();
        MapperFunction *secondaryMF = getSecondaryMapperFunction();
        if (mainMF == secondaryMF) {
            _secondaryMappingFrame->RefreshHistogram();
            _updateSecondaryHistoButton->setEnabled(false);
        }
    }
}

void TFWidget::refreshSecondaryHisto()
{
    return;
    _secondaryMappingFrame->RefreshHistogram();
    refreshMainDuplicateHistogram();

    Update(_dataMgr, _paramsMgr, _rParams, true);
    _updateSecondaryHistoButton->setEnabled(false);
    _secondaryHistoNeedsRefresh = false;
}

void TFWidget::refreshMainDuplicateHistogram()
{
    MapperFunction *mainMF = getMainMapperFunction();
    MapperFunction *secondaryMF = getSecondaryMapperFunction();
    if (mainMF == secondaryMF) {
        _mappingFrame->RefreshHistogram();
        _updateMainHistoButton->setEnabled(false);
    }
}

void TFWidget::checkForCompressionChanges()
{
    int newCLevel = _rParams->GetCompressionLevel();
    if (_cLevel != newCLevel) {
        _cLevel = _rParams->GetCompressionLevel();
        _externalChangeHappened = true;
    }

    int newRefLevel = _rParams->GetRefinementLevel();
    if (_refLevel != newRefLevel) {
        _refLevel = newRefLevel;
        _externalChangeHappened = true;
    }
}

void TFWidget::checkForBoxChanges()
{
    std::vector<double> minExt, maxExt;
    VAPoR::Box *        box = _rParams->GetBox();
    box->GetExtents(minExt, maxExt);
    for (int i = 0; i < minExt.size(); i++) {
        if (minExt[i] != _minExt[i]) {
            _externalChangeHappened = true;
            _minExt[i] = minExt[i];
        }
        if (maxExt[i] != _maxExt[i]) {
            _externalChangeHappened = true;
            _maxExt[i] = maxExt[i];
        }
    }
}

void TFWidget::checkForMainMapperRangeChanges()
{
    MapperFunction *mf;
    mf = getMainMapperFunction();

    double min = _minCombo->GetValue();
    double max = _maxCombo->GetValue();
    double newMin = mf->getMinMapValue();
    double newMax = mf->getMaxMapValue();

    cout << min << " " << max << " " << newMin << " " << newMax << endl;

    if (min != newMin) _mainHistoRangeChanged = true;
    if (max != newMax) _mainHistoRangeChanged = true;
    if (_mainHistoRangeChanged) _mainHistoRangeChanged = true;
}

void TFWidget::checkForSecondaryMapperRangeChanges()
{
    MapperFunction *mf;
    mf = getSecondaryMapperFunction();

    double min = _secondaryMinSliderEdit->GetCurrentValue();
    double max = _secondaryMaxSliderEdit->GetCurrentValue();
    double newMin = mf->getMinMapValue();
    double newMax = mf->getMaxMapValue();

    if (min != newMin) _secondaryHistoRangeChanged = true;
    if (max != newMax) _secondaryHistoRangeChanged = true;
    if (_secondaryHistoRangeChanged) _secondaryHistoRangeChanged = true;
}

void TFWidget::checkForTimestepChanges()
{
    int newTimestep = _rParams->GetCurrentTimestep();
    if (_timeStep != newTimestep) {
        _timeStep = newTimestep;
        _externalChangeHappened = true;
    }
}

// These tests will enable the Update Histo button if a change is found
// If the Auto-update Histo checkbox is checked, we will perform a refresh
void TFWidget::enableUpdateButtonsIfNeeded()
{
    // Check for changes to the primary MapperFunction
    checkForCompressionChanges();
    checkForBoxChanges();
    checkForMainMapperRangeChanges();
    checkForTimestepChanges();

    if (_externalChangeHappened || _mainHistoRangeChanged) {
        MapperFunction *mf = getMainMapperFunction();
        if (mf->GetAutoUpdateHisto()) {
            _mainHistoNeedsRefresh = true;
            _initialized = true;
        } else if (_initialized) {
            _updateMainHistoButton->setEnabled(true);
        } else {
            _updateMainHistoButton->setEnabled(false);
        }
    }
    _mainHistoRangeChanged = false;

    // Now check for changes to the secondary mapper function
    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
        checkForSecondaryMapperRangeChanges();

        if (_externalChangeHappened || _secondaryHistoRangeChanged) {
            MapperFunction *mf = getSecondaryMapperFunction();
            if (mf->GetAutoUpdateHisto()) {
                _secondaryHistoNeedsRefresh = true;
                _initialized = true;
            } else if (_initialized) {
                _updateSecondaryHistoButton->setEnabled(true);
            } else {
                _updateSecondaryHistoButton->setEnabled(false);
                _initialized = true;
            }
        }
        _secondaryHistoRangeChanged = false;
    }
    _externalChangeHappened = false;
}

void TFWidget::updateConstColor()
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
    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
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

void TFWidget::enableTFWidget(bool state)
{
    _loadButton->setEnabled(state);
    _saveButton->setEnabled(state);
    _tfFrame->setEnabled(state);
    _minRangeEdit->setEnabled(state);
    _maxRangeEdit->setEnabled(state);
    _opacitySlider->setEnabled(state);
    _autoUpdateMainHistoCheckbox->setEnabled(state);
    _colorInterpCombo->setEnabled(state);
}

void TFWidget::connectWidgets()
{
    connect(_rangeCombo, SIGNAL(valueChanged(double, double)), this, SLOT(setRange(double, double)));
    connect(_updateMainHistoButton, SIGNAL(pressed()), this, SLOT(updateMainMappingFrame()));
    connect(_autoUpdateMainHistoCheckbox, SIGNAL(stateChanged(int)), this, SLOT(autoUpdateMainHistoChecked(int)));
    connect(_colorInterpCombo, SIGNAL(activated(int)), this, SLOT(setColorInterpolation(int)));
    connect(_whitespaceCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setUseWhitespace(int)));
    connect(_loadButton, SIGNAL(pressed()), this, SLOT(loadTF()));
    connect(_saveButton, SIGNAL(pressed()), this, SLOT(fileSaveTF()));
    connect(_mappingFrame, SIGNAL(updateParams()), this, SLOT(setRange()));
    connect(_mappingFrame, SIGNAL(endChange()), this, SLOT(setRange()));
    connect(_opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));
    connect(_colorSelectButton, SIGNAL(pressed()), this, SLOT(setSingleColor()));
    connect(_useConstColorCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setUsingSingleColor(int)));

    // Connections for our SecondaryVariable transfer function
    //
    connect(_secondaryMappingFrame, SIGNAL(endChange()), this, SLOT(setSecondaryRange()));
    connect(_secondaryOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));
    connect(_updateSecondaryHistoButton, SIGNAL(pressed()), this, SLOT(updateSecondaryMappingFrame()));
    connect(_autoUpdateSecondaryHistoCheckbox, SIGNAL(stateChanged(int)), this, SLOT(autoUpdateSecondaryHistoChecked(int)));
    connect(_secondaryVarInterpCombo, SIGNAL(activated(int)), this, SLOT(setColorInterpolation(int)));
    connect(_secondaryWhitespaceCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setUseWhitespace(int)));
    connect(_secondaryMinSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(setSecondaryMinRange(double)));
    connect(_secondaryMaxSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(setSecondaryMaxRange(double)));
    connect(_secondaryLoadButton, SIGNAL(pressed()), this, SLOT(loadTF()));
    connect(_secondarySaveButton, SIGNAL(pressed()), this, SLOT(fileSaveTF()));
}

void TFWidget::emitTFChange() { emit emitChange(); }

void TFWidget::opacitySliderChanged(int value)
{
    bool mainTF = true;
    if (COLORMAP_VAR_IS_IN_TF2) mainTF = false;
    string          varName = getTFVariableName(mainTF);
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
    _mainHistoRangeChanged = true;

    float values[2];
    float range[2];
    getVariableRange(range, values);
    if (min < range[0]) min = range[0];
    if (min > range[1]) min = range[1];
    if (max > range[1]) max = range[1];
    if (max < range[0]) max = range[0];

    MapperFunction *mf = getMainMapperFunction();

    _paramsMgr->BeginSaveStateGroup("Setting main TFWidget range");

    mf->setMinMapValue(min);
    mf->setMaxMapValue(max);

    if (getAutoUpdateMainHisto() == true) refreshMainHisto();

    _paramsMgr->EndSaveStateGroup();

    emit emitChange();
}

void TFWidget::setSecondaryRange()
{
    _paramsMgr->BeginSaveStateGroup("Setting secondary TFWidget range");

    double min = _secondaryMappingFrame->getMinEditBound();
    setSecondaryMinRange(min);

    double max = _secondaryMappingFrame->getMaxEditBound();
    setSecondaryMaxRange(max);

    _paramsMgr->EndSaveStateGroup();

    emit emitChange();
}

void TFWidget::autoUpdateMainHistoChecked(int state)
{
    bool bstate;
    if (state == 0)
        bstate = false;
    else
        bstate = true;

    MapperFunction *mf = getMainMapperFunction();
    mf->SetAutoUpdateHisto(bstate);

    if (bstate == true) refreshMainHisto();
}

void TFWidget::autoUpdateSecondaryHistoChecked(int state)
{
    bool bstate;
    if (state == 0)
        bstate = false;
    else
        bstate = true;

    MapperFunction *mf = getSecondaryMapperFunction();
    mf->SetAutoUpdateHisto(bstate);

    if (bstate == true) { refreshSecondaryHisto(); }
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
        if (_flags & COLORMAP_VAR_IS_IN_TF2) _secondaryTFE->setEnabled(false);
    } else {
        _rParams->SetUseSingleColor(false);
        if (_flags & COLORMAP_VAR_IS_IN_TF2) _secondaryTFE->setEnabled(true);
    }
}

void TFWidget::setColorInterpolation(int index)
{
    MapperFunction *mf = getMainMapperFunction();

    if (index == 0) {
        mf->setColorInterpType(TFInterpolator::diverging);
    } else if (index == 1) {
        mf->setColorInterpType(TFInterpolator::discrete);
    } else if (index == 2) {
        mf->setColorInterpType(TFInterpolator::linear);
    }
}

void TFWidget::setUseWhitespace(int state)
{
    MapperFunction *mf;
    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
        mf = getSecondaryMapperFunction();
    } else {
        mf = getMainMapperFunction();
    }
    mf->setUseWhitespace(state);
}

void TFWidget::setSecondaryMinRange(double min)
{
    _secondaryHistoRangeChanged = true;

    float values[2];
    float range[2];
    bool  secondaryVar = true;
    getVariableRange(range, values, secondaryVar);

    if (min < range[0]) min = range[0];
    if (min > range[1]) min = range[1];

    MapperFunction *mf = getSecondaryMapperFunction();
    mf->setMinMapValue(min);
}

void TFWidget::setSecondaryMaxRange(double max)
{
    _secondaryHistoRangeChanged = true;

    float values[2];
    float range[2];
    bool  secondaryVar = true;
    getVariableRange(range, values, secondaryVar);

    if (max > range[1]) max = range[1];
    if (max < range[0]) max = range[0];

    MapperFunction *mf = getSecondaryMapperFunction();
    mf->setMaxMapValue(max);
}

bool TFWidget::getAutoUpdateMainHisto()
{
    MapperFunction *mf;
    mf = getMainMapperFunction();

    if (mf->GetAutoUpdateHisto())
        return true;
    else
        return false;
}

bool TFWidget::getAutoUpdateSecondaryHisto()
{
    MapperFunction *mf;
    mf = getSecondaryMapperFunction();

    if (mf->GetAutoUpdateHisto())
        return true;
    else
        return false;
}

MapperFunction *TFWidget::getMainMapperFunction()
{
    // string varname = _rParams->GetVariableName();
    bool            mainTF = true;
    string          varname = getTFVariableName(mainTF);
    MapperFunction *mf = _rParams->GetMapperFunc(varname);
    cout << "GetMapperFunc " << varname << endl;
    assert(mf);
    return mf;
}

MapperFunction *TFWidget::getSecondaryMapperFunction()
{
    string          varname = _rParams->GetColorMapVariableName();
    MapperFunction *mf = _rParams->GetMapperFunc(varname);
    assert(mf);
    return mf;
}

string TFWidget::getTFVariableName(bool mainTF = true)
{
    string varname;

    if (mainTF == true) {
        if (_flags & COLORMAP_VAR_IS_IN_TF2) {
            varname = _rParams->GetVariableName();
        } else {
            varname = _rParams->GetColorMapVariableName();
        }
    } else {
        varname = _rParams->GetColorMapVariableName();
    }

    return varname;
}
