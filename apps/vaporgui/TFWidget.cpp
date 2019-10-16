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
#include <QDebug>
#include <QFileDialog>
#include <QFontDatabase>
#include <QStringList>
#include <qradiobutton.h>
#include <qcolordialog.h>
#include "TwoDSubtabs.h"
#include "RenderEventRouter.h"
#include "vapor/RenderParams.h"
#include "vapor/TwoDDataParams.h"
#include "vapor/ResourcePath.h"
#include "TFWidget.h"
#include "ErrorReporter.h"
#include "FileOperationChecker.h"
#include "VaporWidgets.h"

#define REQUIRED_SAMPLE_SIZE 1000000

using namespace VAPoR;
using namespace TFWidget_;

#define GET_DATARANGE_STRIDE 16
#define CANCEL               -1
#define ACCEPT               0

string TFWidget::_nDimsTag = "ActiveDimension";

TFWidget::TFWidget(QWidget *parent) : QWidget(parent), Ui_TFWidgetGUI()
{
    setupUi(this);

    _initialized = false;
    _externalChangeHappened = false;
    _mainHistoRangeChanged = false;
    _secondaryHistoRangeChanged = false;
    _discreteColormap = false;
    _isOpacitySupported = true;
    _isOpacityIntegrated = false;
    _wasOpacitySliderReleased = false;
    _mainVarName = "";
    _secondaryVarName = "";

    _myRGB[0] = _myRGB[1] = _myRGB[2] = 1.f;

    _loadTFDialog = new LoadTFDialog(this);

    _minCombo = new Combo(_minRangeEdit, _minRangeSlider);
    _maxCombo = new Combo(_maxRangeEdit, _maxRangeSlider);
    _rangeCombo = new RangeCombo(_minCombo, _maxCombo);

    _secondaryMinSliderEdit->SetLabel(QString::fromAscii("Min"));
    _secondaryMinSliderEdit->SetIntType(false);
    _secondaryMinSliderEdit->SetExtents(0.f, 1.f);

    _secondaryMaxSliderEdit->SetLabel(QString::fromAscii("Max"));
    _secondaryMaxSliderEdit->SetIntType(false);
    _secondaryMaxSliderEdit->SetExtents(0.f, 1.f);

    _opacitySlider->setRange(0, 1000);
    _secondaryOpacitySlider->setRange(0, 1000);

    _cLevel = 0;
    _refLevel = 0;
    _timeStep = 0;
    _stride = 1;
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
        adjustSize();
    } else {
        _useConstColorFrame->hide();
        _constColorFrame->hide();
        adjustSize();
    }
}

void TFWidget::configureSecondaryTransferFunction()
{
    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
        if (_tabWidget->count() < 2) _tabWidget->insertTab(1, _secondaryTFE, "Color Mapped VARIABLE");

        _tabWidget->setTabEnabled(1, false);
        _mappingFrame->setColorMapping(false);
        _mappingFrame->setOpacityMapping(false);
        _whitespaceFrame->hide();
        _colorInterpolationFrame->hide();
        _loadSaveFrame->hide();
        adjustSize();
    } else {
        _mappingFrame->setColorMapping(true);
        _mappingFrame->setOpacityMapping(true);
        _tabWidget->removeTab(1);
        _whitespaceFrame->show();
        _colorInterpolationFrame->show();
        _useConstColorFrame->show();
        _constColorFrame->show();
        _loadSaveFrame->show();
        adjustSize();
    }
}

void TFWidget::Reinit(TFFlags flags)
{
    _flags = flags;

    if (_flags & ISOLINES)
        _mappingFrame->setIsolineSliders(true);
    else
        _mappingFrame->setIsolineSliders(false);

    if (_flags & SAMPLING) {
        _mappingFrame->SetIsSampling(true);
        _secondaryMappingFrame->SetIsSampling(true);
    }

    configureSecondaryTransferFunction();
    configureConstantColorControls();

    _tabWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    adjustSize();
}

TFWidget::~TFWidget()
{
    if (_minCombo != nullptr) {
        delete _minCombo;
        _minCombo = nullptr;
    }
    if (_maxCombo != nullptr) {
        delete _maxCombo;
        _maxCombo = nullptr;
    }
    if (_rangeCombo != nullptr) {
        delete _rangeCombo;
        _rangeCombo = nullptr;
    }
}

void TFWidget::loadTF()
{
    string varname = _rParams->GetColorMapVariableName();
    if (varname.empty()) return;

    _loadTFDialog->SetMapperFunction(_rParams->GetMapperFunc(varname));

    int rc = _loadTFDialog->exec();
    if (rc == CANCEL) return;    // cancel event

    string fileName = _loadTFDialog->GetSelectedFile();
    if (!selectedTFFileOk(fileName)) return;

    SettingsParams *sP;
    sP = (SettingsParams *)_paramsMgr->GetParams(SettingsParams::GetClassType());
    sP->SetTFDir(fileName);

    MapperFunction *tf = _rParams->GetMapperFunc(varname);
    VAssert(tf);

    float                            cachedMin = tf->getMinMapValue();
    float                            cachedMax = tf->getMaxMapValue();
    int                              numOpacityMaps = tf->getNumOpacityMaps();
    std::vector<std::vector<double>> controlPoints;
    for (int i = 0; i < numOpacityMaps; i++) { controlPoints.push_back(tf->GetOpacityMap(i)->GetControlPoints()); }
    float opacityScale = tf->getOpacityScale();

    _paramsMgr->BeginSaveStateGroup("Loading Transfer Function from file");

    rc = tf->LoadFromFile(fileName);
    if (rc < 0) {
        MSG_ERR("Error loading transfer function");
    } else {
        bool loadTF3DataRange = _loadTFDialog->GetLoadTF3DataRange();
        if (loadTF3DataRange == false) {
            // Ignore values in the file.  Load values from current session.
            tf->setMinMaxMapValue(cachedMin, cachedMax);
        }

        bool loadTF3Opacity = _loadTFDialog->GetLoadTF3OpacityMap();
        if (loadTF3Opacity == false) {
            // Ignore values in the file.  Load values from current session.
            for (int i = 0; i < numOpacityMaps; i++) { tf->GetOpacityMap(i)->SetControlPoints(controlPoints[i]); }
            tf->setOpacityScale(opacityScale);
        }
    }

    _paramsMgr->EndSaveStateGroup();

    Update(_dataMgr, _paramsMgr, _rParams);
}

bool TFWidget::selectedTFFileOk(string fileName)
{
    QString qFileName = QString::fromStdString(fileName);
    if (!FileOperationChecker::FileGoodToRead(qFileName)) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        return false;
    }

    QString qSuffix = "tf3";
    if (!FileOperationChecker::FileHasCorrectSuffix(qFileName, qSuffix)) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        return false;
    }

    return true;
}

void TFWidget::saveTF()
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
    VAssert(tf);

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

    vector<double> minExt, maxExt;
    _rParams->GetBox()->GetExtents(minExt, maxExt);

    vector<double> rangev;
    int            rc = _dataMgr->GetDataRange(ts, varName, ref, cmp, minExt, maxExt, rangev);

    if (rc < 0) {
        MSG_ERR("Error loading variable");
        return;
    }

    VAssert(rangev.size() == 2);

    range[0] = rangev[0];
    range[1] = rangev[1];

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    values[0] = tf->getMinMapValue();
    values[1] = tf->getMaxMapValue();
}

void TFWidget::calculateStride(string varName)
{
    std::vector<size_t> dimsAtLevel;
    int                 ref = _rParams->GetRefinementLevel();
    int                 rc = _dataMgr->GetDimLensAtLevel(varName, ref, dimsAtLevel);
    VAssert(rc >= 0);

    long size = 1;
    for (int i = 0; i < dimsAtLevel.size(); i++) size *= dimsAtLevel[i];

    _stride = 1;
    if (size > REQUIRED_SAMPLE_SIZE) _stride = 1 + size / REQUIRED_SAMPLE_SIZE;

    _mappingFrame->SetStride(_stride);
}

float TFWidget::getOpacity()
{
    bool mainTF = true;
    if (_flags & COLORMAP_VAR_IS_IN_TF2 && !_rParams->UseSingleColor()) { mainTF = false; }
    string varName = getTFVariableName(mainTF);

    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    VAssert(tf);

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
        if (whitespaceFrame->isHidden()) {
            whitespaceFrame->show();
            adjustSize();
        }
    } else if (t == TFInterpolator::discrete) {
        colorInterpCombo->setCurrentIndex(1);
        if (!whitespaceFrame->isHidden()) {
            whitespaceFrame->hide();
            adjustSize();
        }
    } else {
        colorInterpCombo->setCurrentIndex(2);
        if (!whitespaceFrame->isHidden()) {
            whitespaceFrame->hide();
            adjustSize();
        }
    }
    colorInterpCombo->blockSignals(false);

    int useWhitespace = mf->getUseWhitespace();
    if (useWhitespace) {
        whitespaceCheckbox->setCheckState(Qt::Checked);
    } else {
        whitespaceCheckbox->setCheckState(Qt::Unchecked);
    }
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
    _opacitySlider->setValue(convertOpacityToSliderValue(getOpacity()));

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
    _secondaryOpacitySlider->setValue(convertOpacityToSliderValue(getOpacity()));

    _secondaryMinLabel->setText(QString::number(range[0]));
    _secondaryMaxLabel->setText(QString::number(range[1]));
}

void TFWidget::updateMainMappingFrame()
{
    bool buttonPress = false;
    if (sender() == _updateMainHistoButton || getAutoUpdateMainHisto()) { buttonPress = true; }

    bool histogramRecalculated = _mappingFrame->Update(_dataMgr, _paramsMgr, _rParams, buttonPress);

    if (histogramRecalculated) {
        _updateMainHistoButton->setEnabled(false);
        _mappingFrame->SetHistoNeedsUpdate(false);
        _externalChangeHappened = false;
        _initialized = true;
    } else {
        checkForCompressionChanges();
        checkForBoxChanges();
        checkForMainMapperRangeChanges();
        checkForTimestepChanges();
        if (_externalChangeHappened || _mainHistoRangeChanged) {
            _updateMainHistoButton->setEnabled(true);
            _mappingFrame->SetHistoNeedsUpdate(true);
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
        _secondaryMappingFrame->SetHistoNeedsUpdate(false);
        _externalChangeHappened = false;
    } else {
        checkForCompressionChanges();
        checkForBoxChanges();
        checkForSecondaryMapperRangeChanges();
        checkForTimestepChanges();
        if (_externalChangeHappened || _secondaryHistoRangeChanged) {
            _updateSecondaryHistoButton->setEnabled(true);
            _secondaryMappingFrame->SetHistoNeedsUpdate(true);
        }
    }
}

void TFWidget::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *rParams, bool internalUpdate)
{
    VAssert(paramsMgr);
    VAssert(dataMgr);
    VAssert(rParams);

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

    calculateStride(varname);
    updateQtWidgets();
    updateMainMappingFrame();    // set mapper func to that of current variable, refresh _rParams etc
    updateSecondaryMappingFrame();
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

void TFWidget::refreshSecondaryDuplicateHistogram()
{
    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
        MapperFunction *mainMF = getMainMapperFunction();
        MapperFunction *secondaryMF = getSecondaryMapperFunction();
        if (mainMF == secondaryMF) {
            _secondaryMappingFrame->RefreshHistogram();
            _updateSecondaryHistoButton->setEnabled(false);
            _secondaryMappingFrame->SetHistoNeedsUpdate(false);
        }
    }
}

void TFWidget::refreshMainDuplicateHistogram()
{
    MapperFunction *mainMF = getMainMapperFunction();
    MapperFunction *secondaryMF = getSecondaryMapperFunction();
    if (mainMF == secondaryMF) {
        _mappingFrame->RefreshHistogram();
        _updateMainHistoButton->setEnabled(false);
        _mappingFrame->SetHistoNeedsUpdate(false);
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

    if (min != newMin) { _mainHistoRangeChanged = true; }
    if (max != newMax) { _mainHistoRangeChanged = true; }
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
            _initialized = true;
        } else if (_initialized) {
            _updateMainHistoButton->setEnabled(true);
            _mappingFrame->SetHistoNeedsUpdate(true);
        } else {
            _updateMainHistoButton->setEnabled(false);
            _mappingFrame->SetHistoNeedsUpdate(false);
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
                _secondaryMappingFrame->SetHistoNeedsUpdate(true);
            } else {
                _updateSecondaryHistoButton->setEnabled(false);
                _secondaryMappingFrame->SetHistoNeedsUpdate(false);
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
    if (useSingleColor) {
        _useConstColorCheckbox->setCheckState(Qt::Checked);
        if (_isOpacitySupported) _opacitySlider->show();
    } else {
        _useConstColorCheckbox->setCheckState(Qt::Unchecked);

        if (_flags & COLORMAP_VAR_IS_IN_TF2) { _opacitySlider->hide(); }
    }

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
    connect(_saveButton, SIGNAL(pressed()), this, SLOT(saveTF()));
    connect(_mappingFrame, SIGNAL(updateParams()), this, SLOT(setRange()));
    connect(_mappingFrame, SIGNAL(endChange()), this, SLOT(setRange()));
    connect(_opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));
    connect(_opacitySlider, SIGNAL(sliderReleased()), this, SLOT(opacitySliderReleased()));
    connect(_colorSelectButton, SIGNAL(pressed()), this, SLOT(setSingleColor()));
    connect(_useConstColorCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setUsingSingleColor(int)));

    // Connections for our SecondaryVariable transfer function
    //
    connect(_secondaryMappingFrame, SIGNAL(endChange()), this, SLOT(setSecondaryRange()));
    connect(_secondaryOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));
    connect(_secondaryOpacitySlider, SIGNAL(sliderReleased()), this, SLOT(opacitySliderReleased()));
    connect(_updateSecondaryHistoButton, SIGNAL(pressed()), this, SLOT(updateSecondaryMappingFrame()));
    connect(_autoUpdateSecondaryHistoCheckbox, SIGNAL(stateChanged(int)), this, SLOT(autoUpdateSecondaryHistoChecked(int)));
    connect(_secondaryVarInterpCombo, SIGNAL(activated(int)), this, SLOT(setColorInterpolation(int)));
    connect(_secondaryWhitespaceCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setUseWhitespace(int)));
    connect(_secondaryMinSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(setSecondaryMinRange(double)));
    connect(_secondaryMaxSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(setSecondaryMaxRange(double)));
    connect(_secondaryLoadButton, SIGNAL(pressed()), this, SLOT(loadTF()));
    connect(_secondarySaveButton, SIGNAL(pressed()), this, SLOT(saveTF()));
}

void TFWidget::emitTFChange() { emit emitChange(); }

void TFWidget::opacitySliderChanged(int value)
{
    if (!_wasOpacitySliderReleased) return;
    _wasOpacitySliderReleased = false;

    bool mainTF = true;
    if (COLORMAP_VAR_IS_IN_TF2 && !_rParams->UseSingleColor()) { mainTF = false; }
    string          varName = getTFVariableName(mainTF);
    MapperFunction *tf = _rParams->GetMapperFunc(varName);
    VAssert(tf);
    tf->setOpacityScale(convertSliderValueToOpacity(value));

    emit emitChange();
}

void TFWidget::opacitySliderReleased() { _wasOpacitySliderReleased = true; }

void TFWidget::setRange()
{
    float min = _mappingFrame->getMinEditBound();
    float max = _mappingFrame->getMaxEditBound();
    setRange(min, max);
    emit emitChange();
}

void TFWidget::setRange(double min, double max)
{
    float values[2];
    float range[2];
    getVariableRange(range, values);
    if (min < range[0]) min = range[0];
    if (min > range[1]) min = range[1];
    if (max > range[1]) max = range[1];
    if (max < range[0]) max = range[0];

    if (min != values[0] || max != values[1]) {
        _mainHistoRangeChanged = true;
    } else
        return;

    MapperFunction *mf = getMainMapperFunction();

    _paramsMgr->BeginSaveStateGroup("Setting main TFWidget range");

    mf->setMinMapValue(min);
    mf->setMaxMapValue(max);

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
        if (_flags & COLORMAP_VAR_IS_IN_TF2) {
            _tabWidget->setTabEnabled(1, false);
            _opacitySlider->hide();
        }
    } else {
        _rParams->SetUseSingleColor(false);
        if (_flags & COLORMAP_VAR_IS_IN_TF2) {
            _tabWidget->setTabEnabled(1, true);
            if (_isOpacitySupported) _opacitySlider->show();
        }
    }
}

void TFWidget::setColorInterpolation(int index)
{
    MapperFunction *mf;
    if (_flags & COLORMAP_VAR_IS_IN_TF2) {
        mf = getSecondaryMapperFunction();
    } else {
        mf = getMainMapperFunction();
    }

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
    bool            mainTF = true;
    string          varname = getTFVariableName(mainTF);
    MapperFunction *mf = _rParams->GetMapperFunc(varname);
    VAssert(mf);
    return mf;
}

MapperFunction *TFWidget::getSecondaryMapperFunction()
{
    string          varname = _rParams->GetColorMapVariableName();
    MapperFunction *mf = _rParams->GetMapperFunc(varname);
    VAssert(mf);
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

int TFWidget::convertOpacityToSliderValue(float opacity) const
{
    if (IsOpacityIntegrated())
        return 1000 * powf(opacity, 1 / 4.f);
    else
        return 1000 * opacity;
}

float TFWidget::convertSliderValueToOpacity(int value) const
{
    if (IsOpacityIntegrated())
        return powf(value / 1000.f, 4);
    else
        return value / 1000.f;
}

bool TFWidget::IsOpacitySupported() const { return _isOpacitySupported; }

void TFWidget::SetOpacitySupported(bool value)
{
    if (!value && !_opacitySlider->isHidden()) { _opacitySlider->hide(); }
    if (value && _opacitySlider->isHidden() && _isOpacitySupported) { _opacitySlider->show(); }
    _mappingFrame->setOpacityMapping(value);

    _isOpacitySupported = value;
}

bool TFWidget::IsOpacityIntegrated() const { return _isOpacityIntegrated; }

void TFWidget::SetOpacityIntegrated(bool value) { _isOpacityIntegrated = value; }

LoadTFDialog::LoadTFDialog(QWidget *parent)
: QDialog(parent), _fileDialog(nullptr), _checkboxFrame(nullptr), _fileDialogFrame(nullptr), _colormapButtonFrame(nullptr), _mainLayout(nullptr), _checkboxLayout(nullptr), _fileDialogLayout(nullptr),
  _colormapButtonLayout(nullptr), _fileDialogTab(nullptr), _loadOptionTab(nullptr), _colormapButtonTab(nullptr), _optionSpacer1(nullptr), _optionSpacer2(nullptr), _optionSpacer3(nullptr),
  _loadOpacityMapCheckbox(nullptr), _loadDataBoundsCheckbox(nullptr), _buttonGroup(nullptr), _myDir(""), _loadOpacityMap(false), _loadDataBounds(false), _selectedFile("")
{
    setModal(true);

    initializeLayout();
    configureLayout();

    connectWidgets();
}

LoadTFDialog::~LoadTFDialog() {}

bool LoadTFDialog::GetLoadTF3OpacityMap() const { return _loadOpacityMap; }

bool LoadTFDialog::GetLoadTF3DataRange() const { return _loadDataBounds; }

string LoadTFDialog::GetSelectedFile() const { return _selectedFile; }

void LoadTFDialog::SetMapperFunction(const VAPoR::MapperFunction *fn)
{
    assert(fn);
    //_mapperFnCopy = make_unique<VAPoR::MapperFunction>(*fn);

    _mapperFnCopy = std::unique_ptr<VAPoR::MapperFunction>(new VAPoR::MapperFunction(*fn));

    BuildColormapButtons();
}

void LoadTFDialog::BuildColormapButtons()
{
    // If we've already built the buttons for this directory, return
    //
    if (_myDir == _fileDialog->directory()) { return; }

    rebuildWidgets();

    _myDir = _fileDialog->directory();
    QStringList files = _myDir.entryList(QDir::Files);
    QString     path = _myDir.absolutePath();

    int buttonIndex = 0;
    for (const auto &i : files) {
        if (!FileOperationChecker::FileHasCorrectSuffix(i, "tf3")) { continue; }

        QString fullPath = path + "//" + i;
        if (!FileOperationChecker::FileGoodToRead(fullPath)) { continue; }

        VPushButtonWithDoubleClick *button;
        button = makeButton(path, i);

        int row = buttonIndex % 4;
        int col = buttonIndex / 4;
        _buttonGroup->addButton(button);
        _colormapButtonLayout->addWidget(button, row, col);

        buttonIndex++;
    }

    if (buttonIndex == 0)
        _colormapButtonFrame->hide();
    else
        _colormapButtonFrame->show();
}

void LoadTFDialog::rebuildWidgets()
{
    if (_buttonGroup != nullptr) {
        delete _buttonGroup;
        _buttonGroup = nullptr;
    }
    _buttonGroup = new QButtonGroup(this);
    _buttonGroup->setExclusive(true);

    if (_colormapButtonFrame != nullptr) {
        // this also deletes its child, _colormapButtonLayout
        delete _colormapButtonFrame;
        _colormapButtonFrame = nullptr;
    }
    _colormapButtonFrame = new QFrame;
    _colormapButtonLayout = new QGridLayout(_colormapButtonFrame);

    _colormapButtonTab->addTab(_colormapButtonFrame, "Colormap Preview");
}

VPushButtonWithDoubleClick *LoadTFDialog::makeButton(const QString &path, const QString &file)
{
    QString fullFilePath = path + "//" + file;

    _mapperFnCopy->LoadFromFile(fullFilePath.toStdString());

    float rgb[3];

    int    numEntries = _mapperFnCopy->getNumEntries();
    QImage image(numEntries, 1, QImage::Format_RGB32);
    for (int j = 0; j < numEntries; j++) {
        float lookupValue = _mapperFnCopy->mapIndexToFloat(j);
        _mapperFnCopy->rgbValue(lookupValue, rgb);
        QRgb value = qRgb((int)(rgb[0] * 255), (int)(rgb[1] * 255), (int)(rgb[2] * 255));
        image.setPixel(j, 0, value);
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    pixmap = pixmap.scaled(100, 10);
    QIcon buttonIcon(pixmap);

    VPushButtonWithDoubleClick *button;
    button = new VPushButtonWithDoubleClick("", _colormapButtonTab);
    button->setCheckable(true);
    button->setIcon(buttonIcon);
    button->setIconSize(pixmap.rect().size());
    button->setProperty("path", path);
    button->setProperty("file", file);
    connect(button, SIGNAL(toggled(bool)), this, SLOT(buttonChecked()));
    connect(button, SIGNAL(doubleClicked()), this, SLOT(buttonDoubleClicked()));

    return button;
}

void LoadTFDialog::buttonChecked()
{
    QPushButton *button = (QPushButton *)sender();
    QString      path = button->property("path").toString();
    QString      file = button->property("file").toString();

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(_fileDialog->focusWidget());
    if (lineEdit) { lineEdit->setText(file); }

    _fileDialog->selectFile(path + "//" + file);
}

void LoadTFDialog::buttonDoubleClicked()
{
    QPushButton *button = (QPushButton *)sender();
    QString      path = button->property("path").toString();
    QString      file = button->property("file").toString();
    _fileDialog->selectFile(path + "//" + file);
    accept();
}

void LoadTFDialog::checkSelectedColorButton(const QString &file)
{
    QFileInfo fInfo(file);
    QString   userFile = fInfo.fileName();

    QString                  buttonPath;
    QList<QAbstractButton *> buttons = _buttonGroup->buttons();
    for (auto button : buttons) {
        QString buttonFile = button->property("file").toString();
        if (userFile == buttonFile) {
            button->setChecked(true);
            break;
        }
    }
}

void LoadTFDialog::initializeLayout()
{
    _mainLayout = new QVBoxLayout;

    _loadOptionTab = new QTabWidget;
    _checkboxFrame = new QFrame;
    _checkboxLayout = new QHBoxLayout;
    _loadOpacityMapCheckbox = new QCheckBox;
    _loadDataBoundsCheckbox = new QCheckBox;
    _optionSpacer1 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    _optionSpacer2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    _optionSpacer3 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);

    _fileDialogTab = new QTabWidget;
    _fileDialogFrame = new QFrame;
    _fileDialog = new CustomFileDialog(this);
    _fileDialogLayout = new QVBoxLayout;

    _colormapButtonTab = new QTabWidget;
    _colormapButtonTab->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _colormapButtonFrame = new QFrame;
    _colormapButtonLayout = new QGridLayout(_colormapButtonFrame);
}

void LoadTFDialog::configureLayout()
{
    _loadOpacityMapCheckbox->setLayoutDirection(Qt::RightToLeft);
    _loadOpacityMapCheckbox->setText("Load opacity map from file\t");

    _loadDataBoundsCheckbox->setLayoutDirection(Qt::RightToLeft);
    _loadDataBoundsCheckbox->setText("Load data bounds from file\t");

    _checkboxLayout->addSpacerItem(_optionSpacer1);
    _checkboxLayout->addWidget(_loadOpacityMapCheckbox, 0);
    _checkboxLayout->addSpacerItem(_optionSpacer2);
    _checkboxLayout->addWidget(_loadDataBoundsCheckbox, 0);
    _checkboxLayout->addSpacerItem(_optionSpacer3);
    _checkboxLayout->setContentsMargins(0, 0, 0, 0);
    _checkboxFrame->setLayout(_checkboxLayout);

    _loadOptionTab->addTab(_checkboxFrame, "Options");

    //_colormapButtonLayout = new QGridLayout( _colormapButtonFrame );
    _colormapButtonTab->addTab(_colormapButtonFrame, "Colormap Preview");

    _fileDialog->setWindowFlags(_fileDialog->windowFlags() & ~Qt::Dialog);
    QString directory = QString::fromStdString(Wasp::GetSharePath(string("palettes")));
    _fileDialog->setDirectory(directory);
    _fileDialog->setNameFilter("*.tf3");
    _fileDialogLayout->addWidget(_fileDialog);
    _fileDialogLayout->setContentsMargins(0, 0, 0, 0);
    _fileDialogFrame->setLayout(_fileDialogLayout);
    _fileDialogTab->addTab(_fileDialogFrame, "Select .tf3 File");

    _mainLayout->addWidget(_loadOptionTab, 0);
    _mainLayout->addWidget(_colormapButtonTab, 1);
    _mainLayout->addWidget(_fileDialogTab, 2);

    setLayout(_mainLayout);
    adjustSize();
}

void LoadTFDialog::connectWidgets()
{
    connect(_loadOpacityMapCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setLoadOpacity()));
    connect(_loadDataBoundsCheckbox, SIGNAL(stateChanged(int)), this, SLOT(setLoadBounds()));

    connect(_fileDialog, SIGNAL(okClicked()), this, SLOT(accept()));
    connect(_fileDialog, SIGNAL(cancelClicked()), this, SLOT(reject()));

    connect(_fileDialog, SIGNAL(directoryEntered(const QString &)), this, SLOT(BuildColormapButtons()));

    connect(_fileDialog, SIGNAL(currentChanged(const QString &)), this, SLOT(checkSelectedColorButton(const QString &)));
}

void LoadTFDialog::setLoadOpacity() { _loadOpacityMap = _loadOpacityMapCheckbox->isChecked(); }

void LoadTFDialog::setLoadBounds() { _loadDataBounds = _loadDataBoundsCheckbox->isChecked(); }

void LoadTFDialog::accept()
{
    _loadOpacityMap = _loadOpacityMapCheckbox->isChecked();
    _loadDataBounds = _loadDataBoundsCheckbox->isChecked();
    QStringList selectedFiles = _fileDialog->selectedFiles();
    if (selectedFiles.size() > 0) _selectedFile = selectedFiles[0].toStdString();
    done(ACCEPT);
}

void LoadTFDialog::reject() { done(CANCEL); }

CustomFileDialog::CustomFileDialog(QWidget *parent) : QFileDialog(parent) {}

void CustomFileDialog::done(int result) { emit cancelClicked(); }

void CustomFileDialog::accept() { emit okClicked(); }
